#define F_CPU 16000000L
#include <avr/io.h>
#include "Timer.h"

#include "UARTEasy.h"

int main()
{
	OSCCAL = 0b01101111; // 内蔵クロックを校正
    InitTimer();                         // タイマーの初期化
    InitUART(9600);                      // UARTを9600bpsで初期化
    DDRB = 0xFF;                         // ポートBを出力に設定
    unsigned long tmpTime = GetMillis(); // 現在のミリ秒を取得
    while (1)
    {

        if (GetMillis() - tmpTime > 2000) // 1秒以上経過したかチェック
        {
            PORTB ^= 0xFF;
            tmpTime = GetMillis(); // 現在のミリ秒を更新
        }
    }
}