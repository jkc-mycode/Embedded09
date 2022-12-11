#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <wiringPiSPI.h>
#include <math.h>
#include <fcntl.h>

#define BAUD_RATE 115200    //블루투스의 보율이 바뀔 경우 이 값을 변경해야함
#define GPIO_SPEAKER 23
#define GPIO_LED 24
#define TRUE 1
#define FALSE 0

static const char *UART2_DEV = "/dev/ttyAMA1"; // UART2 연결을 위한 장치 파일

unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);
void serialWriteBytes(const int fd, const char *s);

void Speaker_LED_OnOff(int sp);

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

void Speaker_LED_OnOff(int sp){
    pinMode(GPIO_SPEAKER, OUTPUT);
    pinMode(GPIO_LED, OUTPUT);

    if(sp){
        //스피커 및 LED 동작
        digitalWrite(GPIO_SPEAKER, HIGH);
        for(int i=0; i<10; i++){ //LED 점멸
            digitalWrite(GPIO_LED, HIGH);
            delay(500);
            digitalWrite(GPIO_LED, LOW);
            delay(500);
        }
    }else{
        //스피커 및 LED 끔
        digitalWrite(GPIO_SPEAKER, LOW);
        digitalWrite(GPIO_LED, LOW);
    }
}

int main()
{
    int fd_serial;     // UART2 파일 서술자
    unsigned char dat; //데이터 임시 저장 변수
    FILE* fp; //파일 입출력 변수
    char temp; //기록 내용 담을 임시변수
    char file_buffer[100];

    if (wiringPiSetupGpio() == -1)
        return 1;
    
    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0)
    { // UART2 포트 오픈
        printf("Unable to open serial device.\n");
        return 1;
    }

    while (1)
    {
        if (serialDataAvail(fd_serial))
        {                                //읽을 데이터가 존재한다면,
            dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
            
            switch(dat){
                case '0': //일반모드
                    //초음파 센서 일반모드 함수 실행
                    break;
                case '1': //보안모드
                    //초음파 센서 보안모드 함수 실행
                    break;
                case '2': //초기설정 모드
                    //초음파 센서 초기설정 함수 실행
                    break;
                case '3': //기록 파일 확인
                    if((fp = fopen("./pass_history.txt", "r")) == NULL){ //파일 열리는지 확인
                        return 1; 
                    }
                    while((fgets(file_buffer, sizeof(file_buffer), fp)) != NULL){ //파일이 끝날때 까지 반복
                        serialWriteBytes(fd_serial, file_buffer); //휴대폰으로 파일의 할줄씩 출력
                        printf("%s", file_buffer);
                        memset(file_buffer, 0, sizeof(file_buffer)); //메모리 초기화
                    }
                    fclose(fp);
                    break;
                case '4': //기록 파일 초기화 (파일 내용 지움)
                    if((fp = fopen("./pass_history.txt", "w")) == NULL){ //파일 열리는지 확인
                        return 1; 
                    }
                    fputs(" ", fp);
                    fclose(fp);
                    break;
                case '5': //경보 끄기
                    //경보 on/off 할 함수 추가
                    Speaker_LED_OnOff(FALSE);
                    break;
            }
        }
        delay(10);
    }    
}
