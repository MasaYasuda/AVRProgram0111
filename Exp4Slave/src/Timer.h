#ifndef TIMER_H
#define TIMER_H

#ifndef F_CPU
#define F_CPU 1000000L
#endif // F_CPU

#include <avr/io.h>        // AVRの入出力ポートを制御するためのヘッダファイルをインクルード
#include <avr/interrupt.h> // AVRの割り込みを制御するためのヘッダファイルをインクルード

/**
 * @details
 * クロック1回あたりのマイクロ秒数 = 1000000L / F_CPU = 1
 * オーバーフロー1回あたりのクロック数 256 * 256 = 65536
 * オーバーフロー1回あたりのマイクロ秒数 (=32000) = 65536 * 1 =65536
 */

volatile uint64_t timerCount = 0; // 8Byteデータとして宣言,because GetMillis()のtimerMillisにてオーバーフローを防ぐため

// タイマーの初期化関数
void InitTimer()
{
    TCCR0A = 0b00000000; // タイマーモードは標準
    TCCR0B = 0b00000100; // 分周比を256に設定
    TIMSK0 = 0b00000001; // オーバーフロー割込みを有効化
    sei();               // 割り込みを有効化
}

// タイマー0のオーバーフロー割り込みハンドラ
ISR(TIMER0_OVF_vect)
{
    timerCount++;
}

/**
 * @brief ミリ秒カウンタの値を取得する関数
 * @return unsigned long
 */
unsigned long GetMillis()
{
    // cli(),sei()はtimerMicros読みとり中のtimerMicrosの更新を防ぐ
    cli();
    unsigned long timerMillis = (unsigned long)(timerCount * 65536 / 1000);
    sei();
    return timerMillis; // ミリ秒カウンタの値を返す
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
