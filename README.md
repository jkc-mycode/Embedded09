# Embedded09

팀원 : 20140818 유환준, 20160042 고원동, 20201414 김정찬

## 아이디어 소개
- 물체의 이동을 감지하는 시스템
- 별도의 장소에 부착하여 통행량을 측정하기 위해서 사용
- 블루투스로 연결된 핸드폰에서 보안모드와 통계모드를 조절할 수 있음
- 보안모드에서는 이동 감지 시 경고음과 LED가 동작하고 스마트폰으로 경고 알람이 전송
- 통계모드에서는 이동 감지 시 프로그램 상에서 count를 파일에 기록
- 이동을 감지할 때마다 측정하는 것이 아니라 일정 시간동안의 통행량이 파일에 기록됨
- 스마트폰에서 파일에 저장된 기록을 불러서 확인 가능
- 보안모드에 이동이 감지된 후 스마트폰에서 경고음과 LED를 해제 가능

-----------------------------------

## 사용된 센서 및 장치
- 초음파 센서(HC-SR04)
- 블루투스 모듈(HC-06)
- 액티브 스피커
- LED
- Serial Bluetooth Terminal(휴대폰 앱)

-----------------------------------

## 전체 시스템 구조
![image](https://user-images.githubusercontent.com/90839233/206896733-0d123e5a-1c0c-40eb-ad86-51fce4951a3b.png)
![Embedded_Fors ps](https://user-images.githubusercontent.com/66898122/208358969-8e17a24b-eeb3-4a9d-95c1-115e78276a86.png)

-----------------------------------

## 주요 개발 내용 (자세한 내용은 Security.c 코드 참고)
초음파 센서(HC-SR04), 블루투스(HC-06) 및 BUZZER(액티브 스피커), LED / Security.c

### Set_Range 함수 ###

1. main 함수에서 처음 실행되어 기준이 되는 거리 값을 반환
2. 측정 기본값 = 초기 거리값 - margin(10cm)
3. 스마트폰에서 초기설정 버튼을 누르면 Bluetooth 함수에서 한번 더 실행


### Get_Range 함수 ###

1. Ultrasonic_Sensor 함수 안에서 반복문으로 반복해서 사용
2. 실시간으로 초음파 센서를 사용해서 거리 값을 반환


### Ultrasonic_Sensor 함수 ###

1. main 함수에서 pthread_create 함수로 쓰레드로 생성되어서 실행
2. MODE 변수로 통계모드일 때, 보안모드일 때를 구분해서 동작
3. 블루투스 모듈에 값이 들어오면 pthread_cond_signal 함수로 Bluetooth 함수 깨워서 모드변수 변경
4. Set_Range 함수와 Get_Range 함수에서 반환된 값을 비교
5. Get_Range 함수의 반환값이 더 작으면 범위에 들어왔다고 인식하고 Counter 함수 실행


### Bluetooth 함수 ###

1. main 함수에서 pthread_create 함수로 쓰레드로 생성되어서 실행
2. 코드 시작시 일단 pthread_cond_wait 함수로 인해 대기상태로 들어감
3. 블루투스 모듈과 연결된 기기에서 값이 주어지면 그때 깨어나서 실행
4. 연결된 기기에서 받아온 값에 따라서 동작 

```
    0 : 통계모드로 변경 => MODE 변수 0으로 변경
    1 : 보안모드로 변경 => MODE 변수 1로 변경
    2 : 초기 설정하기 => Set_Range 함수 실행
    3 : 기록 파일 요청 => 파일 열어서 읽어서 출력
    4 : 기록 파일 초기화 => 파일 내용 초기화
    5 : 경보 해제 => Alert_Off 함수 실행
    6 : 일간 측정으로 변경 => TIME_MODE 변수 0으로 변경
    7 : 시간 측정으로 변경 => TIME_MODE 변수 1로 변경
    8 : 분간 측정으로 변경 => TIME_MODE 변수 2로 변경
    9 : 종료
```
![KakaoTalk_20221219_140918593](https://user-images.githubusercontent.com/90839233/208352292-40694ef4-357e-47a3-9931-257a5865a961.jpg)



### Counter 함수 ###

1. 기준이되는 시간과 계속 변하는 시간 구조체 변수를 매개변수로 가져옴
2. record.txt 파일을 열어서 기록할 준비
3. TIME_MODE에 따라서 일간, 시간, 분간으로 구분해서 실행
4. 기준 시간과 계속변하는 시간이 서로 다르면 파일에 count 변수로 시간을 기록 (시간이 변했다는 것을 의미)


### Alert_On 함수 ###

1. WARNING 변수 TRUE로 변경
2. 호출 시 alert_record.txt 파일에 알람 기록 
3. 기록한 내용 바로 블루투스로 연결된 기기에 출력
4. LED는 점멸되도록 실행 (LOW <-> HIGH)
5. BUZZER는 HIGH로 실행


### Alert_Off 함수 ###

1. WARNING 변수 FALSE로 변경
2. 블루투스로 연결된 기기에서 값이 들어오면 Bluetooth 함수를 깨움
3. 호출 시 LED와 BUZZER를 LOW로 변경

    
----------------------------

## 제한조건 구현 내용 (멀티프로세스/쓰레드, IPC/뮤텍스)
1. 쓰레드를 통해서 블루투스 모듈과 초음파 센서를 동시에 실행시킨다.
2. 블루투스와 초음파센서 사이에 뮤텍스가 반드시 필요한 곳이 없었다.
3. 대신에 Pthread의 조건 변수를 사용하여 블루투스와 초음파 센서의 동기화에 사용했다. 
4. 블루투스 모듈에서 pthread_cond_wait() 를 통해서 실행 시 일단 대기 상대로 만든다.
5. 초음파 센서에서 MODE나 WARNING의 변경을 확인하고 가져올 때만 pthread_cond_signal()로 블루투스를 깨운다.
6. pthread_cond_signal()로 깨어났을 때만 값이 적용된다.

```c
    void *Ultrasonic_Sensor(void *t){
      ....
      pthread_cond_signal(&cond); //블루투스 쓰레드를 대기상태에서 깨우는 함수
      ....
    }

    void *Bluetooth(){
      ....
      pthread_cond_wait(&cond, &mutex); //블루투스를 대기상태로 바꿈
      ....
    }
```

----------------------------

## 사용 설명서
1. 장치를 설치 후 블루투스로 스마트폰과 연동시켜준다.
2. 스마트폰에서 '초기 설정' 메뉴를 눌러 측정할 거리를 계산한다.
3. 기본적으로 통계모드가 실행된다.
4. 보안모드로 변경을 원하면 스마트폰에서 '보안' 메뉴를 선택한다.
5. 통행 기록을 확인하고 싶다면 스마트폰에서 '기록 확인' 메뉴를 선택한다.
6. 통행 기록을 삭제하고 싶다면 스마트폰에서 '기록 삭제' 메뉴를 선택한다.
7. 보안모드에서 경보 발생 시 울리는 경보음과 LED를 끄고 싶다면 '경보 해제' 메뉴를 선택한다.

----------------------------

## 문제 및 해결방안
1. 초음파 센서의 측정이 유지되면서 블루투스 입력에 대한 처리를 즉각적으로 보장해주어야 한다.
 
    &rarr; Pthread 조건변수 pthread_cond 를 사용하여 두 쓰레드의 활성 상태를 제어할 수 있다.


2. 단위 측정 시간과 현재 시간의 차이점 계산이 필요하다.

    &rarr; 시간 구조체 변수를 2개 선언하고 하나는 현재 시간을 계속 갱신한다.

    &rarr; 다른 하나는 비동기로 유지하면서 두 구조체의 값이 다를 때 memcpy()를 통해 이전 시간을 복사한다.

    &rarr; 그렇게 시간이 달라지면 파일에 기록한다.

----------------------------

## 한계점
1. 사람 여럿이 딱 붙어 지나가면 한 명으로 인식하게 된다.
2. 초음파 센서 범위 아래나 위로 지나가면 인식할 수 없다. 
3. 초음파 센서 값이 튀어 물체가 인식되었다고 처리될 수도 있다.
4. 사람이 아닌 아주 작은 물체에도 count가 기록이 된다.

----------------------------

## 시연 영상
[![Video Label](https://img.youtube.com/vi/Ix-1Xho6wZE/0.jpg)](https://youtu.be/Ix-1Xho6wZE)

----------------------------

## 참고자료
1. 강의자료
2. Pthread_cond documentation (https://www.joinc.co.kr/w/Site/Thread/Beginning/PthreadApiReference#AEN144)
3. 조건변수를 통한 스레드 동기화 설명, 예제 (https://reakwon.tistory.com/99)
4. 라즈베리파이에 GPIO 파이썬을 이용해 초음파 센서 (https://playneko.github.io/2020/06/19/rasberry-pi/rasberry-pi-025/)





