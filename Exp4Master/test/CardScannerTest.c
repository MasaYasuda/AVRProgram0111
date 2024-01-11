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

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();
    InitCardScanner();

    Wait(2000);          // 2秒待つ
    EnableCardScanner(); // カードスキャナーを有効にする
    while (1)            // 無限ループ
    {
        if (CheckCardScanner())   // カードスキャナーのボタンが押されたかチェック
            DisableCardScanner(); // カードスキャナーを無効にする
        ChangePhaseCardScanner(); // カードスキャナーのフェーズを変更する
    }
}