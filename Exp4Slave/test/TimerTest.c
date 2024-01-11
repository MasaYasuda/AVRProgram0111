#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"

int main() // メイン関数
{
	OSCCAL = 0b01101111; // 内蔵クロックを校正
	DDRC = 0b00100000;	 // PORTC5を出力に設定
	InitTimer();		 // タイマーを初期化
	unsigned long timeSwitched = 0;
	while (1) // 無限ループ
	{
		if (GetMillis() - timeSwitched > 1000)
		{

			PORTC ^= 0b00100000; // PORTC5の状態を反転
			timeSwitched = GetMillis();
		}
	}
}
