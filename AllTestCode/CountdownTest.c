// CountdownTest.c
#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "Countdown.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitUART(9600);
    InitSpeaker();

    Wait(2000);        // 2秒待つ
    EnableCountdown(); // カウントダウンを有効にする(命令をSlaveに送信)
    while (1)
    {
        MakePlayingSound();
        if (UARTCheck() > 0) // 受信バッファにデータがあるかチェック
        {
            unsigned char RXdata = UARTReceive(); // 受信バッファからデータを受信
            if (ResultCheckCountdown(RXdata))     // カウントダウンの結果をチェック
                DisableCountdown();               // カウントダウンを無効にする
        }
    }
}