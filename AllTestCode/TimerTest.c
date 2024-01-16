// TimerTest.c
#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"

int main()
{
	DDRC = 0b00000001; // PC0を出力に設定
	InitTimer();	   // タイマーを初期化
	while (1)
	{
		PORTC ^= 0b00000001; // PC0の状態を反転
		Wait(1000);
	}
}
