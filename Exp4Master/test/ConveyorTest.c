#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "Conveyor.h"

int main()
{
    InitTimer();
    InitSpeaker();
    InitConveyor();
    EnableConveyor();
    unsigned long tmpTime = GetMillis(); // 現在のミリ秒を取得してtmpTimeに格納
    while (GetMillis() - tmpTime < 5000) // 5秒間のループ
    {
        if (CheckButtonConveyor())            // ボタンが押されているか確認
            OutputConveyor(dutyRateConveyer); // デューティ比に応じてコンベアを動かす
        else
            OutputConveyor(0);      // ボタンが押されていなければコンベアを停止
        ChangePhaseTmpDecelerate(); // 一時減速のフェーズを変更
        MakePlayingSound();         // 再生中の音を作成
    }
    TmpDecelerateConveyor();              // コンベアを一時減速
    tmpTime = GetMillis();                // 現在のミリ秒を再取得してtmpTimeに格納
    while (GetMillis() - tmpTime < 10000) // 10秒間のループ
    {
        if (CheckButtonConveyor())            // ボタンが押されているか確認
            OutputConveyor(dutyRateConveyer); // デューティ比に応じてコンベアを動かす
        else
            OutputConveyor(0);      // ボタンが押されていなければコンベアを停止
        ChangePhaseTmpDecelerate(); // 一時減速のフェーズを変更
        MakePlayingSound();         // 再生中の音を作成
    }
    DisableConveyor(); // コンベアを無効にする
    while (1)
    {
        OutputConveyor(0); // コンベアを停止状態にする
    }
}