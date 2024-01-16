// Timer.h
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

/**
 * |TCCR0A|タイマ/カウンタ0制御レジスタＡ
 * 0b00000000
 *   ││││││└┴ 波形生成種別|標準動作なので00
 *   ||||└┴── 予約|
 *   ||└┴──── 比較B出力選択|00で標準ポート動作(OC0B切断)
 *   └┴────── 比較A出力選択|00で標準ポート動作(OC0A切断)
 * 参考資料:mega88.pdf (p.78)
 *
 * |TCCR0B|タイマ/カウンタ0制御レジスタＢ
 * 0b00000100
 *   │││││└┴┴ クロック選択|256分周させるので100 (分周比は割込みの頻度による動作遅延と、時刻の精度を考慮し設定した)
 *   ||||└─── 波形生成種別|標準動作なので0
 *   ||└┴──── 予約|
 *   |└────── OC0B強制変更|直ちに比較一致を強制する必要はないので0
 *   └─────── OC0A強制変更|直ちに比較一致を強制する必要はないので0
 * 参考資料:mega88.pdf (p.80)
 *
 * |TIMSK0|タイマ/カウンタ0割り込み許可レジスタ
 * 0b00000001
 *   │││││││└ タイマ／カウンタ0溢れ割り込み許可|使用するので1
 *   ||||||└─ タイマ／カウンタ01比較A割り込み許可|使用しないので0
 *   |||||└── タイマ／カウンタ0比較B割り込み許可|使用しないので0
 *   └┴┴┴┴─── 予約|
 * 参考資料:mega88.pdf (p.82)
 *
 * ※ 補足：sei(),cli()について
 * |SREG|ステータスレジスタ
 * 7bit目… 全割り込み許可|sei()で1,cli()で0とできる。
 * 参考資料:mega88.pdf (p.16)
 */
void InitTimer()
{
    TCCR0A = 0b00000000; // タイマーモードは標準
    TCCR0B = 0b00000100; // 分周比を256に設定
    TIMSK0 = 0b00000001; // オーバーフロー割込みを有効化
    sei();
}

// オーバーフロー時の割込みベクタ  参考資料:mega.pdf (p.46)
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
