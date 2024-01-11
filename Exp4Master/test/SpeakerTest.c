#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

int main()
{
    InitTimer();                         // タイマーの初期化
    InitSpeaker();                       // スピーカーの初期化
    unsigned long tmpTime = GetMillis(); // 現在のミリ秒を取得してtmpTimeに格納
    while (GetMillis() - tmpTime < 2000)
    {                       // 2000ミリ秒間、以下の処理を繰り返す
        MakePlayingSound(); // 再生中の音を作成
    }
    SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗時の効果音を設定
    while (1)
    {                       // 無限ループ
        MakePlayingSound(); // 再生中の音を作成
    }
}
