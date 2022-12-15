#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
//#include <math.h>
#include <fcntl.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <pthread.h>
//#include <ctype.h>

#define BAUD_RATE 115200 
#define SONIC_Trig  18
#define SONIC_Echo  4
#define BUZZER 23
#define LED 24
#define BT_RXD 1
#define BT_TXD 0

bool MODE = 0;      //모드 전역변수, bluetooth에서 직접 접근
bool WARNING = 0;   //보안 모드에서 현재 경보가 켜져있는지 아닌지
int TIME_MODE = 2;  // 0 = day, 1 = hour, 2 = min
float full_range = 0;
float v_range = 0;
int count = 0;
int fd_serial;     // UART2 파일 서술자
int INDEX = 0;


//ultra sonic sensor
float Get_Range();
void Set_Range();
void *Ultrasonic_Sensor(void* t);

//
void Alert_On(struct tm* m_time, struct tm* comp);
void Alert_Off();
void Counter(struct tm* m_time,struct tm* comp);
void File_Updater(struct tm* m_time,struct tm* comp);

static const char *UART2_DEV = "/dev/ttyAMA1"; // UART2 연결을 위한 장치 파일

void *Bluetooth();
unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);
void serialWriteBytes(const int fd, const char *s);

pthread_cond_t cond;

int main(){
    //시간 기록을 위한 변수 (비갱신)
    time_t sys_time = time(NULL);
    struct tm checktime = *localtime(&sys_time);
    pthread_t tid1, tid2;
    
    if(wiringPiSetupGpio() < 0){
        printf("wiringPiSetupGpio failed \n");
        return -1;
    }

    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0)
    { // UART2 포트 오픈
        printf("Unable to open serial device.\n");
    }
    
    printf("wiringPi Setup Complete \n");

    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(SONIC_Trig, OUTPUT);
    pinMode(SONIC_Echo, INPUT);

    //거리 측정 (초기 셋팅)
    Set_Range();

    printf("\nDevice Function Started... \n");

    pthread_cond_init(&cond, NULL);
    pthread_create(&tid1, NULL, Bluetooth, NULL);
    delay(100);
    pthread_create(&tid2, NULL, Ultrasonic_Sensor, &checktime);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_cond_destroy(&cond);
    return 0;  
}

void *Ultrasonic_Sensor(void* t){
    struct tm* comp_time = (struct tm*) t;
    float temp = 0;
    bool is_in = false;
    while(1){
        bool t_mode = MODE;

        if(t_mode == 0){        //mode useual
            if (serialDataAvail(fd_serial)){
                pthread_cond_signal(&cond);
            }
            
            time_t sys_time = time(NULL);
            struct tm maintime = *localtime(&sys_time);
            printf("v_range : %f\n", v_range);
            temp = Get_Range();

            //File_Updater(&maintime, comp_time);
            if(temp < v_range){
                is_in = true;
                //Counter(&maintime, comp_time, &count);
            }else if((temp >= v_range) && (is_in == true)){
                printf("====================\n");
                Counter(&maintime, comp_time);
                is_in = false;
            }
            if(MODE != t_mode){
                printf("MODE CHANGE : STATISTICS -> SECURITY \n");
                //break;
            }
            fflush(stdout);
            delay(100);
            
        }else if(t_mode == 1){  //mode security
            if (serialDataAvail(fd_serial)){
                pthread_cond_signal(&cond);
            }
            time_t sys_time = time(NULL);
            struct tm maintime = *localtime(&sys_time);

            temp = Get_Range();
            if(temp < v_range){
                Alert_On(&maintime, comp_time);
            }
            if(MODE != t_mode){
                printf("MODE CHANGE : SECURITY -> STATISTICS \n");
                //break;
            }
            fflush(stdout);
            delay(10);
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
    printf("temp : %f\n", distance);
    return distance;
}

void Set_Range(){
    float temp = 0.0f;
    float margin = 10.0f;

    temp = Get_Range();
    temp = temp - margin;
    v_range = temp;
    printf("%f", temp);
}

//일정 시간이 경과하면 카운터 초기화,
//기본적으로 파일에 카운터 저장
void Counter(struct tm* m_time,struct tm* comp){
    FILE* fp;
    char buf[1024];
    printf("================%d  %d================\n", comp->tm_min, m_time->tm_min);

    switch (TIME_MODE){
    case 0:     // per day
        if(comp->tm_mday != m_time->tm_mday){
            memcpy(&(*comp), &(*m_time), sizeof(struct tm));
            count = 0;
            INDEX++;
        }
        count++;
        break;
    case 1:     // per hour
        if(comp->tm_hour != m_time->tm_hour){
            count = 0; 
            memcpy(&(*comp), &(*m_time), sizeof(struct tm));
            INDEX++;
        }
        count++;
        break;
    case 2:     // per min
        if(comp->tm_min != m_time->tm_min){
            fp = fopen("./record.txt", "atw");
            sprintf(buf, "%2d/%2d/%2d %2d:%2d (%2d명)\n", 1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_time->tm_hour,m_time->tm_min,count);
            fprintf(fp, buf);
            count = 0; 
            fclose(fp);
            count = 0; 
            memcpy(&(*comp), &(*m_time), sizeof(struct tm));
            //INDEX++;
        }
        count++;
        break;
    default:
        printf("invalid time setting \n");
        break;
    }

}

void File_Updater(struct tm* m_time,struct tm* comp){
    FILE* tempw;
    FILE* tempr;
    char buffer[255];
    tempw = fopen("./record.txt", "atw"); //파일 없을경우 생성, 파일 존재할 경우 뒤에 내용 추가
    tempr = fopen("./record.txt", "w");
    int tempidx = 0;

    if(INDEX > 0){
        while(fgets(buffer, 255, tempr) != NULL){
            if(tempidx == INDEX-1) break;
            tempidx++;
        }
    }

    switch (TIME_MODE){
    case 0:     // per day
        if((comp->tm_mday != m_time->tm_mday)){
            fprintf(tempw, "%d %d/%d/%d -%d-\n",
                    INDEX,1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,count);
        }else{
            fprintf(tempr, "%d %d/%d/%d -%d-\n",
                    INDEX,1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,count);
        }
        break;
    case 1:     // per hour
        if(comp->tm_hour != m_time->tm_hour){
            fprintf(tempw, "%d %d/%d/%d %d -%d-\n",
                    INDEX, 1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,comp->tm_hour,count);
        }else{
            fprintf(tempr, "%d %d/%d/%d %d -%d-\n",
                    INDEX, 1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,comp->tm_hour,count);
        }
        break;
    case 2:     // per min
        if(comp->tm_min != m_time->tm_min){
            fprintf(tempw, "%d %d/%d/%d %d:%d -%d-\n",
                    INDEX, 1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,comp->tm_hour,comp->tm_min,count);
        }else{
            fprintf(tempr, "%d %d/%d/%d %d:%d -%d-\n",
                    INDEX, 1900+comp->tm_year,comp->tm_mon+1,comp->tm_mday,comp->tm_hour,comp->tm_min,count);
        }
        break;
    default:
        printf("invalid time setting \n");
        break;
    }
    fclose(tempw);
    fclose(tempr);
}

void Alert_On(struct tm* m_time, struct tm* comp){
    FILE* temp;
    time_t sys_time = time(NULL);
    struct tm* checktime = localtime(&sys_time);
    char buf[1024];

    temp = fopen("./alert_record.txt", "atw");
    if(checktime->tm_sec == m_time->tm_sec){
        sprintf(buf, "******WARNING******\n%d/%d/%d %d:%d:%d\n***********************\n",
            1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_time->tm_hour,m_time->tm_min,m_time->tm_sec);
        fprintf(temp, buf);
        serialWriteBytes(fd_serial, buf);
    }
    fclose(temp);

    WARNING = TRUE;
    
    digitalWrite(BUZZER, HIGH);
    while(WARNING){
        if (serialDataAvail(fd_serial)){
            pthread_cond_signal(&cond);
        }
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
    delay(200);
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
}

void Alert_off(){
    //스피커 및 LED 끔
    if (serialDataAvail(fd_serial)){
        pthread_cond_signal(&cond);
    }
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

void *Bluetooth()
{
    unsigned char dat; //데이터 임시 저장 변수
    FILE* fp; //파일 입출력 변수
    char temp; //기록 내용 담을 임시변수
    char file_buffer[100];

    while (1)
    {
        pthread_cond_wait(&cond, &mutex);
        if (serialDataAvail(fd_serial))
        {                                //읽을 데이터가 존재한다면,
            dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
            
            if(dat == '0'){ //통계모드 변경
                MODE = 0;
                serialWriteBytes(fd_serial, "통계모드 실행\n");
            }
            else if(dat == '1'){ //보안모드 변경
                MODE = 1;
                serialWriteBytes(fd_serial, "보안모드 실행\n");
            }
            else if(dat == '2'){ //초기설정 모드
                Set_Range();
                serialWriteBytes(fd_serial, "측정 범위 변경\n");
            }
            else if(dat == '3'){ //파일 열리는지 확인
                if((fp = fopen("./record.txt", "r")) == NULL){ //파일 열리는지 확인
                    break;
                }
                while((fgets(file_buffer, sizeof(file_buffer), fp)) != NULL){ //파일이 끝날때 까지 반복
                    serialWriteBytes(fd_serial, file_buffer); //휴대폰으로 파일의 할줄씩 출력
                    printf("%s", file_buffer);
                    memset(file_buffer, 0, sizeof(file_buffer)); //메모리 초기화
                }
                fclose(fp);
            }
            else if(dat == '4'){ //기록삭제
                if((fp = fopen("./record.txt", "w")) == NULL){ //파일 열리는지 확인
                    break;
                }
                fputs(" ", fp);
                count = 0;
                fclose(fp);
                serialWriteBytes(fd_serial, "측정 기록 삭제\n");
            }
            else if(dat == '5'){ //경보 해제
                WARNING = FALSE;
                Alert_off();
            }
            else if(dat == '6'){ //일간 측정으로 변경
                TIME_MODE = 0;
                serialWriteBytes(fd_serial, "기록 간격 : 하루\n");
            }
            else if(dat == '7'){ //시간 측정으로 변경
                TIME_MODE = 1;
                serialWriteBytes(fd_serial, "기록 간격 : 1시간\n");
            }
            else if(dat == '8'){ //분간 측정으로 변경
                TIME_MODE = 2;
                serialWriteBytes(fd_serial, "기록 간격 : 1분\n");
            }
            else if(dat == '9'){ //종료
                serialWriteBytes(fd_serial, "프로그램이 종료됩니다.\n");
                exit(0);
            }
        }
        delay(10);
    } 
}
