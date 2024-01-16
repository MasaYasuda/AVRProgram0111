#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"

int main() // メイン関数
{
    InitTimer();
    InitUART(9600);    // ボーレート9600で開始
    DDRC = 0b00000001; // PC0を出力に設定
    unsigned long timeSwitched = GetMillis();
    while (1)
    {
        if (GetMillis() - timeSwitched > 1000) // 1秒ごとに
        {
            UARTTransmit(0b00001111);   // データを送信する
            timeSwitched = GetMillis(); // 切り替え時刻を更新
        }
        if (UARTCheck() > 0) // 受信していたら
        {
            if (UARTReceive() == 0b00001111) // データが問題なければ
                PORTC ^= 0b00000001;         // PC0の状態を反転
        }
    }
}
