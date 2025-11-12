#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cv2, numpy as np, math, time, argparse
import can
from struct import pack

# ===== CAN 설정 =====
CAN_IFACE       = "can0"
CAN_ID_TRIGGER  = 0x210   # data[0]==1 → LKAS 시작, 0 → 정지
CAN_ID_STOP     = 0x211   # (임의) 수신 시 LKAS 즉시 정지
CAN_ID_STEER    = 0x321   # <-- 조향 명령 보내는 ID (추가)
SEND_HZ         = 30

# ===== Lane 파라미터 =====
ROI_TOP_Y   = 0.55
ROI_TOP_X_L = 0.03
ROI_TOP_X_R = 0.97

# Canny
CANNY_LOW   = 200
CANNY_HIGH  = 255

# Hough (1차/2차)
HOUGH1_THR, HOUGH1_MINLEN, HOUGH1_GAP = 50, 0.15, 0.05
HOUGH2_THR, HOUGH2_MINLEN, HOUGH2_GAP = 30, 0.08, 0.08

# 라인 각도 필터
ANGLE_MIN_DEG = 10
ANGLE_MAX_DEG = 89

# 오프셋 계산 기준 y
Y_REF_RATIO   = 0.95

# 오프셋 EMA
EMA_ALPHA     = 0.20

# ===== 단일 차선 처리 정책 =====
SINGLE_SIDE_POLICY       = "fixed"     # "fixed" | "width" | "alpha"
FIXED_OFFSET_ABS         = 140
DEFAULT_LANE_WIDTH_RATIO = 0.35
LONE_ALPHA               = 0.55
SLOPE_DECIDE_SINGLE      = True        # 한 줄만 남으면 m>0 → + / m<0 → -

# 필요 시 오프셋 바이어스(px)
OFFSET_BIAS = 0

# === “가까움 + 기울기 유사” 중복제거/선택 튜닝 ===
SLOPE_SIM_DEG      = 8.0
X_PROX_FACTOR      = 0.10
Y_PROX_FACTOR      = 0.18
MIN_SEP_FACTOR     = 0.20
BOTTOM_NEAR_FACTOR = 0.18

# ===== 조향 변환 =====
STEER_RANGE = 100
KP          = 450.0
DEADBAND_PX = 4
LPF_ALPHA   = 0.35

# ===== 시작 동작 =====
STARTUP_WARMUP_FRAMES = 6
STARTUP_ZERO_MODE = 'bias'   # 'bias' 또는 'hold'

# =============================
# ===== 내부 상태 ============
# =============================
_state = {
    "lane_width_px": None,
    "offset_px": 0,
    "offset_px_ema": 0.0,
    "zero_bias_px": None,
}
_boot = { "warmup": STARTUP_WARMUP_FRAMES }

# =============================
# ===== 유틸 함수 ============
# =============================
def _build_roi_mask(h, w):
    mask = np.zeros((h, w), np.uint8)
    top_y = int(h * ROI_TOP_Y)
    poly = np.array([[
        (int(w*ROI_TOP_X_L), top_y), (int(w*ROI_TOP_X_R), top_y),
        (w-1, h-1), (0, h-1)
    ]], np.int32)
    cv2.fillPoly(mask, poly, 255)
    return mask

def _angle_deg(x1,y1,x2,y2):
    dx, dy = x2-x1, y2-y1
    return abs(math.degrees(math.atan2(dy, dx)))

def _run_hough(edges, w, thr, minlen, gap):
    return cv2.HoughLinesP(
        edges, 1, np.pi/180, threshold=thr,
        minLineLength=int(w * minlen),
        maxLineGap=int(w * gap)
    )

def _seg_to_mb(seg):
    x1,y1,x2,y2 = seg
    if y2 == y1:
        return None
    m = (x2 - x1) / float(y2 - y1)  # x = m*y + b
    b = x1 - m * y1
    return float(m), float(b)

def _x_at_y(mb, y):
    m,b = mb
    return float(m*y + b)

def _mb_to_angle_deg(mb):
    m,_ = mb
    rad = math.atan2(1.0, m if m!=0 else 1e-6)  # dy/dx = 1/m
    ang = abs(math.degrees(rad))
    return ang if ang <= 90 else 180-ang

# =============================
# ===== 오프셋 검출 ==========
# =============================
def detect_offset(img):
    h, w = img.shape[:2]
    cx_img = w // 2
    y_ref  = int(h * Y_REF_RATIO)

    # === 전처리: HLS 화이트 → Blur → Canny ===
    hls   = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)
    white = cv2.inRange(hls, (0,200,0), (255,255,255))
    roi_mask = _build_roi_mask(h, w)
    white = cv2.bitwise_and(white, roi_mask)
    blur  = cv2.GaussianBlur(white, (5,5), 0)
    edges = cv2.Canny(blur, CANNY_LOW, CANNY_HIGH)

    k3 = cv2.getStructuringElement(cv2.MORPH_RECT, (3,3))
    edges = cv2.dilate(edges, k3, 1); edges = cv2.erode(edges,  k3, 1)

    edges[:int(h*ROI_TOP_Y)+2, :] = 0
    edges[:, :6]  = 0
    edges[:, -6:] = 0

    lines = _run_hough(edges, w, HOUGH1_THR, HOUGH1_MINLEN, HOUGH1_GAP)
    if lines is None or len(lines) < 4:
        more = _run_hough(edges, w, HOUGH2_THR, HOUGH2_MINLEN, HOUGH2_GAP)
        lines = more if lines is None else np.vstack([lines, more]) if more is not None else lines

    cands = []
    if lines is not None:
        for x1,y1,x2,y2 in lines[:,0,:]:
            a = _angle_deg(x1,y1,x2,y2)
            if a < ANGLE_MIN_DEG or a > ANGLE_MAX_DEG:
                continue
            MIN_Y_SPAN = max(10, int(0.012*h))
            MIN_LEN_PX = max(25, int(0.03*w))
            dx, dy = x2-x1, y2-y1
            seg_len = math.hypot(dx, dy)
            if abs(y1-y2) < MIN_Y_SPAN or seg_len < MIN_LEN_PX:
                continue

            mb = _seg_to_mb((x1,y1,x2,y2))
            if mb is None:
                continue

            cands.append({
                "seg":  (x1,y1,x2,y2),
                "mb":   mb,
                "x_ref": _x_at_y(mb, y_ref),
                "ymax":  max(y1,y2),
                "len":   seg_len,
                "ang":   _mb_to_angle_deg(mb),
            })

    reps = []
    if cands:
        cands.sort(key=lambda d:(d["ymax"], d["len"]), reverse=True)
        x_thr = w * X_PROX_FACTOR
        y_thr = h * Y_PROX_FACTOR

        for c in cands:
            merged = False
            for i, r in enumerate(reps):
                if abs(c["x_ref"] - r["x_ref"]) <= x_thr and \
                   abs(c["ymax"]  - r["ymax"])  <= y_thr and \
                   abs(c["ang"]   - r["ang"])   <= SLOPE_SIM_DEG:
                    if (c["ymax"], c["len"]) > (r["ymax"], r["len"]):
                        reps[i] = c
                    merged = True
                    break
            if not merged:
                reps.append(c)

    chosen = []
    if reps:
        reps.sort(key=lambda d: d["x_ref"])
        best = max(reps, key=lambda d:(d["ymax"], d["len"]))
        partner = None
        min_sep_px = w * MIN_SEP_FACTOR
        for r in reps:
            if abs(r["x_ref"] - best["x_ref"]) >= min_sep_px:
                if partner is None or abs(r["x_ref"] - best["x_ref"]) > abs(partner["x_ref"] - best["x_ref"]):
                    partner = r
        if partner is not None:
            chosen = [best, partner]
            chosen.sort(key=lambda d: d["x_ref"])
        else:
            chosen = [best]

    offset_px = None
    quality   = 0
    flags     = 0

    if len(chosen) == 2:
        xL = _x_at_y(chosen[0]["mb"], y_ref)
        xR = _x_at_y(chosen[1]["mb"], y_ref)
        if xL > xR: xL, xR = xR, xL
        lane_center = 0.5*(xL + xR)
        offset_px   = int(round((cx_img - lane_center) - OFFSET_BIAS))
        lane_w_px   = xR - xL
        _state["lane_width_px"] = lane_w_px if _state["lane_width_px"] is None \
                                  else 0.8*_state["lane_width_px"] + 0.2*lane_w_px
        quality = 900; flags |= 0b00000011

    elif len(chosen) == 1:
        one = chosen[0]
        m,_ = one["mb"]
        x1  = one["x_ref"]

        if SLOPE_DECIDE_SINGLE:
            offset_px = (+abs(FIXED_OFFSET_ABS) if m > 0 else -abs(FIXED_OFFSET_ABS))
            offset_px = int(offset_px - OFFSET_BIAS)
            quality   = 650; flags |= 0b00000100
            flags    |= 0b00000010 if x1 >= cx_img else 0b00000001
        else:
            if SINGLE_SIDE_POLICY == "fixed":
                offset_px = (+abs(FIXED_OFFSET_ABS) if x1 >= cx_img else -abs(FIXED_OFFSET_ABS))
                offset_px = int(offset_px - OFFSET_BIAS)
            elif SINGLE_SIDE_POLICY == "width":
                lane_w_est = _state["lane_width_px"] if _state["lane_width_px"] is not None else (DEFAULT_LANE_WIDTH_RATIO*w)
                lane_w_est = float(np.clip(lane_w_est, 0.20*w, 0.80*w))
                lane_center = (x1 - lane_w_est*0.5) if x1 >= cx_img else (x1 + lane_w_est*0.5)
                offset_px = int(round((cx_img - lane_center) - OFFSET_BIAS))
            else:
                lane_center = (1.0 - LONE_ALPHA)*cx_img + LONE_ALPHA*x1 if x1 >= cx_img \
                              else LONE_ALPHA*x1 + (1.0 - LONE_ALPHA)*cx_img
                offset_px = int(round((cx_img - lane_center) - OFFSET_BIAS))
            quality = 600; flags |= 0b00000100
            flags   |= 0b00000010 if x1 >= cx_img else 0b00000001

    else:
        offset_px = _state["offset_px"]
        quality = 150; flags |= 0b00001000

    _state["offset_px"] = offset_px
    if _state["offset_px_ema"] == 0.0:
        _state["offset_px_ema"] = float(offset_px)
    else:
        _state["offset_px_ema"] = EMA_ALPHA*float(offset_px) + (1.0-EMA_ALPHA)*_state["offset_px_ema"]
    ema_val = int(round(_state["offset_px_ema"]))

    if _boot["warmup"] > 0:
        flags |= 0b00010000
        if STARTUP_ZERO_MODE == 'hold':
            shown = 0
        else:
            if _state["zero_bias_px"] is None:
                _state["zero_bias_px"] = ema_val
            shown = ema_val - _state["zero_bias_px"]
    else:
        if STARTUP_ZERO_MODE == 'bias' and _state["zero_bias_px"] is not None:
            shown = ema_val - _state["zero_bias_px"]
        else:
            shown = ema_val

    offset_norm = shown / float(w)
    return { "offset_px": shown, "offset_norm": offset_norm, "quality": quality, "flags": flags }

# ==============================
# ===== 조향 변환 & CAN =====
# ==============================
def clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v

def offset_to_steer(offset_px, img_width, prev_steer=None):
    if offset_px is None:
        return 0 if prev_steer is None else int(0.8*prev_steer)
    if abs(offset_px) <= DEADBAND_PX:
        target = 0
    else:
        target = (offset_px * KP) / float(img_width)
    target = clamp(int(round(target)), -STEER_RANGE, +STEER_RANGE)
    if prev_steer is None:
        return target
    return clamp(int(round(LPF_ALPHA*target + (1-LPF_ALPHA)*prev_steer)),
                 -STEER_RANGE, +STEER_RANGE)

def drain_can(bus):
    """
    0x210: data[0]==1 → enable, data[0]==0 → disable
    0x211: (payload 없거나 data[0]==1) → 강제 disable
    반환값: None(변화 없음) | True(활성) | False(비활성)
    """
    enabled = None
    while True:
        msg = bus.recv(timeout=0.0)
        if msg is None:
            break
        if msg.arbitration_id == CAN_ID_TRIGGER and len(msg.data) >= 1:
            enabled = (msg.data[0] == 1)
        elif msg.arbitration_id == CAN_ID_STOP:
            # data 비어있어도 중지, 있으면 data[0]==1일 때 중지
            if len(msg.data) == 0 or msg.data[0] == 1:
                enabled = False
    return enabled

# =========================
# ===== Main Loop =====
# =========================
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--device", default="/dev/video0")
    ap.add_argument("--no-gui", action="store_true")
    args = ap.parse_args()
    NO_GUI = args.no_gui

    bus = can.Bus(interface="socketcan", channel=CAN_IFACE)
    cap = cv2.VideoCapture(args.device, cv2.CAP_V4L2)
    if not cap.isOpened():
        print(f"카메라 열기 실패: {args.device}")
        return
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,1280); cap.set(cv2.CAP_PROP_FRAME_HEIGHT,720); cap.set(cv2.CAP_PROP_FPS,30)

    lkas_enabled = False
    last_send    = 0.0
    prev_steer   = 0
    send_period  = 1.0 / SEND_HZ

    _state["offset_px"] = 0
    _state["offset_px_ema"] = 0.0
    _state["zero_bias_px"] = None
    _boot["warmup"] = STARTUP_WARMUP_FRAMES

    print("[LKAS Ready]" + (" (headless)" if NO_GUI else " q:종료"))

    try:
        while True:
            en = drain_can(bus)
            if en is not None:
                lkas_enabled = en

            ok, frame = cap.read()
            if not ok:
                if NO_GUI: time.sleep(0.001)
                continue

            if lkas_enabled:
                res = detect_offset(frame)
                offset_px = res["offset_px"]
                steer = offset_to_steer(offset_px, frame.shape[1], prev_steer); prev_steer = steer

                now = time.time()
                if now - last_send >= (1.0 / SEND_HZ):
                    if _boot["warmup"] > 0:
                        can_offset = 0
                        _boot["warmup"] -= 1
                    else:
                        can_offset = offset_px

                    canData = max(-200, min(200, int(can_offset)))
                    canData = int((canData + 200) / 2)
                    bus.send(can.Message(arbitration_id=CAN_ID_STEER, data=pack('B', canData), is_extended_id=False))
                    last_send = now

                if not NO_GUI:
                    overlay = frame.copy()
                    cv2.putText(
                        overlay,
                        f"RUN steer={steer:+d} off={offset_px:+d} q={res['quality']} flg=0x{res['flags']:02X} "
                        f"bias={_state['zero_bias_px']} (slope={'Y' if SLOPE_DECIDE_SINGLE else 'N'})",
                        (12,32), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,255,0), 2
                    )
                    cv2.imshow("LKAS", overlay)
            else:
                if not NO_GUI:
                    overlay = frame.copy()
                    cv2.putText(overlay,
                                f"IDLE (start:0x210[1], stop:0x211) warmup={_boot['warmup']} bias={_state['zero_bias_px']}",
                                (12,32), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,0,255), 2)
                    cv2.imshow("LKAS", overlay)

            if not NO_GUI:
                if (cv2.waitKey(1) & 0xFF) == ord('q'):
                    break
            else:
                time.sleep(0.001)

    finally:
        try: bus.shutdown()
        except: pass
        cap.release()
        if not NO_GUI: cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
