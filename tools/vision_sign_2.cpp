// red2can.cpp
#include <opencv2/opencv.hpp>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace cv;
using namespace std;

// ===== CAN 설정 =====
static const char*  CAN_IFACE   = "can0";
static const canid_t CAN_ID_RED  = 0x0D4;  // 212 decimal
static const uint8_t CAN_DATA    = 0x01;   // 1바이트 페이로드

struct Args{
    int camIndex=0, width=1280, height=720, fps=30;
    int debug=1;          // 1: 디버그 창 표시
    int streak=2;         // 연속 N프레임 감지 시 송신
    double area=1200.0;   // 최소 면적 기준
};

Args parseArgs(int argc, char** argv){
    Args a;
    for(int i=1;i<argc;i++){
        string s=argv[i];
        auto pickI=[&](const string& k,int& d){ if(s==k && i+1<argc) d=atoi(argv[++i]); };
        auto pickD=[&](const string& k,double& d){ if(s==k && i+1<argc) d=atof(argv[++i]); };
        pickI("--camera", a.camIndex);
        pickI("--width",  a.width);
        pickI("--height", a.height);
        pickI("--fps",    a.fps);
        pickI("--debug",  a.debug);
        pickI("--streak", a.streak);
        pickD("--area",   a.area);
    }
    return a;
}

int openCan(const char* ifname=CAN_IFACE){
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(s < 0) { perror("socket"); return -1; }
    sockaddr_can addr{}; addr.can_family = AF_CAN; addr.can_ifindex = if_nametoindex(ifname);
    if(addr.can_ifindex == 0){ perror("if_nametoindex"); close(s); return -1; }
    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){ perror("bind"); close(s); return -1; }
    return s;
}

bool sendRedOnce(int sock){
    if(sock < 0) return false;
    struct can_frame f{}; f.can_id = CAN_ID_RED; f.can_dlc = 1; f.data[0] = CAN_DATA;
    ssize_t n = write(sock, &f, sizeof(f));
    if(n != (ssize_t)sizeof(f)){ perror("can write"); return false; }
    printf("[CAN SEND] ID=0x%03X, Data=[0x%02X]\n", (unsigned)CAN_ID_RED, CAN_DATA);
    return true;
}

static inline void drawText(Mat& img, const string& t, Point org, double scale=0.8, int thick=2, Scalar fg=Scalar(255,255,255)){
    putText(img, t, org, FONT_HERSHEY_SIMPLEX, scale, Scalar(0,0,0), thick+2, LINE_AA);
    putText(img, t, org, FONT_HERSHEY_SIMPLEX, scale, fg, thick, LINE_AA);
}

int main(int argc, char** argv){
    Args args = parseArgs(argc, argv);

    // ===== 카메라 =====
    VideoCapture cap;
    cap.open(args.camIndex, cv::CAP_V4L2);   // V4L2 우선
    if(!cap.isOpened()) cap.open(args.camIndex);
    if(!cap.isOpened()){
        fprintf(stderr, "Camera open failed\n");
        return 1;
    }
    cap.set(CAP_PROP_FRAME_WIDTH,  args.width);
    cap.set(CAP_PROP_FRAME_HEIGHT, args.height);
    cap.set(CAP_PROP_FPS,          args.fps);

    // ===== CAN =====
    int canSock = openCan(CAN_IFACE);
    if(canSock < 0){
        fprintf(stderr, "CAN open failed (will continue without sending)\n");
    }

    // ===== HSV 빨간색 구간 (BGR→HSV 기준) =====
    // 조도가 낮으면 S,V 하한을 더 낮추세요: 80→60, 60→50 등
    Scalar redLow1 (  0,  80, 60), redHigh1( 10,255,255);
    Scalar redLow2 (170,  80, 60), redHigh2(180,255,255);

    const double AREA_THR = args.area;
    const int    STREAK_N = max(1, args.streak);

    int streak = 0;
    bool sentLatch = false;  // 감지되면 1회 송신, 사라지면 해제

    if(args.debug){
        namedWindow("cam",  WINDOW_NORMAL);
        namedWindow("mask", WINDOW_NORMAL);
        resizeWindow("cam",  960, 540);
        resizeWindow("mask", 480, 270);
    }

    Mat frame, hsv, m1, m2, mask;
    Mat k = getStructuringElement(MORPH_ELLIPSE, Size(5,5));

    while(true){
        if(!cap.read(frame)) continue;     // frame은 BGR

        // === 파이프라인: BGR → HSV → inRange → Morph ===
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, redLow1, redHigh1, m1);
        inRange(hsv, redLow2, redHigh2, m2);
        bitwise_or(m1, m2, mask);

        morphologyEx(mask, mask, MORPH_OPEN,  k);
        morphologyEx(mask, mask, MORPH_CLOSE, k);

        // 최대 컨투어 면적
        vector<vector<Point>> cs;
        findContours(mask, cs, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        double maxArea = 0.0;
        int maxi = -1;
        for(int i=0;i<(int)cs.size();++i){
            double a = contourArea(cs[i]);
            if(a > maxArea){ maxArea = a; maxi = i; }
        }

        bool detected = (maxArea > AREA_THR);
        if(detected){
            streak++;
            if(streak >= STREAK_N && !sentLatch){
                sendRedOnce(canSock);   // ← 여기서 실제 CAN 송신
                sentLatch = true;
            }
        }else{
            streak = 0;
            if(sentLatch){
                printf("[RED] Lost → latch released\n");
            }
            sentLatch = false;
        }

        if(args.debug){
            Mat vis = frame.clone();
            if(detected && maxi >= 0){
                Rect r = boundingRect(cs[maxi]);
                rectangle(vis, r, Scalar(0,0,255), 2);
                char buf[64]; snprintf(buf, sizeof(buf), "R area=%.0f", maxArea);
                drawText(vis, buf, Point(20,40), 1.0, 2, Scalar(0,0,255));
            }else{
                drawText(vis, "NO RED", Point(20,40), 1.0, 2, Scalar(0,255,255));
            }
            imshow("cam",  vis);
            imshow("mask", mask);
            int k = waitKey(1);
            if(k==27 || k=='q') break;
        }else{
            if(waitKey(1)==27) break;
        }
    }

    if(canSock >= 0) close(canSock);
    return 0;
}
