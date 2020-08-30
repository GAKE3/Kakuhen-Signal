/*
 確変信号を擬似的に出す
 1st Build 20/08/26
 2nd Build 20/08/29 SW4を遅延タイマーの有効切り替えに変更
                    大当り中はスタート信号出力をしないようにした
*/

/*
    Kakuhen-Signal

    Copyright (c) 2020 GAKE3

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php
*/

/*
	I/O
	 2 I スタート信号入力
	 3 I 大当り信号入力
	 4 I DIPSW-1
	 5 I DIPSW-2
	 6 I DIPSW-3
	 7 I DIPSW-4
	11 O スタート信号出力
	12 O 大当り信号出力
	13 O 確変信号出力
	ProMicro使用時
	14 O スタート信号出力
	15 O 大当り信号出力
	16 O 確変信号出力
	※大当り信号出力中は確変信号も出力する

	DIPSW
	123
	000 大当り信号入力時のみ確変信号を出す
	100 スタート信号 1回
	010 スタート信号 2回
	110 スタート信号 3回
	001 スタート信号 4回
	101 スタート信号 6回
	011 スタート信号 8回
	111 常に確変信号を出す

	SW4 遅延タイマーの設定 ONで無効
*/


#define PRO_MICRO 1 // ProMicro使用時は1にする

#define useSerial 1 // シリアル通信を使う(デバッグ用)

byte asig, ksig; // 大当り・確変信号ポート

byte kkcount; // 確変回数カウント
byte ckcount; // 残回数カウント
bool sta; // スタート信号解除待ち
unsigned long tmr; // 遅延タイマー
#ifdef useSerial
	unsigned long tmrd; // デバッグ用タイマー
	bool aflg;
#endif

int tmrc = 2000; // 遅延タイマー既定値(msec)

void setup() {
	int base = 1;

#ifdef useSerial
	Serial.begin(9600);
#endif

	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	pinMode(4, INPUT_PULLUP);
	pinMode(5, INPUT_PULLUP);
	pinMode(6, INPUT_PULLUP);
	pinMode(7, INPUT_PULLUP);
#ifdef PRO_MICRO
	pinMode(14, OUTPUT);
	pinMode(15, OUTPUT);
	pinMode(16, OUTPUT);
	asig = 1;
	ksig = 2;
#else
	pinMode(11, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(13, OUTPUT);
	asig = 4;
	ksig = 5;
#endif

	// DIPSW読み取り
	for (int i = 0; i <= 2; i++) {
		if (!digitalRead(4 + i)) {
			kkcount = kkcount + base;
		}
		base = base * 2;
	}
	if (kkcount == 6) kkcount = 8;
	if (kkcount == 5) kkcount = 6;
	if (!digitalRead(7)) tmrc = 0;
#ifdef useSerial
#endif

}

void loop() {
	// 出力ON  PORTB &= ~_BV(3);
	// 出力OFF PORTB |= _BV(3); 

	if (!digitalRead(2) && digitalRead(3)) {
		PORTB &= ~_BV(3);
	} else {
		PORTB |= _BV(3);
		sta = false;
	}

	if (!digitalRead(3)) {
		PORTB &= ~_BV(asig);
		PORTB &= ~_BV(ksig);
		ckcount = kkcount;
#ifdef useSerial
		if (!aflg) {
			Serial.println("ATARI IN");
			aflg = true;
		}
#endif
	} else {
		PORTB |= _BV(asig);
		if (ckcount == 0) {
			if (tmr < millis()) PORTB |= _BV(ksig);
		} else {
			PORTB &= ~_BV(ksig);
		}
#ifdef useSerial
		aflg = false;
#endif
	}
	if (!digitalRead(2) && !sta) {
		sta = true;
		if (ckcount > 0 && tmr < millis()) {
#ifdef useSerial
			Serial.println("START CD");
#endif
			ckcount--;
		} else {
#ifdef useSerial
			Serial.println("START IN");
#endif
		}
		tmr = millis() + tmrc;
	}
	if (kkcount == 7) ckcount = kkcount;
#ifdef useSerial
	tmrd = millis();
	if((tmrd % 1000) == 0) {
		if (!digitalRead(7)) Serial.print("@");
		Serial.print("SW = ");
		Serial.print(kkcount);
		Serial.print(" CO = ");
		Serial.print(ckcount);
		Serial.print(" TM = ");
		Serial.println(tmr - tmrd);
	}
#endif
}
