#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "Conveyor.h"
#include "FallSensor.h"
#include "SlotMaster.h"
#include "JammerMotor.h"

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();

    Wait(5000); // 5秒待つ
    EnableJammerMotor(); // ジャマーモーターを有効にする
    while (1) // 無限ループ
    {
        if (CheckButtonJammerMotor()) // ジャマーモーターのボタンが押されたかチェック
            DisableJammerMotor(); // ジャマーモーターを無効にする
        ChangePhaseJammerMotor(); // ジャマーモーターのフェーズを変更する
    }
}