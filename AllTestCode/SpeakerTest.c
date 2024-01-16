// SpeakerTest.c
#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();                                                       // タイマーの初期化
    InitSpeaker();                                                     // スピーカーの初期化
    unsigned long tmpTime = GetMillis();                               // 現在のミリ秒を取得してtmpTimeに格納
    SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗時の効果音を設定
    while (GetMillis() - tmpTime < 5000)                               // 5秒間
    {
        MakeWaitingSound(); // 待機中の音を作成
    }

    tmpTime = GetMillis();
    SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功時の効果音を設定
    while (GetMillis() - tmpTime < 5000)
    {
        MakePlayingSound(); // PLAY中の音を作成
    }

    tmpTime = GetMillis();
    EnableWeirdSound(); // WeirdSoundを有効化
    while (GetMillis() - tmpTime < 20000)
    {                            // 無限ループ
        MakePlayingSound();      // PLAY中の音を作成
        ChangePhaseWeirdSound(); // 状態遷移
    }
}
