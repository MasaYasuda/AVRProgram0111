#define F_CPU 1000000L
#include "ADConversion.h"
#include "LightVariesLED.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitSpeaker();
    InitLightVariesLED();
    Wait(2000); // 初期状態確認のため少し待機

    EnableLightVariesLED(); // LEDの点灯を開始する
    while (1)
    {
        MakePlayingSound();
        if (CheckVolumeLED())        // ボリュームの状態を確認する
            DisableLightVariesLED(); // LEDの点灯を停止する
        ChangePhaseLightVariesLED(); // LEDの点灯状態を更新する
    }
}