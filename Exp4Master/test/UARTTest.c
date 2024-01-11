#define F_CPU 1000000L  // CPUのクロック周波数を設定
#include <avr/io.h>     // AVRの入出力を行うためのヘッダファイルをインクルード
#include <util/delay.h> // ディレイ関数を使用するためのヘッダファイルをインクルード
#include "UART.h"       // UART通信を行うためのヘッダファイルをインクルード

int main() // メイン関数
{
    unsigned char goalstate = 0; // 目標状態を初期化
    OSCCAL = 0b01101111;         // クロックを校正
    DDRB = 0b00001000;           // DDRBの3ビット目を出力に設定
    UARTSetup(9600);             // 9600bpsで通信を開始
    while (1)                    // 無限ループ
    {
        if (UARTCheck > 0) // 受信データがある場合
        {
            unsigned char val = UARTReceive; // 受信データを読み込む
            PORTB = val << 3;                // 受信データを3ビット左シフトしてPORTBに出力
        }
        goalstate = 1 - goalstate; // 目標状態を反転
        UARTTransmit(goalstate);   // 目標状態を送信
        _delay_ms(800);            // 800ms待機
    }
}
