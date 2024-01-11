#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "Conveyor.h"
#include "FallSensor.h"
#include "SlotMaster.h"
#include "JammerMotor.h"
#include "ADConversion.h"
#include "CardScanner.h"
#include "Countdown.h"

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();
    InitCardScanner();

    Wait(2000);        // 2秒待つ
    EnableCountdown(); // カウントダウンを有効にする
    EnableConveyor();
    while (1) // 無限ループ
    {
        if (UARTCheck() > 0) // UARTにデータがあるかチェック
        {
            unsigned char RXdata = UARTReceive(); // UARTからデータを受信
            if (ResultCheckCountdown(RXdata))     // カウントダウンの結果をチェック
            {
                DisableCountdown(); // カウントダウンを無効にする
            }
        }
        MakePlayingSound();
    }
}