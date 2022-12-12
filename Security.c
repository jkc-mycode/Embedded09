#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

#define SONIC_Trig  18
#define SONIC_Echo  4
#define BUZZER 23
#define LED 24
#define BT_RXD 14
#define BT_TXD 15

bool MODE = 0;      //모드 전역변수, bluetooth에서 직접 접근
bool WARNING = 0;   //보안 모드에서 현재 경보가 켜져있는지 아닌지
int TIME_MODE = 0;  // 0 = day, 1 = hour
time_t sys_time = time(NULL);

//ultra sonic sensor
float Get_Range();
void Ultrasonic_Sensor(float v_range, struct tm* m_time, struct tm* comp_time);

//
void Alert(struct tm* m_time);
void Counter(struct tm* m_time,struct tm* comp, int *m_cnt);

//외부 입력 처리


int main(){
    float full_range = 0;
    float valid_range = 0;
    float margin = 15.0f;

    //시간 기록을 위한 변수
    struct tm *maintime;
    maintime = localtime(&sys_time);
    struct tm checktime;
    checktime = maintime;
    
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
        Ultrasonic_Sensor(valid_range, &maintime, &checktime);
    }
    
        
    
}

void Ultrasonic_Sensor(float v_range, struct tm* m_time, struct tm* comp_time){
    float temp = 0;
    int count = 0;
    bool t_mode = MODE;
    if(t_mode == 0){        //mode useual
         while (1){
            m_time = localtime(&sys_time);
            temp = Get_Range();
            if(temp < v_range){
                Counter(&m_time, &comp_time, &count);
            }
            if(MODE != t_mode){
                printf("MODE CHANGE : STATISTICS -> SECURITY \n");
                break;
            }
        }
    }else if(t_mode == 1){  //mode security
        while (1)
        {
            m_time = localtime(&sys_time);
            temp = Get_Range();
            if(temp < v_range){
                Alert(&m_time);
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
            *m_cnt++;
            fprintf(temp, "%d/%d/%d -%d-\n",
                    1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_cnt);
            *comp = &m_time;
            *m_cnt = 0;
        }
        *m_cnt++;
        break;
    case 1:     // per hour
        if(comp->tm_hour != m_time->tm_hour){
            fprintf(temp, "%d/%d/%d %d -%d-\n",
                    1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_time->tm_hour,m_cnt);
            *m_cnt = 0; 
            *comp = &m_time;
        }
        *m_cnt++;
        break;
    default:
        printf("invalid time setting \n");
        break;
    }

    fclose(temp);
}

void Alert(struct tm* m_time){
    FILE * temp;
    temp = fopen("alert_record.txt", "atw");
    fprintf(temp, "%d/%d/%d %d:%d:%d\n",
            1900+m_time->tm_year,m_time->tm_mon+1,m_time->tm_mday,m_time->tm_hour,m_time->tm_min,m_time->tm_sec);
    fclose(temp);

    WARNING = TRUE;
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
}
