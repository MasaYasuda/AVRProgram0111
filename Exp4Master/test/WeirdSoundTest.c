#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

int main()
{
    InitTimer();                         // タイマーの初期化
    InitSpeaker();                       // スピーカーの初期化
    unsigned long tmpTime = GetMillis(); // 現在のミリ秒を取得してtmpTimeに格納
    // 通常の再生音を2000ミリ秒間再生する
    while (GetMillis() - tmpTime < 2000)
    {
        MakePlayingSound(); // 再生中の音を作成
    }

    // WeirdSoundを有効にする
    EnableWeirdSound(); // WeirdSoundを有効化

    tmpTime = GetMillis(); // 現在のミリ秒を再取得してtmpTimeに格納
    // WeirdSoundを10000ミリ秒間再生する
    while (GetMillis() - tmpTime < 10000)
    {
        MakePlayingSound(); // 再生中の音を作成
    }

    // WeirdSoundを無効にする
    DisableWeirdSound(); // WeirdSoundを無効化

    // 通常の再生音を無限に再生する
    while (1)
    {
        MakePlayingSound(); // 再生中の音を作成
    }
}
