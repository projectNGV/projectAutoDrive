#include "bluetooth.h"


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asclin1.h"

#include "Ifx_Types.h"
#define BUFSIZE     128
#define KB_BS '\x7F'
#define KB_CR '\r'


void bluetoothIsr(char c)
{
//    myPrintf("%c", c);
}

void bluetoothInit(void)
{
    asclin1InitUart();
}

void bluetoothAtCommand(char *cmd)
{
    char buf[30];
    int i = 0;
    sprintf(buf, "%s", cmd);
    while (buf[i] != 0)
    {
        asclin1OutUart(buf[i]);
        i++;
    }
    asclin1OutUart(buf[i]);
//    asclin1OutUart('\r');
//    asclin1OutUart('\n');

    delayMs(300);
}

char bluetoothRecvByteBlocked(void)
{
    return asclin1InUart();
}

char bluetoothRecvByteNonBlocked(void)
{
    unsigned char ch = 0;
    int res;
    res = asclin1PollUart(&ch);

    return res == 1 ? ch : -1;
}

void bluetoothSendByteBlocked(unsigned char ch)
{
    asclin1OutUart(ch);
}

void bluetoothPrintf(const char *fmt, ...)
{
    char buffer[128];
    char buffer2[128]; // add \r before \n
    char *ptr;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);
    int j = 0;
    for (int i = 0; buffer[i]; i++)
    {
        if (buffer[i] == '\n')
        {
            buffer2[j++] = '\r';
            buffer2[j++] = buffer[i];
        }
        else
        {
            buffer2[j++] = buffer[i];
        }
    }
    buffer2[j] = '\0';

    for (ptr = buffer2; *ptr; ++ptr)
        bluetoothSendByteBlocked((const unsigned char) *ptr);
}




void bluetoothScanf(const char *fmt, ...)
{
    uint8 c = 0;
    char buf[128];
    int idx = 0, i;
    char *pstr, *pidx;

    memset(buf, 0, 128);
    while (c != '\r')
    {
        c = asclin1InUart();
        if (c == KB_BS || c == 0x8)
        {
            if (idx > 0) {
                buf[idx - 1] = 0;
                idx--;
                bluetoothPrintf("%c %c", 8,8);
            }
            continue;
        }
        else
        {
            if (c == KB_CR) {
                idx += 1;
                buf[idx] = '\0';
            } else {
                buf[idx] = c;
                idx += 1;
            }
        }
        bluetoothPrintf("%c", c);
    }
    bluetoothPrintf("\n");

    va_list ap;
    va_start(ap, fmt);
    while ((c = *fmt++) != 0)
    {
        if (c == '%')
        {
            uint8 c1;
            c = *fmt++;
            switch (c)
            {
                case 'c':
                    *va_arg(ap, char *) = buf[0];
                    buf[0] = '\0';
                    break;
                case 'd':
                    *va_arg(ap, int *) = atoi(buf);
                    pidx = strchr(buf, ' ');
                    if (pidx != NULL) { *pidx = '\0'; }
                    for (i = 0; ; i++)
                    {
                        if (buf[i] == '\0' || buf[i] == ' ') { buf[i] = '\0'; break; }
                        buf[i] = '\0';
                    }
                    break;
                case 's':
                    pstr = va_arg(ap, char *);
                    for (i = 0; buf[i] != '\0'; i++)
                    {
                        *pstr++ = buf[i];
                        buf[i] = '\0';
                    }
                    *pstr = '\0';
                    break;
                case 'l':
                    c1 = *fmt++;
                    if (c1 == 'd') {
                        *va_arg(ap, long long *) = atoll(buf);
                        pidx = strchr(buf, ' ');
                        if (pidx != NULL) { *pidx = '\0'; }
                        for (i = 0; ; i++)
                        {
                            if (buf[i] == '\0' || buf[i] == ' ') { buf[i] = '\0'; break; }
                            buf[i] = '\0';
                        }
                    } else if (c1 == 'f') {
                        *va_arg(ap, double *) = atof(buf);
                        pidx = strchr(buf, ' ');
                        if (pidx != NULL) { *pidx = '\0'; }
                        for (int i = 0; ; i++)
                        {
                            if (buf[i] == '\0' || buf[i] == ' ') { buf[i] = '\0'; break; }
                            buf[i] = '\0';
                        }
                    }
                    break;
                case 'f':
                    *va_arg(ap, float *) = (float)(atof(buf));
                    pidx = strchr(buf, ' ');
                    if (pidx != NULL) { *pidx = '\0'; }
                    for (i = 0; ; i++)
                    {
                        if (buf[i] == '\0' || buf[i] == ' ') { buf[i] = '\0'; break; }
                        buf[i] = '\0';
                    }
                    break;
                default:
                    break;
            }
            remove_null(buf);
        }
    }
    va_end(ap);
}

static void remove_null(char *s)
{
    int i, start_idx;
    char buf[128];

    start_idx = 0;
    while (*(s + start_idx++) == '\0');
    memset(buf, 0, 128);
    strcpy(buf, (s + (start_idx - 1)));

    memset(s, 0, 128);
    i = 0;
    while (buf[i] != '\0')
    {
        *(s + i) = buf[i];
        ++i;
    }
    *(s + i) = '\0';
}



