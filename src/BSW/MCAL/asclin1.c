#include "asclin1.h"
#include "main0.h"

extern MotorState motorState;
extern volatile boolean g_isLogin;

volatile boolean g_rx_getLine;
volatile char g_rx_buffer[RX_BUFFER_SIZE];
static int rx_idx = 0;

void rxBufferFlush (void);

void rxBufferFlush (void)
{
    rx_idx = 0;
    memset((void*) g_rx_buffer, 0, RX_BUFFER_SIZE);
    g_rx_getLine = FALSE;
}

IFX_INTERRUPT(asclin1RxIsrHandler, 0, ISR_PRIORITY_ASCLIN1_RX);
void asclin1RxIsrHandler (void)
{
    char data = asclin1InUart();
//    if (!g_isLogin)
//    {
//        if (data == '\n' || data == '\r')
//        {
//            g_rx_buffer[rx_idx] = '\0';
//            g_rx_getLine = TRUE;
//        }
//        else
//        {
//            if (rx_idx < (RX_BUFFER_SIZE - 1))
//            {
//                g_rx_buffer[rx_idx++] = data;
//            }
//        }
//    }
//    else
//    {
    asclin0OutUart(data);
        motorState.lastKeyInput = data;

//    }
}

/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
/* ASCLIN1 for mikroBUS */
void asclin1InitUart (void)
{
    /* set numerator and denominator for 9600 baudrate */
//    unsigned int numerator = 48; // 9600
    unsigned int numerator = 576; // 115200

    unsigned int denominator = 3125;

    /* RXA/P15.1, TX/P15.0 */
    /* Set TX/P15.0 to "output" and "high" */
    MODULE_P15.IOCR0.B.PC0 = 0x12;
    MODULE_P15.OUT.B.P0 = 1;

    /* Enable ASCn */
    IfxScuWdt_clearCpuEndinit(IfxScuWdt_getGlobalEndinitPassword());
    MODULE_ASCLIN1.CLC.U = 0;
    IfxScuWdt_setCpuEndinit(IfxScuWdt_getGlobalEndinitPassword());
    /* read back for activating module */
    (void) MODULE_ASCLIN1.CLC.U;

    /* select RX as input pin */
    MODULE_ASCLIN1.IOCR.B.ALTI = 0; // Select Alternate Input A

    /* Program ASC1 */
    MODULE_ASCLIN1.CSR.U = 0;

    /* configure TX and RX FIFOs */
    MODULE_ASCLIN1.TXFIFOCON.U = (1 << 6) /* INW: (1 == 1 byte) */
    | (1 << 1) /* ENO */
    | (1 << 0); /* FLUSH */
    MODULE_ASCLIN1.RXFIFOCON.U = (1 << 31) /* BUF: (1 == Single Stage RX Buffer) */
    | (1 << 6) /* OUTW: (1 == 1 byte) */
    | (1 << 1) /* ENI */
    | (1 << 0); /* FLUSH */

    MODULE_ASCLIN1.BITCON.U = (9 << 0) /* PRESCALER: 10 */
    | (15 << 16) /* OVERSAMPLING: 16 */
    | (9 << 24) /* SAMPLEPOINT: position 7,8,9 */
    | (1u << 31); /* SM: 3 samples per bit */
    /* data format: 8N1 */
    MODULE_ASCLIN1.FRAMECON.U = (1 << 9) /* STOP: 1 bit */
    | (0 << 16) /* MODE: Init */
    | (0 << 30); /* PEN: no parity */
    MODULE_ASCLIN1.DATCON.U = (7 << 0); /* DATLEN: 8 bit */

    /* set baudrate value */
    MODULE_ASCLIN1.BRG.U = (denominator << 0) /* DENOMINATOR */
    | (numerator << 16); /* NUMERATOR */

    MODULE_ASCLIN1.FRAMECON.B.MODE = 1; /* ASC mode */
    MODULE_ASCLIN1.CSR.U = 1; /* select CLC as clock source */

    MODULE_ASCLIN1.FLAGSSET.U = (IFX_ASCLIN_FLAGSSET_TFLS_MSK << IFX_ASCLIN_FLAGSSET_TFLS_OFF);

    /* Initialize ASCLIN0 RX interrupt */
    volatile Ifx_SRC_SRCR *src;
    src = (volatile Ifx_SRC_SRCR*) (&MODULE_SRC.ASCLIN.ASCLIN[1].RX);
    src->B.SRPN = ISR_PRIORITY_ASCLIN1_RX;
    src->B.TOS = 0;
    src->B.CLRR = 1; /* clear request */
    MODULE_ASCLIN1.FLAGSENABLE.B.RFLE = 1; /* enable rx fifo fill level flag */
    src->B.SRE = 1; /* interrupt enable */
}

/* Send character CHR via the serial line */
void asclin1OutUart (const unsigned char chr)
{
    /* wait until space is available in the FIFO */
    while (!(MODULE_ASCLIN1.FLAGS.B.TFL != 0))
        ;

    /* TX Clear */
    MODULE_ASCLIN1.FLAGSCLEAR.U = (IFX_ASCLIN_FLAGSCLEAR_TFLC_MSK << IFX_ASCLIN_FLAGSCLEAR_TFLC_OFF);

    /* send the character */
    MODULE_ASCLIN1.TXDATA.U = chr;
}

/* Receive (and wait for) a character from the serial line */
unsigned char asclin1InUart (void)
{
    unsigned char ch;

    /* wait for a new character */
    while (asclin1PollUart(&ch) == 0)
        ;

    return ch;
}

/* Check the serial line if a character has been received.
 returns 1 and the character in *chr if there is one
 else 0
 */
int asclin1PollUart (unsigned char *chr)
{
    unsigned char ret;
    int res = 0;

    if (MODULE_ASCLIN1.FLAGS.B.RFL != 0) /* If RX Ready */
    {
        ret = (unsigned char) MODULE_ASCLIN1.RXDATA.U;
        /* acknowledge receive */
        MODULE_ASCLIN1.FLAGSCLEAR.U = (IFX_ASCLIN_FLAGSCLEAR_RFLC_MSK << IFX_ASCLIN_FLAGSCLEAR_RFLC_OFF); // RX Clear
        /* check for error condition */
        if ((MODULE_ASCLIN1.FLAGS.U)
                & ((IFX_ASCLIN_FLAGS_PE_MSK << IFX_ASCLIN_FLAGS_PE_OFF)
                        | (IFX_ASCLIN_FLAGS_FE_MSK << IFX_ASCLIN_FLAGS_FE_OFF)
                        | (IFX_ASCLIN_FLAGS_RFO_MSK << IFX_ASCLIN_FLAGS_RFO_OFF)))
        {
            /* reset error flags */
            MODULE_ASCLIN1.FLAGSCLEAR.U = ((IFX_ASCLIN_FLAGSCLEAR_PEC_MSK << IFX_ASCLIN_FLAGSCLEAR_PEC_OFF)
                    | (IFX_ASCLIN_FLAGSCLEAR_FEC_MSK << IFX_ASCLIN_FLAGSCLEAR_FEC_OFF)
                    | (IFX_ASCLIN_FLAGSCLEAR_RFOC_MSK << IFX_ASCLIN_FLAGSCLEAR_RFOC_OFF));
            /* Buffer clear, reset module */
        }
        else
        {
            /* this is a valid character */
            *chr = ret;
            res = 1;
        }
    }

    return res;
}
