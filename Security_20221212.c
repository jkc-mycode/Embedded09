#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#define BAUD_RATE 115200 
#define SONIC_Trig  18
#define SONIC_Echo  4
#define BUZZER 23
#define LED 24
#define BT_RXD 1
#define BT_TXD 0

int SIGNAL = 0;     // bluetooth가 전송하는 설정 요구 값
bool MODE = 0;      //모드 전역변수, bluetooth에서 직접 접근
bool WARNING = 0;   //보안 모드에서 현재 경보가 켜져있는지 아닌지
int TIME_MODE = 0;  // 0 = day, 1 = hour

//ultra sonic sensor
float Get_Range();
void Ultrasonic_Sensor(float v_range, struct tm* comp_time);

//
void Alert_On(struct tm* m_time);
void Alert_Off();
void Counter(struct tm* m_time,struct tm* comp, int *m_cnt);

static const char *UART2_DEV = "/dev/ttyAMA1"; // UART2 연결을 위한 장치 파일

void Bluetooth()
unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);
void serialWriteBytes(const int fd, const char *s);

//외부 입력 처리


int main(){
    float full_range = 0;
    float valid_range = 0;
    float margin = 15.0f;

    //시간 기록을 위한 변수 (비갱신)
    time_t sys_time = time(NULL);
    struct tm checktime = *localtime(&sys_time);
    
    if(wiringPiSetupGpio() < 0){
        printf("wiringPiSetupGpio failed \n");
        return -1;
    }
    if(wiringPiI2CSetupInterface() < 0){
        printf("wiringPiI2CSetUpInterface failed \n");
        return -1;
    }
    printf("wiringPi Setup Complete \n");

    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(SONIC_Trig, OUTPUT);
    pinMode(SONIC_Echo, INPUT);

    //거리 측정 (초기 셋팅)
    full_range = Get_Range();
    valid_range = full_range - margin;

    printf("Device Function Started... \n");

    while (1)                       
    {
        Ultrasonic_Sensor(valid_range, &checktime);
    }
    
    return 0;  
    
}

void Ultrasonic_Sensor(float v_range, struct tm* comp_time){
    float temp = 0;
    int count = 0;
    bool t_mode = MODE;
    if(t_mode == 0){        //mode useual
         while (1){
            time_t sys_time = time(NULL);
            struct tm maintime = *localtime(&sys_time);

            temp = Get_Range();
            if(temp < v_range){
                Counter(&maintime, &comp_time, &count);
            }
            if(MODE != t_mode){
                printf("MODE CHANGE : STATISTICS -> SECURITY \n");
                break;
            }
        }
    }else if(t_mode == 1){  //mode security
        while (1)
        {
            time_t sys_time = time(NULL);
            struct tm maintime = *localtime(&sys_time);

            temp = Get_Range();
            if(temp < v_range){
                Alert(&maintime);
            }
            if(MODE != t_mode){
                printf("MODE CHANGE : SECURITY -> STATISTICS \n");
                break;
            }
        }
    }
    
}

float Get_Range(){
    float distance, start, stop;

    //
    digitalWrite(SONIC_Echo, LOW);
    digitalWrite(SONIC_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(SONIC_Trig, LOW);
    //

    //
    while(digitalRead(SONIC_Echo) == LOW) 
        start = micros();
    while(digitalRead(SONIC_Echo) == HIGH) 
        stop = micros();
    //

    distance = (stop - start) / 58.;
    delay(50);

    return distance;
}

//일정 시간이 경과하면 카운터 초기화,
//기본적으로 파일에 카운터 저장
void Counter(struct tm* m_time,struct tm* comp, int *m_cnt){
    FILE* temp;
    temp = fopen("record.txt", "atw"); //파일 없을경우 생성, 파일 존재할 경우 뒤에 내용 추가

    switch (TIME_MODE){
    case 0:     // per day
        if(comp->tm_mday != m_time->tm_mday){
            fprintf(temp, "%d/%d/%d -%d-\n",
                    1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,m_cnt);
            memcpy(&(*comp), &(*m_time), sizeof(struct tm));
            *m_cnt = 0;
        }
        *m_cnt++;
        break;
    case 1:     // per hour
        if(comp->tm_hour != m_time->tm_hour){
            fprintf(temp, "%d/%d/%d %d -%d-\n",
                    1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,comp->tm_hour,m_cnt);
            *m_cnt = 0; 
            memcpy(&(*comp), &(*m_time), sizeof(struct tm));
        }
        *m_cnt++;
        break;
    default:
        printf("invalid time setting \n");
        break;
    }

    fclose(temp);
}

void Alert_On(struct tm* m_time){
    FILE* temp;
    temp = fopen("alert_record.txt", "atw");
    fprintf(temp, "%d/%d/%d %d:%d:%d\n",
            1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_time->tm_hour,m_time->tm_min,m_time->tm_sec);
    fclose(temp);

    WARNING = TRUE;
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
}

/// @brief


void Alert_off(){
        //스피커 및 LED 끔
    if(WARNING == FALSE){
        digitalWrite(BUZZER, LOW);
        digitalWrite(LED, LOW);
    }
}

//여러 바이트의 데이터를 씀
void serialWriteBytes(const int fd, const char *s)
{
    write(fd, s, strlen(s));
}

unsigned char serialRead(const int fd)
{
    unsigned char x;
    if (read(fd, &x, 1) != 1) // read 함수를 통해 1바이트 읽어옴
        return -1;
    return x; //읽어온 데이터 반환
}

void serialWrite(const int fd, const unsigned char c)
{
    write(fd, &c, 1); // write 함수를 통해 1바이트 씀
}

void Bluetooth()
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
                    MODE = 0;
                    //초음파 센서 일반모드 함수 실행
                    break;
                case '1': //보안모드
                    MODE = 1;
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
                    WARNING = FALSE;
                    Alert_off();
                    break;
            }
        }
        delay(10);
    }    
}