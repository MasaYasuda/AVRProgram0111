/**
 * @brief
 * ボタン　PB7
 * 出力　　PB2
 */

#ifndef JAMMER_MOTOR_H
#define JAMMER_MOTOR_H

#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"

void InitJammerMotor();
void EnableJammerMotor();
void ChangePhaseJammerMotor();
int CheckButtonJammerMotor();
void DisableJammerMotor();

int flagEnableJammerMotor = 0;
unsigned long timeEnableJammerMotor = 0;

// 初期化関数
void InitJammerMotor()
{
    DDRB &= 0b01111111;  // PB7を入力設定
    PORTB |= 0b10000000; // PB7をプルアップ
    DDRB |= 0b00000100;  // PB2を出力設定
}

// 開始
void EnableJammerMotor()
{
    if (flagEnableJammerMotor == 1)
        return;
    flagEnableJammerMotor = 1;
    timeEnableJammerMotor = GetMillis();
    SetSoundEffect(SEJammerMotorLength, SEJammerMotorIntervals, SEJammerMotorPitchs);
}
// 動作の更新
void ChangePhaseJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return;
    if (GetMillis() - timeEnableJammerMotor > 5000) // 五秒以内に解除されていなければ
    {
        PORTB |= 0b00000100; // 出力オン
        // 効果音を設定
    }
}

// ボタンの状態確認
int CheckButtonJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return 0;
    if (0 == ((PINB >> 7) & 0b1)) // ボタンが押されている（=0V）とき
    {
        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs);
        return 1;
    }
    else
        return 0;
}

// 終了
void DisableJammerMotor()
{
    if (flagEnableJammerMotor == 0)
        return;
    PORTB &= 0b11111011; // 出力オフ
    flagEnableJammerMotor = 0;
}



#endif // JAMMER_MOTOR_H