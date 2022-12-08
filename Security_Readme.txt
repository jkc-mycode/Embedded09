필요 센서
ultrasonic
buzzer
bluetooth
+ timer  (시스템 함수 time.h 로 대체)

Security.c (main) 에서 처리해야 하는 task
    ultrasonicc(HC-SR04) using gpio 17, 18 (PWM)
    buzzer(KY-012) using GPIO 23 (Basic gpio(HIGH,LOW))

UserControll.c (bluetooth) 에서 처리해야 하는 task
    bluetooth(HC-06) using GPIO 14, 15 (UART)

프로세스 간 통신 지원을 바탕으로 작성
불가시 직접 파일에 접근하는 방법으로 우회.

1, 최대 거리 측정
        
            
2, 최대 거리 - margin = 측정 유효 범위

(반복)
    (MODE 0)
        3, 측정 유효 범위 > 측정값 = 물체 인식
        4, 물체 인식 후, 다시 측정 유효 범위로 복귀시 counter 함수 호출
        5, counter함수 갱신시 파일에 기록 
            -> (timer.h활용해서 시간대별 인파 기록)
                -> (counter 값을 시간대별 파일로 출력)
    (MODE 1)
        3, 측정 유효 범위 > 측정값 = 물체 인식
        4, 물체 인식시 buzzer 작동((waring) on) 
            gpio 23(buzzer) 에 HIGH 신호 전송
            시간이 기록된 별개의 Security 기록 파일 작성
            BreadBoard 의 최측면 한 라인에 gpio 선을 연결시켜 HIGH입력시 LED도 같이 점등되게끔 구현
        5, UserControl에서 bool값 변화 (bool WARING FALSE) 수신시 
            bool WARING FALSE로 변경, gpio 23 LOW신호 전송
            (UserControl에서 직접 bool값 변경시 함수 내에서 bool값 변경을 확인하는 if 구문 작성 필요)

    (interupt = bluetooth)(X) -> 
        (user 에서 main으로 보내는 pipe나 signal로 구현)
            (UserControl.c ->Security.c)
            case 0
                - MODE변경 (bool True -> FALSE)
            case 1
                - MODE변경 (bool FALSE ->True)
            case 2
                - 외부에서 파일 데이터에 접근할 예정이니 파일 갱신 정지(lock ?)
                - Usercontrol.c 에서 파일 전송 완료 신호 수신시 (unlock)
            case 3
                - 파일 삭제, count 초기화
        
            (Security.c -> UserControl.c)
            - 

-----------------------------------------------------------
UserControl.c (bluetooth)
    프로그램의 각 task 처리에 대해 
    RX TX 능동적 변경 설계 필요

    0. 사용 전 기본 wiringPI 셋팅

    (반복)
        1, 사용자 입력 대기

        2.1.1, 사용자가 값 입력 (user(phone) -> UserControl.c (HC-06))
        (DEMO)  0 - mode 0 로 변경
        char    1 - mode 1 로 변경
                2 - 기록 파일 요청
                3 - 기록 파일 초기화
        2.1.2, 입력 값 Security.c로 전송 (UserControl.c -> Security.c)

        2.2.1, 외부 메시지 수신( UserControl.c (HC-06) -> user(phone))
        (DEMO)  case 0 (파일 요청 처리) 
                    Security.c가 저장한 텍스트 파일에 접근, 파일읗 읽어 UART 통신으로 전송
                    파일 전송 완료시 Security.c 에 완료 신호 전송
                case 1 (경고 메시지 처리)
                    Security.c에서 UserControl.c로 보낸 WARING문구(문자열 배열 char buf[255]) 수신시 
                    UART통신으로 해당 문구 전송
                        (예) 
                        ---------------WARING---------------
                        "22,12,10 12:42:11 MOVE DETECTED"
                        ------------------------------------
                    
                    (user(phone) -> HC-06 으로 아무 입력이나 입력받을 시)
                    Security.c으로 경보 해제 시그널 (bool WARING FALSE) 전송 
                        -> (아니면 Security.c의 bool값에 직접 접근할 수 있게 설계 (추후))
                    

