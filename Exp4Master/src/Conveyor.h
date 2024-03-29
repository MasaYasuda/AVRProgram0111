// Conveyor.h
/*
 * ボタン　PB5
 * 出力　　PB3(OC2A) Timer2を使用
 * 参考文献: "試しながら学ぶAVR入門" p55
 */
#ifndef _CONVEYOR_H_
#define _CONVEYOR_H_

#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

// プロトタイプ宣言
void InitConveyor();                              // コンベアを初期化する
void EnableConveyor();                            // コンベアを有効にする
int CheckButtonConveyor();                        // ボタンの状態を確認する
void ChangePhaseTmpDecelerate();                  // 一時減速のフェーズを変更する
void OutputConveyor(unsigned char value);         // コンベアに出力する
void DisableConveyor();                           // コンベアを無効にする
void TmpDecelerateConveyor();                     // 一時的に減速する設定する
void SetDutyRateConveyor(unsigned char dutyRate); // デューティ比を設定する

// グローバル変数
int flagEnableConveyor = 0;         // コンベア有効フラグ
unsigned char dutyRateConveyer = 0; // デューティ比

int flagEnableTmpDecelerateConveyor = 0;           // 一時減速の有効フラグ
unsigned long timeEnableTmpDecelerateConveyor = 0; // 一時減速開始時刻

/**
 * |TCCR2A|タイマ/カウンタ2制御レジスタA
 * 0bxxxx0011
 *   ││││││└┴ 波形生成種別|高速PWM動作なので11
 *   ||||└┴── 予約|
 *   ||└┴──── 比較B出力選択|
 *   └┴────── 比較A出力選択|OC2A切断は00,高速PWMの非反転動作は10
 * 参考資料:mega88.pdf (p.114)
 *
 * |TCCR2B|タイマ/カウンタ2制御レジスタB
 * 0b00000001
 *   │││││└┴┴ クロック選択|分周無しにするので001
 *   ||||└─── 波形生成種別|高速PWM動作なので1
 *   ||└┴──── 予約|
 *   |└────── OC2B強制変更|PWM動作なので関係なし
 *   └─────── OC2A強制変更|PWM動作なので関係なし
 * 参考資料:mega88.pdf (p.116)
 */
void InitConveyor()
{
    DDRB &= 0b11011111;  // PB5:入力
    PORTB |= 0b00100000; // PB5:プルアップ抵抗を有効

    DDRB |= 0b00001000;   // PB3(OC2A):出力
    TCCR2A |= 0b00000011; // タイマー2を設定（PWMモード）
    TCCR2B |= 0b00000001; // クロックを規定
}

void EnableConveyor()
{
    flagEnableConveyor = 1; // コンベアを有効にする
}

int CheckButtonConveyor()
{
    if (0 == ((PINB >> 5) & 0b1)) // ボタンが押されている（=0V）とき
        return 1;                 // ボタンが押されている
    else
        return 0;
}

void ChangePhaseTmpDecelerate()
{
    if (flagEnableConveyor == 0) // 無効の場合は何もしない
        return;
    if (flagEnableTmpDecelerateConveyor == 1) // 一時減速が有効の場合
    {
        if (GetMillis() - timeEnableTmpDecelerateConveyor > 5000) // 5秒経過したら
            flagEnableTmpDecelerateConveyor = 0;                  // 一時減速を無効にする
    }
}

//|OCR2A|タイマ/カウンタ2比較Aレジスタ  参考資料:mega88.pdf (p.117)
void OutputConveyor(unsigned char value)
{
    if (flagEnableConveyor == 0)              // コンベアが無効の場合
        value = 0;                            // 出力を0にする
    if (flagEnableTmpDecelerateConveyor == 1) // 一時減速が有効の場合
    {
        value = (unsigned char)(value / 3); // 出力を1/10に減少
    }

    // PWM出力の設定
    /*
     * @note 0でOCR2A=0としない理由(LightVariesLEDでも同様)
     * デューティサイクルが0%（つまり、信号が常に「オフ」）でも、PWM信号は完全に0Vにはならなかったため。
     */
    if (0 == value) // 出力が0の場合
    {
        TCCR2A &= 0b01111111; // 割込み無効
        PORTB &= 0b11110111;  // 0V出力
    }
    else // 出力が0以外の場合
    {
        TCCR2A |= 0b10000000; // PWM出力有効化
        OCR2A = value;        // OCR2Aに値を設定
    }
}

void DisableConveyor()
{
    OutputConveyor(0);
    flagEnableConveyor = 0; // コンベアを無効にする
}

void TmpDecelerateConveyor()
{
    flagEnableTmpDecelerateConveyor = 1;           // 一時減速を有効にする
    timeEnableTmpDecelerateConveyor = GetMillis(); // 一時減速開始時刻を記録
}

void SetDutyRateConveyor(unsigned char dutyRate)
{
    dutyRateConveyer = dutyRate; // dutyRateを設定する
}

#endif //_CONVEYOR_H_
