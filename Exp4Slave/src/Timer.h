/*
 * オーバーフロー1回あたりのクロック数 256 * 256 = 65536(回)
 * クロック1回あたりのマイクロ秒数 = 1000000L / F_CPU = 1(µs)
 * オーバーフロー1回あたりのマイクロ秒数  = 65536 * 1 = 65536(µs)
 */

#ifndef TIMER_H
#define TIMER_H

#ifndef F_CPU
#define F_CPU 1000000L
#endif // F_CPU

#include <avr/io.h>
#include <avr/interrupt.h> // 割り込み制御のためのヘッダファイル

// プロトタイプ宣言
void InitTimer();                  // タイマーの初期化関数
unsigned long GetMillis();         // ミリ秒カウンタの値を取得する関数
void Wait(unsigned long waitTime); // 指定された時間だけ待機する関数

// グローバル変数
volatile uint64_t timerCount = 0; // 8Byteデータとして宣言 (∵GetMillis()のtimerMillisにてオーバーフローを防ぐため)

void InitTimer()
{
    TCCR0A = 0b00000000; // タイマーモードは標準
    TCCR0B = 0b00000100; // 分周比を256に設定
    TIMSK0 = 0b00000001; // オーバーフロー割込みを有効化
    sei();
}

// オーバーフロー割り込みハンドラ
ISR(TIMER0_OVF_vect)
{
    timerCount++;
}

unsigned long GetMillis()
{
    // cli(),sei()はtimerMicros読みとり中のtimerMicrosの更新を防ぐ
    cli();
    unsigned long timerMillis = (unsigned long)(timerCount * 65536 / 1000); // timerCountをμsにしてから1000で割ってmsにする。
    sei();
    return timerMillis;
}

void Wait(unsigned long waitTime)
{
    // 時間を取得
    unsigned long lastTime = GetMillis();
    // 指定時間経過するまで待機
    while (GetMillis() - lastTime < waitTime)
    {
    }
}

#endif // TIMER_H
