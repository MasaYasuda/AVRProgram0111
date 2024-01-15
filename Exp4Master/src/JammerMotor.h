/*
 * ボタン　PB7
 * 出力　　PB2
 */

#ifndef JAMMER_MOTOR_H
#define JAMMER_MOTOR_H

#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

// プロトタイプ宣言
void InitJammerMotor();        // 初期化
void EnableJammerMotor();      // 有効化
void ChangePhaseJammerMotor(); // フェーズ変更
int CheckButtonJammerMotor();  // ボタンの状態確認
void DisableJammerMotor();     // 無効化

// グローバル変数
int flagEnableJammerMotor = 0;
unsigned long timeEnableJammerMotor = 0;

// 初期化関数
void InitJammerMotor()
{
    DDRB &= 0b01111111;  // PB7を入力設定
    PORTB |= 0b10000000; // PB7をプルアップ
    DDRB |= 0b00000100;  // PB2を出力設定
}

void EnableJammerMotor()
{
    if (flagEnableJammerMotor == 1)
        return;
    flagEnableJammerMotor = 1;
    timeEnableJammerMotor = GetMillis();
    SetSoundEffect(SEJammerMotorLength, SEJammerMotorIntervals, SEJammerMotorPitchs); // JammerMotor警告音を鳴らす
}

void ChangePhaseJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return;
    if (GetMillis() - timeEnableJammerMotor > 5000) // 五秒以内に解除されていなければ
    {
        PORTB |= 0b00000100; // 出力オン(回り始める)
    }
}

int CheckButtonJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return 0;
    if (0 == ((PINB >> 7) & 0b1)) // ボタンが押されている（=0V）とき
    {
        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を鳴らす
        return 1;
    }
    else
        return 0;
}

void DisableJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return;
    PORTB &= 0b11111011; // 出力オフ
    flagEnableJammerMotor = 0;
}

#endif // JAMMER_MOTOR_H