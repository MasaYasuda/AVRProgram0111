/**
 * @brief
 * ボリューム　PC0
 * 出力　　PD3(OC2B)
 */

#ifndef LIGHT_VARIES_LED_H
#define LIGHT_VARIES_LED_H

#include <avr/io.h>
#include "Timer.h"
#include "ADConversion.h"
#include "Speaker.h"
#include "Conveyor.h"

void InitLightVariesLED();        // LEDの初期化
void EnableLightVariesLED();      // LEDの点灯を開始する
void ChangePhaseLightVariesLED(); // LEDの点灯状態を更新する
int CheckVolumeLED();             // ボリュームの状態を確認する
void DisableLightVariesLED();     // LEDの点灯を停止する

int flagEnableLightVariesLED = 0;              // LED点灯の有効フラグ
unsigned long timeEnableLightVariesLED = 0;    // LED点灯開始時刻
unsigned long limitTimeLightVariesLED = 10000; // LED点灯の制限時間

unsigned int targetLightVariesLED = 0; // LEDの目標点灯値

// LEDの初期化
void InitLightVariesLED()
{
    DDRC &= 0b11111110;   // PC0を入力設定
    PORTC |= 0b00000001;  // PC0にプルアップ抵抗を設定
    DDRD |= 0b00001000;   // PD3(OC2B)を出力設定
    PORTD |= 0b00001000;  // PD3に初期値を設定
    TCCR2A |= 0b00000011; // タイマー2をPWMモードで設定
    TCCR2B |= 0b00000001; // タイマー2のクロックを設定
}

// LEDの点灯を開始する
void EnableLightVariesLED()
{
    if (flagEnableLightVariesLED == 1) // 既に有効な場合は何もしない
        return;

    PORTD &= 0b11110111;                    // PD3をLOWに設定
    timeEnableLightVariesLED = GetMillis(); // 現在の時間を記録
    flagEnableLightVariesLED = 1;           // LEDの変動を有効にする

    unsigned int presentVolumeLED = AnalogInput(0); // 現在のボリューム値を取得
    if (presentVolumeLED < 341)                     // ボリューム値が341未満の場合
        targetLightVariesLED = 511;                 // 目標値を511に設定

    else if (presentVolumeLED < 682) // ボリューム値が682未満の場合
    {
        int randNum = (int)((presentVolumeLED % 2) * 2) - 1; // -1または1をランダムに生成
        targetLightVariesLED = 511 + 341 * randNum;          // 目標値を計算して設定
    }
    else                            // それ以外の場合
        targetLightVariesLED = 511; // 目標値を511に設定
}

// LEDの点灯状態を更新する
void ChangePhaseLightVariesLED()
{
    if (flagEnableLightVariesLED == 0) // LEDが有効でない場合は何もしない
        return;

    int diff = (int)(abs(targetLightVariesLED - AnalogInput(0)) / 2); // 目標値と現在値の差分の絶対値を計算
    if (diff > 255)                                                   // 差分が255を超える場合は255に制限
        diff = 255;
    int value = 255 - diff; // PWMの値を計算
    /**
     * @note 0でOCR2B=0としない理由
     * デューティサイクルが0%（つまり、信号が常に「オフ」）でも、PWM信号は完全に0Vにはならない。
     * これはPWMの特性上、PWM信号がデジタルからアナログへの変換を行う際に、一部の電圧が「漏れ」てしまうため。
     */
    if (0 == value) // PWMの値が0の場合
    {
        TCCR2A &= 0b11001111; // PWM出力を無効にする
    }
    else // PWMの値が0以外の場合
    {
        TCCR2A |= 0b00100000; // PWM出力を有効にする
        OCR2B = value;        // OCR2BにPWMの値を設定
    }

    if (GetMillis() - timeEnableLightVariesLED > limitTimeLightVariesLED) // 制限時間を超えた場合
    {
        // 時間切れ
        TmpDecelerateConveyor();
        SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
        DisableLightVariesLED();                                           // LEDの点灯を停止する
    }
}

// ボリュームの状態を確認する
int CheckVolumeLED()
{
    if (flagEnableLightVariesLED == 0) // LEDが有効でない場合は何もしない
        return 0;

    int diff = (int)abs(targetLightVariesLED - AnalogInput(0)); // 目標値と現在値の差分の絶対値を計算
    if (diff == 0)                                              // 差分が0の場合
    {
        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs);
        return 1;
    }
    return 0; // それ以外の場合は0を返す
}

// LEDの点灯を停止する
void DisableLightVariesLED()
{
    PORTD |= 0b00001000;          // PD3をHIGHに設定
    flagEnableLightVariesLED = 0; // LED点灯の有効フラグを無効にする
}

#endif // LIGHT_VARIES_LED_H
