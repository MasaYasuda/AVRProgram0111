#define F_CPU 1000000L
#include "Timer.h"
#include "Speaker.h"
#include "Conveyor.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitSpeaker();
    InitConveyor();
    EnableConveyor();
    // TmpDecelerateConveyor();              // コンベアを一時減速
    unsigned long tmpTime = GetMillis();  // 現在のミリ秒を取得し格納
    while (GetMillis() - tmpTime < 15000) // 15秒間のループ
    {
        MakePlayingSound();
        if (CheckButtonConveyor())            // ボタンが押されているか確認
            OutputConveyor(dutyRateConveyer); // デューティ比に応じてコンベアを動かす
        else
            OutputConveyor(0);      // ボタンが押されていなければコンベアを停止
        ChangePhaseTmpDecelerate(); // 一時減速のフェーズを変更
    }
    DisableConveyor(); // コンベアを無効にする
    while (1)
    {
        MakePlayingSound();
        OutputConveyor(0); // コンベアを停止状態にする
    }
}