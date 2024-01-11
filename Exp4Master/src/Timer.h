#ifndef TIMER_H
#define TIMER_H

#ifndef F_CPU
#define F_CPU 1000000L
#endif // F_CPU

#include <avr/io.h>        // AVRの入出力ポートを制御するためのヘッダファイルをインクルード
#include <avr/interrupt.h> // AVRの割り込みを制御するためのヘッダファイルをインクルード

#define CLOCK_MICROS (1000000L / F_CPU)                    // クロック1回あたりのマイクロ秒数 (=1)
#define OVERFLOW_CLOCKNUM (64 * 256)                       // オーバーフロー1回あたりのクロック数 (=16384)
#define OVERFLOW_MICROS (OVERFLOW_CLOCKNUM * CLOCK_MICROS) // オーバーフロー1回あたりのマイクロ秒数 (=16384)

#define MILLIS_UNIT (OVERFLOW_MICROS / 1000)    // オーバーフロー1回あたりのミリ秒数(小数点以下切り捨て) (=16)
#define REMAINDER_UNIT (OVERFLOW_MICROS % 1000) // オーバーフロー1回あたりのミリ以下の余分 (=384)

volatile unsigned long TimerMillis = 0;        // タイマーのミリ秒カウンタ
volatile unsigned int TimerRemainderCount = 0; // タイマーの余分カウンタ

// タイマーの初期化関数
void InitTimer()
{
    TCCR0A = 0b00000000; // タイマーモードは標準
    TCCR0B = 0b00000011; // 分周比を64に設定
    TIMSK0 = 0b00000001; // オーバーフロー割込みを有効化
    sei();               // 割り込みを有効化
}

// タイマー0のオーバーフロー割り込みハンドラ
ISR(TIMER0_OVF_vect)
{
    unsigned long ms = TimerMillis;         // ミリ秒カウンタの現在値を取得
    unsigned int cnt = TimerRemainderCount; // 余分カウンタの現在値を取得

    ms += MILLIS_UNIT;     // ミリ秒カウンタを増分
    cnt += REMAINDER_UNIT; // 余分カウンタを増分
    if (cnt >= 1000)       // 余分カウンタが最大値を超えた場合
    {
        ms += 1;     // ミリ秒カウンタを1増分
        cnt -= 1000; // 余分カウンタから最大値を引く
    }

    TimerMillis = ms;          // ミリ秒カウンタを更新
    TimerRemainderCount = cnt; // 余分カウンタを更新
}

/**
 * @brief ミリ秒カウンタの値を取得する関数
 * @return unsigned long
 */
unsigned long GetMillis()
{
    unsigned long ms;

    cli();            // 割り込みを無効化
    ms = TimerMillis; // ミリ秒カウンタの値を取得
    sei();            // 割り込みを有効化

    return ms; // ミリ秒カウンタの値を返す
}

/**
 * @brief 指定された時間だけ待機する関数
 * @param waitTime ミリ秒
 */
void Wait(unsigned long waitTime)
{
    // 最後に確認した時間を取得
    unsigned long lastTime = GetMillis();
    // 現在の時間が最後に確認した時間から指定された時間だけ経過するまで待機
    while (GetMillis() - lastTime < waitTime)
    {
    }
}

#endif // TIMER_H
