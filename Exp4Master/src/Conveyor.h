/**
 * @details
 * ボタン　PB5
 * 出力　　PB3(OC2A) Timer2を使用
 * 参考文献: "試しながら学ぶAVR入門" p55
 */
#ifndef _CONVEYOR_H_
#define _CONVEYOR_H_

#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

int flagEnableConveyor = 0;         // コンベアの有効フラグ
unsigned char dutyRateConveyer = 0; // コンベアのデューティ比

// 一時減速（"フリーズ"）関連
int flagEnableTmpDecelerateConveyor = 0;           // 一時減速の有効フラグ
unsigned long timeEnableTmpDecelerateConveyor = 0; // 一時減速開始時刻

// 初期化関数
void InitConveyor()
{
    DDRB &= 0b11011111;  // PB5を入力に設定
    PORTB |= 0b00100000; // PB5にプルアップ抵抗を有効にする

    DDRB |= 0b00001000;   // PB3(OC2A)を出力に設定
    TCCR2A |= 0b00000011; // タイマー2を設定（PWMモード）
    TCCR2B |= 0b00000001; // クロックを規定
}

void EnableConveyor()
{
    flagEnableConveyor = 1; // コンベアを有効にする
}

// ボタンの状態確認
int CheckButtonConveyor()
{
    if (0 == ((PINB >> 5) & 0b1)) // ボタンが押されている（=0V）とき
        return 1;                 // ボタンが押されている
    else
        return 0; // ボタンが押されていない
}

void ChangePhaseTmpDecelerate()
{
    if (flagEnableConveyor == 0) // コンベアが無効の場合は何もしない
        return;
    if (flagEnableTmpDecelerateConveyor == 1) // 一時減速が有効の場合
    {
        if (GetMillis() - timeEnableTmpDecelerateConveyor > 5000) // 5秒経過したら
            flagEnableTmpDecelerateConveyor = 0;                  // 一時減速を無効にする
    }
}

/**
 * @brief 出力する
 * @param value 出力の大きさ(0~255)
 */
void OutputConveyor(unsigned char value)
{
    if (flagEnableConveyor == 0)                   // コンベアが無効の場合
        value = 0;                                 // 出力を0にする
    else if (flagEnableTmpDecelerateConveyor == 1) // 一時減速が有効の場合
    {
        value = (unsigned char)(((float)value) / 10); // 出力を1/10に減少
    }
    // PWM出力の設定
    if (0 == value) // 出力が0の場合
    {
        TCCR2A &= 0b01111111; // PWM出力無効
        PORTB &= 0b11110111;
    }
    else // 出力が0以外の場合
    {
        TCCR2A |= 0b10000000; // PWM出力有効化
        OCR2A = value;        // OCR2AにPWMの値を設定
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
    dutyRateConveyer = dutyRate; // dutyRateConveyerにdutyRateを設定する
}

#endif //_CONVEYOR_H_
