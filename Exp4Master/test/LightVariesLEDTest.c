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
#include "LightVariesLED.h"

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();
    InitCardScanner();
    InitLightVariesLED();

    Wait(2000);
    EnableLightVariesLED(); // LEDの点灯を開始する
    while (1)
    {
        if (CheckVolumeLED()) // ボリュームの状態を確認する
        {
            SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定する
            DisableLightVariesLED(); // LEDの点灯を停止する
        }
        ChangePhaseLightVariesLED(); // LEDの点灯状態を更新する
        MakePlayingSound();
    }
}