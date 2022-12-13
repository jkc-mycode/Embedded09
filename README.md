# Embedded09

### 아이디어 소개
- 물체의 이동을 감지하는 시스템
- 별도의 장소에 부착하여 통행량을 측정하기 위해서 사용
- 블루투스로 연결된 핸드폰에서 보안모드와 통계모드를 조절할 수 있음
- 보안모드에서는 이동 감지 시 경고음과 LED가 동작하고 스마트폰으로 경고 알람이 전송
- 통계모드에서는 이동 감지 시 프로그램 상에서 count를 파일에 기록
- 이동을 감지할 때마다 측정하는 것이 아니라 일정 시간동안의 통행량이 파일에 기록됨
- 스마트폰에서 파일에 저장된 기록을 불러서 확인 가능
- 보안모드에 이동이 감지된 후 스마트폰에서 경고음과 LED를 해제 가능

### 사용된 센서 및 장치
- 초음파 센서(HC-SR04)
- 블루투스 모듈(HC-06)
- 액티브 스피커
- LED

### 전체 시스템 구조
![image](https://user-images.githubusercontent.com/90839233/206896733-0d123e5a-1c0c-40eb-ad86-51fce4951a3b.png)

### 주요 개발 내용
- 초음파 센서(HC-SR04), 블루투스(HC-06) 및 BUZZER(액티브 스피커), LED / Security.c

  1. 초기 거리 측정
  2. 측정 기본값 = 초기 거리값 - margin(센서 오차범위)

(통계모드일 때)

  1. 현재 측정값이 측정 기본값보다 작으면 물체 인식 (ex. 측정 기본값이 99cm이고 현재 거리 측정값이 30cm일 때 물체인식)
  2. 물체 인식 후 현재 측정값이 측정 기본값과 동일해지면 counter 함수 호출
  3. counter 함수 갱신 시 파일에 기록

(보안모드일 때)

  1. 측정 기본값 > 현재 측정값 = 물체 인식
  2. 물체 인식시 LED, BUZZER 작동(bool WARNING = TRUE)
  3. 블루투스로 알람 해제를 입력하면 bool WARNING값이 FALSE가 되고 LED, BUZZER 작동 종료

(Bluetooth 함수)

1. 사용자 입력 대기
2. 사용자가 값 입력 (휴대폰의 Serial Bluetooth Terminal앱으로 0~5 입력 -> Bluetooth모듈로 수신)
3. 블루투스로 받은 사용자의 입력에 따른 함수 실행
- 0 : 보안모드로 변경
- 1 : 통계모드로 변경
- 2 : 기록 파일 요청
- 3 : 기록 파일 초기화
- 4 : 초기 설정하기
- 5 : 경보 끄기


### 제한조건 구현 내용
1. 제한조건 구현 내용 (멀티프로세스/쓰레드, IPC/뮤텍스)

### 사용 설명서
1. 장치를 설치 후 블루투스로 스마트폰과 연동시켜준다.
2. 스마트폰에서 '초기 설정' 이라는 메뉴를 눌러 측정할 거리를 계산한다.
3. 기본적으로 통계모드가 실행된다.
4. 보안모드로 변경을 원하면 스마트폰에서 '보안' 메뉴를 선택한다.
5. 통행량의 기록을 확인하고 싶다면 스마트폰에서 '기록 확인' 메뉴를 선택한다.
6. 기록된 통행량을 삭제하고자 한다면 스마트폰에서 '기록 삭제' 메뉴를 선택한다.
7. 보안모드에서 경보 발생 시 울리는 경보음과 LED를 끄고 싶다면 '경보 해제' 메뉴를 선택한다.

### 문제 및 해결방안

### 한계점
1. 사람 여럿이 딱 붙어 지나가면 한 명으로 인식하게 된다.
2. 초음파 센서 범위 아래나 위로 지나가면 인식할 수 없다. 

### 시연 영상

### 참고자료






