#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <wiringPiSPI.h>
#include <math.h>

#define BAUD_RATE 115200    //블루투스의 보율이 바뀔 경우 이 값을 변경해야함
static const char *UART2_DEV = "/dev/ttyAMA1"; // UART2 연결을 위한 장치 파일
unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);
void serialWriteBytes(const int fd, const char *s);

//여러 바이트의 데이터를 씀
void serialWriteBytes(const int fd, const char *s)
{
    write(fd, s, strlen(s));
}

// 1바이트 데이터를 읽음
unsigned char serialRead(const int fd)
{
    unsigned char x;
    if (read(fd, &x, 1) != 1) // read 함수를 통해 1바이트 읽어옴
        return -1;
    return x; //읽어온 데이터 반환
}

// 1바이트 데이터를 씀
void serialWrite(const int fd, const unsigned char c)
{
    write(fd, &c, 1); // write 함수를 통해 1바이트 씀
}

int main()
{
    int fd_serial;     // UART2 파일 서술자
    unsigned char dat; //데이터 임시 저장 변수

    if (wiringPiSetupGpio() == -1)
        return 1;
    
    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0)
    { // UART2 포트 오픈
        printf("Unable to open serial device.\n");
        return 1;
    }
    pinMode(18, OUTPUT);

    while (1)
    {
        if (serialDataAvail(fd_serial))
        {                                //읽을 데이터가 존재한다면,
            dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
            
            printf("%c", dat);
            fflush(stdout);
            if(dat != '0' || dat != '1' )
                continue;
            else if(dat == '1') //security mode
                
            else if(dat == '0') //general mode
                
        }
        delay(10);
    }    
}