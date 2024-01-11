#ifndef ADCONVERSION_H
#define ADCONVERSION_H

#include <avr/io.h>

unsigned int AnalogInput(unsigned int pinnum)
{
    ADMUX = 0b01000000 | (0b1111&pinnum); // AD変換ポート設定初期化(基準電圧はAVCC)
    ADCSRA |= 0b11000000; // AD変換許可＆開始
    while (0 == ((ADCSRA >> 4) & 0b1))
    {
    }
    unsigned int result = ADCL;
    result |= ADCH << 8;
    return result;
}

#endif //ADCONVERSION_H
/**
 * @brief
 * スキャナー上 PC1
 * スキャナー下 PC2
 * フォトインタラプタ　カードが挟まっているときAnalog値はおよそ0になる。逆にカードがないときAnalog値はおよそ1023になる。
 */
#ifndef CARD_SCANNER_H
#define CARD_SCANNER_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "ADConversion.h"
#include "Speaker.h"
#include "Conveyor.h"

void InitCardScanner();        // 初期化関数
void EnableCardScanner();      // 開始
int CheckCardScanner();        // スキャナーの状態確認
void ChangePhaseCardScanner(); // 動作の更新
void DisableCardScanner();     // 終了

int flagEnableCardScanner = 0;

unsigned int previousCardTop = 0;
unsigned int previousCardBottom = 0;
unsigned long timeExitCardTop = 0;

unsigned long timeEnableCardScanner = 0;
unsigned long limitTimeCardScanner = 10000;

// 初期化関数
void InitCardScanner()
{
    DDRC &= 0b11111001;
}

// 開始
void EnableCardScanner()
{
    if (flagEnableCardScanner == 1)
        return;
    UARTTransmit(0b00010001); // Slaveに描画送信
    timeEnableCardScanner = GetMillis();
    flagEnableCardScanner = 1;
}

// スキャナーの状態確認
int CheckCardScanner()
{
    if (flagEnableCardScanner == 0)
        return 0;

    unsigned long presentCardTop = AnalogInput(1);
    unsigned long presentCardBottom = AnalogInput(2);

    if (previousCardBottom < 500) // 前回カード下部が差し込まれている場合
    {
        if (previousCardTop > 500) // 前回カード上部は抜かれていた場合
        {
            if (presentCardBottom > 500) // 今回カード下部も抜かれている場合
            {
                unsigned long timediff = GetMillis() - timeExitCardTop; // カード上部と下部それぞれの抜き取りの時間差を計算
                if (timediff > 100 && timediff < 1000)                  // 時間差が一定の範囲内であったら
                {
                    previousCardTop = presentCardTop;
                    previousCardBottom = presentCardBottom;
                    {
                        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs);
                        return 1;
                    }
                }
            }
        }
        else // 前回カード上部も差し込まれていた場合
        {
            if (presentCardTop > 500) // 今回カード上部は抜かれている場合
            {
                timeExitCardTop = GetMillis();
            }
        }
    }

    previousCardTop = presentCardTop;
    previousCardBottom = presentCardBottom;
    return 0;
}

// 動作の更新
void ChangePhaseCardScanner()
{
    if (flagEnableCardScanner == 0)
        return;
    if (GetMillis() - timeEnableCardScanner > 10000) // 10秒以内に解除されていなければ時間切れ
    {
        TmpDecelerateConveyor();                                           // コンベアを一時的に減速
        SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
        DisableCardScanner();                                              // カードスキャナーを無効にする
    }
}

// 終了
void DisableCardScanner()
{
    UARTTransmit(0b00010000); // Slaveに描画終了送信
    flagEnableCardScanner = 0;
}

#endif // CARD_SCANNER_H
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
/**
 * @brief
 * ボタン　Slave
 * 出力　　Slave
 */

#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "Speaker.h"
#include "Conveyor.h"

int flagEnableCountdown = 0;

// 開始
void EnableCountdown()
{
     // Slaveに送信
     UARTTransmit(0b00100000);
     flagEnableCountdown = 1;
}

int ResultCheckCountdown(unsigned char RXdata) // カウントダウンの結果をチェックする
{
     if (RXdata == 0b00100001) // 成功の場合
     {
          SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
          return 1;
     }
     else if (RXdata == 0b00100000) // 失敗の場合
     {
          TmpDecelerateConveyor();                                           // コンベアを減速
          SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
          return 1;
     }
     else
          return 0; // それ以外の場合は0を返す
}

void DisableCountdown()
{
     flagEnableCountdown = 0;
}

#endif // COUNTDOWN_H
/**
 * センサー PC3
 */

#ifndef FALL_SENSOR_H
#define FALL_SENSOR_H

#include <avr/io.h>

void InitFallSensor()
{
    DDRC &= 0b11110111;
    PORTC|= 0b00001000;
}

int CheckFallSensor()
{
    if (((PINC >> 3) & 0b1) == 0)
    {
        return 1;
    }
    return 0;
}

#endif // FALL_SENSOR_H
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
#define F_CPU 1000000L
#include <stdlib.h>
#include "Timer.h"
#include "UART.h"
#include "Speaker.h"
#include "Conveyor.h"
#include "FallSensor.h"
#include "SlotMaster.h"
#include "JammerMotor.h"
#include "Countdown.h"
#include "CardScanner.h"
#include "LightVariesLED.h"
#include "WifiSwitch.h"

const char WAITING = 0; // 待機状態を表す定数
const char PLAYING = 1; // プレイ中状態を表す定数
const char SLOT = 2;    // スロットモードを表す定数
char mode = 0;          // 現在のモードを保持する変数

unsigned long timeStartPlaying = 0; // プレイ開始時間を保持する変数

int flagRX = 0;           // 受信フラグ、0で初期化
unsigned char RXdata = 0; // 受信データ、0で初期化

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();
    InitCardScanner();
    InitLightVariesLED();

    mode = WAITING; // 初期モードをWAITING状態に設定

    while (1)
    {
        if (mode == WAITING) // 現在のモードがWAITINGの場合
        {
            while (1)
            {
                MakeWaitingSound();
                if (CheckButtonConveyor()) // ボタンが押されたら開始準備
                {
                    timeStartPlaying = GetMillis();        // 現在の時間を取得し、プレイ開始時間として設定
                    srand((unsigned int)timeStartPlaying); // プレイ開始時間を元に乱数生成器を初期化
                    mode = SLOT;                           // モードをSLOTに変更
                    break;                                 // ループを抜ける
                }
            }
        }

        if (mode == PLAYING) // 現在のモードがプレイ中の場合
        {
            ResetMusic();     // 音楽をリセット
            EnableConveyor(); // コンベアを有効にする
            while (1)
            {
                MakePlayingSound(); // プレイ中の音を作成

                // データ受信
                flagRX = 0;          // 受信フラグをリセット
                if (UARTCheck() > 0) // UARTにデータがあるかチェック
                {
                    flagRX = 1;             // 受信フラグをセット
                    RXdata = UARTReceive(); // UARTからデータを受信
                }

                // イベントの生成
                int randEventNum = rand() % 10000; // ランダムなイベント番号を生成
                if (randEventNum < 10)
                    EnableWeirdSound(); // WeirdSoundを有効にする
                else if (randEventNum < 20)
                    EnableJammerMotor(); // ジャマモーターを有効にする
                else if (randEventNum < 30)
                    EnableCountdown(); // カウントダウンを有効にする
                else if (randEventNum < 40)
                    EnableCardScanner(); // カードスキャナーを有効にする
                else if (randEventNum < 50)
                    EnableLightVariesLED(); // LEDの光を有効にする

                //  イベント解除の確認
                if (CheckButtonWeirdSound())                  // WeirdSoundのボタンをチェック
                    DisableWeirdSound();                      // WeirdSoundを無効にする
                if (CheckButtonJammerMotor())                 // ジャマモーターのボタンをチェック
                    DisableJammerMotor();                     // ジャマモーターを無効にする
                if (CheckCardScanner())                       // カードスキャナーをチェック
                    DisableCardScanner();                     // カードスキャナーを無効にする
                if (CheckVolumeLED())                         // LEDのボリュームをチェック
                    DisableLightVariesLED();                  // LEDの点灯を停止する
                if (ResultCheckCountdown(RXdata) && (flagRX)) // カウントダウンの結果と受信フラグをチェック
                    DisableCountdown();                       // カウントダウンを無効にする

                // イベントの状態遷移
                ChangePhaseWeirdSound();     // WeirdSoundのフェーズを変更
                ChangePhaseJammerMotor();    // ジャマモーターのフェーズを変更
                ChangePhaseCardScanner();    // カードスキャナーのフェーズを変更
                ChangePhaseLightVariesLED(); // LEDの光のフェーズを変更

                // 落下センサーが反応したかチェック
                if (CheckFallSensor()) // 落下センサーをチェック
                {
                    DisableWeirdSound();     // WeirdSoundを無効にする
                    DisableJammerMotor();    // ジャマモーターを無効にする
                    DisableCountdown();      // カウントダウンを無効にする
                    DisableCardScanner();    // カードスキャナーを無効にする
                    DisableLightVariesLED(); // LEDの光を無効にする
                    DisableConveyor();       // コンベアを無効にする

                    SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
                    mode = SLOT;                                                                // モードをスロットモードに変更
                    break;
                }

                // プレイの制限時間を超過した場合
                if (GetMillis() - timeStartPlaying > 60000) // プレイ開始から60秒以上経過したかチェック
                {
                    DisableWeirdSound();     // WeirdSoundを無効にする
                    DisableJammerMotor();    // ジャマモーターを無効にする
                    DisableCountdown();      // カウントダウンを無効にする
                    DisableCardScanner();    // カードスキャナーを無効にする
                    DisableLightVariesLED(); // LEDの光を無効にする

                    SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
                    while (1)
                    {
                        OutputConveyor(dutyRateConveyer); // コンベアの出力を設定
                        if (CheckFallSensor())            // 落下センサーをチェック
                            break;
                        MakeWaitingSound(); // 待機音を作成
                    }
                    DisableConveyor(); // コンベアを無効にする
                    mode = WAITING;    // モードを待機モードに戻る
                    break;
                }
            }
        }

        if (mode == SLOT) // 現在のモードがスロットモードの場合
        {
            ResetMusic();
            EnableSlot(); // スロットを有効にする
            while (1)
            {
                MakeWaitingSound();
                if (UARTCheck() > 0) // UARTにデータがあるかチェック
                {
                    unsigned char RXdata = UARTReceive(); // UARTからデータを受信
                    if (ResultCheckSlot(RXdata))          // スロットの結果をチェック
                    {
                        DisableSlot();  // スロットを無効にする
                        mode = PLAYING; // モードをプレイ中状態に戻す
                        break;          // 内側のwhileループを抜ける
                    }
                }   
            }
        }
    }
}
/**
 * @brief
 * ボタン　Slave
 * 出力　　Slave
 */

#ifndef SLOT_MASTER_H
#define SLOT_MASTER_H

#include <avr/io.h>
#include "UART.h"
#include "Timer.h"
#include "Speaker.h"

int flagEnableSlot = 0; // スロット有効フラグ

void EnableSlot() // スロットを有効にする
{
    // Slaveに開始信号を送信
    UARTTransmit(0b01000000);
    SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
    flagEnableSlot = 1;                                                         // スロットを有効にする
}

void DisableSlot() // スロットを無効にする
{
    flagEnableSlot = 0; // スロットを無効にする
}

int ResultCheckSlot(unsigned char RXdata) // スロットの結果をチェックする
{
    if (RXdata == 0b01000010) // 大当たりの場合
    {
        SetSoundEffect(SEJackpotLength, SEJackpotIntervals, SEJackpotPitchs); // Jackpot音を設定
        SetDutyRateConveyor(255);                                             // コンベアのデューティ比を最大に設定
        return 1;
    }
    else if (RXdata == 0b01000001) // アタリの場合
    {
        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SEFailedPitchs); // 成功音を設定
        SetDutyRateConveyor(200);                                                // コンベアのデューティ比を中間に設定
        return 1;
    }
    else if (RXdata == 0b01000000) // ハズレの場合
    {
        SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
        SetDutyRateConveyor(150);                                          // コンベアのデューティ比を低めに設定
        return 1;
    }
    else
        return 0;
}

#endif // SLOT_MASTER_H
/**
 * @details Timer1を使用（CTCモード）
 * PB1に出力　(一応変更可)
 * ボタン PB6
 */

#ifndef SPEAKER_H_
#define SPEAKER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer.h"
#include "Conveyor.h"

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/
void SoundOutput(unsigned int frequency);										  // 周波数に応じた音を出力
void ResetMusic();																  // 音楽をリセット
void InitSpeaker();																  // スピーカーを初期化
void MakeWaitingSound();														  // 待機音を生成
void MakePlayingSound();														  // 再生音を生成
void SetSoundEffect(int length, unsigned int intervals[], unsigned int pitchs[]); // 効果音を設定
void EnableWeirdSound();														  // WeirdSoundを有効化
int CheckButtonWeirdSound();													  // WeirdSound用ボタンのチェック
void ChangePhaseWeirdSound();													  // WeirdSoundのフェーズ変更
void DisableWeirdSound();														  // WeirdSoundを無効化

// 音階の定義
#define C4 262	// ド
#define D4 294	// レ
#define E4 330	// ミ
#define F4 349	// ファ
#define G4 392	// ソ
#define A4 440	// ラ
#define B4 494	// シ
#define C5 523	// ド（1オクターブ上）
#define D5 587	// レ（1オクターブ上）
#define E5 659	// ミ（1オクターブ上）
#define F5 698	// ファ（1オクターブ上）
#define G5 784	// ソ（1オクターブ上）
#define A5 880	// ラ（1オクターブ上）
#define B5 988	// シ（1オクターブ上）
#define C6 1046 // ド（2オクターブ上）

// WAITING MODEのデフォルト音楽
int IndexWaitingMusic = 0;										// 待機音楽の現在のインデックス
unsigned long previousTimeSwitchedWaitingMusic = 0;				// 前回Indexを変更した時間
int WAITING_MUSIC_INDEX_LENGTH = 4;								// モード選択中のデフォルト音楽の長さ
unsigned int wAITING_MUSIC_INTERVALS[4] = {500, 500, 500, 500}; // 音の間隔
unsigned int WAITING_MUSIC_PITCHS[4] = {F4, 0, C4, 0};			// 音の高さ

// PLAYING MODEのデフォルト音楽
int IndexPlayingMusic = 0;																									 // 再生音楽の現在のインデックス
unsigned long previousTimeSwitchedPlayingMusic = 0;																			 // 前回Indexを変更した時間
int PLAYING_MUSIC_INDEX_LENGTH = 16;																						 // プレイ中のデフォルト音楽の長さ
unsigned int PLAYING_MUSIC_INTERVALS[16] = {400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400}; // 音の間隔
unsigned int PLAYING_MUSIC_PITCHS[16] = {C4, C4, G4, G4, A4, A4, G4, 0, F4, F4, E4, E4, D4, D4, C4, 0};						 // きらきら星の音階

// 効果音関連
int flagEnableSoundEffect = 0;					   // 効果音を有効にするフラグ
int IndexSoundEffect = 0;						   // 効果音の現在のインデックス
unsigned long previousTimeSwitchedSoundEffect = 0; // 前回Indexを変更した時間
int soundEffectIndexLength = 0;					   // 効果音の長さ
unsigned int soundEffectIntervals[20] = {0};	   // 効果音の間隔
unsigned int soundEffectPitchs[20] = {0};		   // 効果音の高さ

// JammerMotor警告効果音
int SEJammerMotorLength = 7;												  // JammerMotor警告効果音の長さ
unsigned int SEJammerMotorIntervals[7] = {250, 250, 250, 250, 250, 250, 250}; // JammerMotor警告効果音の間隔
unsigned int SEJammerMotorPitchs[7] = {B5, F5, B5, F5, B5, F5, 0};			  // JammerMotor警告効果音のピッチ

// SlotJackpot効果音
int SEJackpotLength = 7;												  // SlotJackpot効果音の長さ
unsigned int SEJackpotIntervals[7] = {400, 400, 300, 100, 200, 600, 600}; // SlotJackpot効果音の間隔
unsigned int SEJackpotPitchs[7] = {D4, B4, A4, C5, 0, B4, 0};			  // SlotJackpot効果音のピッチ

// 成功時効果音
int SESuccessedLength = 4;									 // 成功時効果音の長さ
unsigned int SESuccessedIntervals[4] = {100, 100, 200, 100}; // 成功時効果音の間隔
unsigned int SESuccessedPitchs[4] = {G5, E4, C6, 0};		 // 成功時効果音の高さ

// 失敗時効果音
int SEFailedLength = 4;									  // 失敗時効果音の長さ
unsigned int SEFailedIntervals[4] = {100, 100, 200, 100}; // 失敗時効果音の間隔
unsigned int SEFailedPitchs[4] = {B4, A4, F4, 0};		  // 失敗時効果音の高さ

// WeirdSound関連
int flagEnableWeirdSound = 0;			  // WeirdSoundを有効にするフラグ
int timeEnableWeirdSound = 0;			  // WeirdSoundを有効にした時間
unsigned long limitTimeWeirdSound = 5000; // WeirdSoundの制限時間

/**
 * @brief 音を出力する関数
 * @param frequency 目標の周波数（約7.63Hzから250kHzまで）
 */
// 音を出力する関数を定義します
void SoundOutput(unsigned int frequency)
{
	if (frequency == 0) // 指定周波数が0のときタイマーの割込みを停止する
	{
		TIMSK1 = 0x00;		 // 割込みを停止する
		PORTB &= 0b11111101; // PB1の出力をLOWに設定する
	}
	else
	{
		// 周波数を2倍にします
		unsigned int tmp = frequency * 2;
		// OCR1AにCPUクロック周波数を周波数の2倍で割った値から1を引いた値を設定します。
		// これにより、目標の周波数に対応するビットが設定されます。
		OCR1A = F_CPU / tmp - 1;
		// タイマー1の比較Aマッチ割り込みを許可します
		TIMSK1 = 0b00000010;
		// 割り込みを有効にします
		sei();
	}
}

// タイマー割込みで実行される関数
ISR(TIMER1_COMPA_vect)
{
	PINB ^= 0b00000010; // PB1の出力を反転する
}

void ResetMusic()
{
	IndexPlayingMusic = 0;				  // 再生音楽のインデックスをリセット
	previousTimeSwitchedPlayingMusic = 0; // 再生音楽の時間をリセット
	IndexWaitingMusic = 0;				  // 待機音楽のインデックスをリセット
	previousTimeSwitchedWaitingMusic = 0; // 待機音楽の時間をリセット
}

// 初期化関数
void InitSpeaker()
{
	// クロック校正
	OSCCAL = 0b10000000; // クロック校正値を設定

	DDRB |= 0b00000010;	 // PB1を出力設定にする
	TCCR1A = 0b00000000; // CTCモードを設定する
	TCCR1B = 0b00001001; // CTCモードと分周比(1)を設定する

	DDRB &= 0b10111111;	 // PB6(ボタン)を入力設定にする
	PORTB |= 0b01000000; // PB6(ボタン)をプルアップする
	ResetMusic();		 // スピーカーの設定をリセット
}

void MakeWaitingSound()
{
	int pitch = 0; // 再生する音の高さ

	if (flagEnableSoundEffect == 1) // 効果音を鳴らすとき
	{
		pitch = soundEffectPitchs[IndexSoundEffect];												// 効果音の高さを取得
		if (GetMillis() - previousTimeSwitchedSoundEffect > soundEffectIntervals[IndexSoundEffect]) // 次の効果音に移るタイミングか
		{
			IndexSoundEffect++;								// 効果音のインデックスを進める
			previousTimeSwitchedSoundEffect = GetMillis();	// 時間を更新
			pitch = soundEffectPitchs[IndexSoundEffect];	// 新しい効果音の高さを取得
			if (IndexSoundEffect == soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常の待機音を再生
	{
		pitch = WAITING_MUSIC_PITCHS[IndexWaitingMusic];												 // 待機音の高さを取得
		if (GetMillis() - previousTimeSwitchedWaitingMusic > wAITING_MUSIC_INTERVALS[IndexWaitingMusic]) // 次の音に移るタイミングか
		{
			IndexWaitingMusic = (IndexWaitingMusic + 1) % WAITING_MUSIC_INDEX_LENGTH; // 待機音のインデックスを進める
			previousTimeSwitchedWaitingMusic = GetMillis();							  // 時間を更新
			pitch = WAITING_MUSIC_PITCHS[IndexWaitingMusic];						  // 新しい待機音の高さを取得
		}
	}
	SoundOutput(pitch); // 音を出力
}

void MakePlayingSound()
{
	int pitch = 0; // 再生する音の高さ

	if (flagEnableSoundEffect == 1) // 効果音を鳴らすとき
	{
		pitch = soundEffectPitchs[IndexSoundEffect];												// 効果音の高さを取得
		if (GetMillis() - previousTimeSwitchedSoundEffect > soundEffectIntervals[IndexSoundEffect]) // 次の効果音に移るタイミングか
		{
			IndexSoundEffect++;								// 効果音のインデックスを進める
			previousTimeSwitchedSoundEffect = GetMillis();	// 時間を更新
			pitch = soundEffectPitchs[IndexSoundEffect];	// 新しい効果音の高さを取得
			if (IndexSoundEffect == soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常の再生音を再生
	{
		pitch = PLAYING_MUSIC_PITCHS[IndexPlayingMusic];												 // 再生音の高さを取得
		if (GetMillis() - previousTimeSwitchedPlayingMusic > PLAYING_MUSIC_INTERVALS[IndexPlayingMusic]) // 次の音に移るタイミングか
		{
			IndexPlayingMusic = (IndexPlayingMusic + 1) % PLAYING_MUSIC_INDEX_LENGTH; // 再生音のインデックスを進める
			previousTimeSwitchedPlayingMusic = GetMillis();							  // 時間を更新
			pitch = PLAYING_MUSIC_PITCHS[IndexPlayingMusic];						  // 新しい再生音の高さを取得
		}

		if (flagEnableWeirdSound == 1)								 // WeirdSoundが有効なとき
			if (pitch != 0)											 // pitchが0、すなわち元の音が"無音"ではないとき
				pitch = (unsigned int)(((float)pitch * 0.62) + 100); // WeirdSoundのピッチを変更
	}
	SoundOutput(pitch); // 音を出力
}

// SoundEffect部分
// 効果音を設定
void SetSoundEffect(int length, unsigned int intervals[], unsigned int pitchs[])
{
	IndexSoundEffect = 0;							 // 効果音のインデックスをリセット
	flagEnableSoundEffect = 1;						 // 効果音を有効にする
	soundEffectIndexLength = length;				 // 効果音の長さを設定
	for (int i = 0; i < soundEffectIndexLength; i++) // 効果音の配列を設定
	{
		soundEffectIntervals[i] = intervals[i]; // 効果音の間隔を設定
		soundEffectPitchs[i] = pitchs[i];		// 効果音の高さを設定
	}
	previousTimeSwitchedSoundEffect = GetMillis(); // 時間を更新
}

// WeiredSound部分
//  開始
void EnableWeirdSound()
{
	if (flagEnableWeirdSound == 1) // 既にWeirdSoundが有効なら何もしない
		return;
	flagEnableWeirdSound = 1;			// WeirdSoundを有効にする
	timeEnableWeirdSound = GetMillis(); // WeirdSoundを有効にした時間を記録
}

// ボタンの状態確認
int CheckButtonWeirdSound()
{
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return 0;
	if (((PINB >> 6) & 0b1) == 0) // ボタンが押されていたら1を返す
	{
		SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs);
		return 1;
	}

	return 0; // それ以外は0を返す
}

void ChangePhaseWeirdSound()
{
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return;
	if (GetMillis() - timeEnableWeirdSound > 5000) // 失敗
	{
		TmpDecelerateConveyor();										   // コンベアを減速
		SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 効果音を流す
		DisableWeirdSound();											   // WeirdSoundを無効にする
	}
}

void DisableWeirdSound()
{
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return;
	flagEnableWeirdSound = 0; // WeirdSoundを無効にする
}

#endif // SPEAKER_H_
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
#ifndef UART_H_
#define UART_H_

// バッファの長さの定義
#define TX_BUF_LEN 16 // 送信バッファの長さ
#define RX_BUF_LEN 16 // 受信バッファの長さ

#include <avr/io.h>        // AVRの入出力を制御するためのヘッダファイル
#include <avr/interrupt.h> // AVRの割込みを制御するためのヘッダファイル

// 受信バッファと送信バッファのインデックス
volatile unsigned int RxBufHead; // 受信バッファの中の一番古いデータの要素番号
volatile unsigned int RxBufTail; // 受信バッファの中の一番新しいデータの要素番号
volatile unsigned int TxBufHead; // 送信バッファの中の一番古いデータの要素番号
volatile unsigned int TxBufTail; // 送信バッファの中の一番新しいデータの要素番号

// 受信バッファと送信バッファ
volatile unsigned char RxBuf[RX_BUF_LEN];
volatile unsigned char TxBuf[TX_BUF_LEN];

/**
 * @brief UARTの設定を行う関数
 * @param baudrate ボーレート。基本は9600を代入
 */
void InitUART(unsigned long baudrate)
{
    // Try u2x mode first
    uint16_t baudrateSetting = (F_CPU / 4 / baudrate - 1) / 2; // ボーレートの設定
    UCSR0A = 0b00000011;                                       // MPCMは0でも良い。

    UBRR0H = baudrateSetting >> 8; // ボーレートレジスタの上位バイト
    UBRR0L = baudrateSetting;      // ボーレートレジスタの下位バイト

    UCSR0C = 0b00000110; // フレームフォーマットの設定

    UCSR0B |= 0b10011000; // 受信完了割込みと送信完了割込みを有効にする
    UCSR0B &= 0b11011111; // 9ビットデータモードを無効にする

    sei(); // 割込みを有効にする
}

/**
 * @brief 受信バッファにデータがあるかを確認する関数
 * @return int バッファにあるバイト数
 */
int UARTCheck(void)
{
    return ((unsigned int)(RX_BUF_LEN + RxBufHead - RxBufTail)) % RX_BUF_LEN;
}

/**
 * @brief 受信バッファからデータを取得する関数
 * @return unsigned char
 * @note 必ずUARTCheckで受信バッファが空でないことを確認してから実行
 */
unsigned char UARTReceive(void)
{
    unsigned char c = RxBuf[RxBufTail];                // データを取得
    RxBufTail = (unsigned char)(RxBufTail + 1) % RX_BUF_LEN; // インデックスを更新
    return c;                                          // データを返す
}

// 新たにUART受信した時の割込み関数
ISR(USART_RX_vect)
{
    if (0 == ((UCSR0A >> 2) & 0x01)) // フレームエラーがない場合
    {
        unsigned char c = UDR0;                                      // データレジスタからデータを取得
        unsigned int i = (unsigned int)(RxBufHead + 1) % RX_BUF_LEN; // 次のインデックスを計算

        if (RxBufTail != i) // バッファが満杯でない場合
        {
            RxBuf[RxBufHead] = c; // データをバッファに格納
            RxBufHead = i;        // インデックスを更新
        }
    }
    else
    {
        UDR0; // フレームエラーがある場合、データを読み捨てる
    };
}

/**
 * @brief データを送信バッファに格納する関数
 * @param c 送信する1バイトデータ
 */
void UARTTransmit(unsigned char c)
{
    if (TxBufHead == TxBufTail && 1 == ((UCSR0A >> 5) & 0x01)) // バッファが空で、送信可能な場合
    {
        /**
         * 以下の4行に関して。データをデータレジスタに格納(UDR0 = c;)し、
         * 送信を開始(UCSR0A |= 0b01000011;)するこの2つの操作は
         * 、一連の操作としてアトミック（不可分）に行いたいため、割込みが発生しないようにしています。
         */
        cli();                // 割込みを無効にする
        UDR0 = c;             // データをデータレジスタに格納
        UCSR0A |= 0b01000000; // 送信を開始
        sei();                // 割込みを有効にする
    }
    else
    {
        unsigned char i = (TxBufHead + 1) % TX_BUF_LEN; // 次のインデックスを計算
        TxBuf[TxBufHead] = c;                     // データをバッファに格納

        cli();                // 割込みを無効にする
        TxBufHead = i;        // インデックスを更新
        UCSR0B |= 0b00100000; // 送信完了割込みを有効にする
        sei();                // 割込みを有効にする
    }

    return; // 送信成功
}

// 送信バッファが空いたときに実行する割込み関数
ISR(USART_UDRE_vect)
{
    unsigned char c = TxBuf[TxBufTail];       // バッファからデータを取得
    TxBufTail = (TxBufTail + 1) % TX_BUF_LEN; // インデックスを更新

    UDR0 = c;             // データをデータレジスタに格納
    UCSR0A |= 0b01000000; // 送信を開始

    if (TxBufHead == TxBufTail) // バッファが空になった場合
    {
        UCSR0B &= 0b11011111; // 送信完了割込みを無効にする
    }
}

#endif // UART_H_
/**
 * Wifiスイッチ
 * 入力スイッチ PB4
 */
#ifndef WIFI_SWITCH_H
#define WIFI_SWITCH_H

#include <avr/io.h>

void InitWifiSwitch()
{
    DDRB &= 0b11101111; // PB4を入力として設定
    PORTB |= 0b00010000; // PB4にプルアップ抵抗を有効にする
}

// LOWで1が戻る
int CheckWifiSwitch()
{
    if ((PINB >> 4) & 0b1 == 0) // PB4がLOWの場合は1を返す
        return 1;
    return 0; // それ以外の場合は0を返す
}

#endif // WIFI_SWITCH_H
#ifndef BUTTONS_SLAVE_H
#define BUTTONS_SLAVE_H

#include <avr/io.h>

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

void InitButtons();
// 各ボタンの押下の検出
int CheckButtonL();
int CheckButtonC();
int CheckButtonR();

/**
 * スロットのボタン
 * ボタンL PD4
 * ボタンC PD2
 * ボタンR PD3
 */

void InitButtons(){
    DDRD &= 0b11100011;  // 入力モード
    PORTD |= 0b00011100; // プルアップ
}

// 各ボタンの押下の検出
int CheckButtonL()
{
    if (((PIND >> 4) & 0b1) == 0)
    {
        return 1;
    }
    return 0;
}
int CheckButtonC()
{
    if (((PIND >> 2) & 0b1) == 0)
    {
        return 1;
    }
    return 0;
}
int CheckButtonR()
{
    if (((PIND >> 3) & 0b1 )== 0)
    {
        return 1;
    }
    return 0;
}

#endif // BUTTONS_SLAVE_H
#ifndef CARD_SCAN_SLAVE_H
#define CARD_SCAN_SLAVE_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"

int flagEnableCardScanner = 0;


// 0:無関係 1:起動 2:終了
int OrderCheckCardScanner(unsigned char RXdata)
{
    if (RXdata == 0b00010001)
        return 1; // 起動
    else if (RXdata == 0b00010000)
        return 2; // 停止
    else
        return 0;
}


// 開始
void EnableCardScanner()
{
    flagEnableCardScanner = 1;
    OverlayMatrix(SMatrix, 12, 0, 4, 5);
    OverlayMatrix(CMatrix, 8, 0, 4, 5);
    OverlayMatrix(AMatrix, 4, 0, 4, 5);
    OverlayMatrix(NMatrix, 0, 0, 4, 5);
}

// 終了
void DisableCardScanner()
{
    flagEnableCardScanner = 0;
    OverlayMatrix(offlight, 0, 0, 16, 5);
}

#endif // CARD_SCAN_SLAVE_H
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <avr/io.h>

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

/**
 * COMSignal  PB0
 * ...プルアップ
 * 確認時...LOWになってなければLOWを出力。送信終了したらプルアップに戻す
 * 
 */

void InitComSignal(){
    DDRB &= 0b11111110;  // 入力モード
    PORTB |= 0b00000001; // プルアップ
}
void WaitComSignal(){
    while((PINB&0b1)==0){
    }
    DDRB |= 0b00000001; // 出力モード
    PORTB &= 0b11111110;  // LOWを出力
}

#endif // COMMUNICATION_H
#ifndef COUNTDOWN_SLAVE_H
#define COUNTDOWN_SLAVE_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "ButtonsSlave.h"
#include "CommunicationSlave.h"
#include "LEDMatrix.h"

int flagEnableCountdown = 0;          // カウントダウンが有効かどうかのフラグ
unsigned int timeEnableCountdown = 0; // カウントダウン開始時刻

int flagSuccessedCountdown = 0; // カウントダウン成功フラグ

void EnableCountdown();      // カウントダウンを有効にする関数
void ChangePhaseCountdown(); // カウントダウンの各フェーズを変更する関数
int _CheckButtonCountdown(); // ボタンの状態を確認する関数
void DisableCountdown();     // カウントダウンを無効にする関数

int OrderCheckCountdown(unsigned char RXdata)
{
    if (RXdata == 0b00100000)
        return 1; // 起動
    else
        return 0;
}

// result:0=Failure, 1=Success
void SendResultCountdown(unsigned char result)
{
    UARTTransmit(0b00100000 | result);
}

// 開始
void EnableCountdown()
{
    flagEnableCountdown = 1;               // カウントダウンを有効にする
    timeEnableCountdown = GetMillis();     // 現在の時刻を取得
    flagSuccessedCountdown = 0;            // 成功フラグをリセット
    OverlayMatrix(offlight, 0, 5, 16, 11); // LEDマトリックスを初期状態に設定
}
// 動作の更新
void ChangePhaseCountdown()
{
    if (!flagEnableCountdown) // カウントダウンが無効なら何もしない
        return;

    unsigned long tmpTime = GetMillis() - timeEnableCountdown; // 経過時間を計算
    if (tmpTime < 1000)
    {
        OverlayMatrix(fiveMatrix, 6, 8, 5, 5); // 5を表示
        return;
    }
    else if (tmpTime < 2000)
    {
        OverlayMatrix(fourMatrix, 6, 8, 5, 5); // 4を表示
        return;
    }
    else if (tmpTime < 3000)
    {
        OverlayMatrix(threetrix, 6, 8, 5, 5); // 3を表示
        return;
    }
    else if (tmpTime < 4000)
    {
        OverlayMatrix(twoMatrix, 6, 8, 5, 5); // 2を表示
        return;
    }
    else if (tmpTime < 5000)
    {
        OverlayMatrix(oneMatrix, 6, 8, 5, 5); // 1を表示
        return;
    }
    else if (tmpTime < 6000)
    {
        OverlayMatrix(onlight, 0, 6, 16, 10);  // LEDマトリックスを点灯
        OverlayMatrix(zeroMatrix, 6, 8, 5, 5); // 0を表示
        if (_CheckButtonCountdown())           // ボタンが押されたか確認
        {
            flagSuccessedCountdown = 1; // 成功フラグをセット
        }
        else if (!flagSuccessedCountdown) // 成功していなければ何もしない
            return;
    }

    if (flagSuccessedCountdown) // 成功している場合
    {
        if (tmpTime < 7000)
        {
            OverlayMatrix(offlight, 0, 6, 16, 10);     // LEDマトリックスを消灯
            OverlayMatrix(diamondMatrix, 6, 8, 5, 5);  // ダイヤモンドを表示
            OverlayMatrix(diamondMatrix, 1, 8, 5, 5);  // ダイヤモンドを表示
            OverlayMatrix(diamondMatrix, 11, 8, 5, 5); // ダイヤモンドを表示
        }
        else
        {
            WaitComSignal();
            SendResultCountdown(1);
            InitComSignal();
            DisableCountdown(); // カウントダウンを無効にする
        }
    }
    else // 成功していない場合
    {
        if (tmpTime < 7000)
        {
            OverlayMatrix(xMatrix, 6, 8, 5, 5); // Xを表示
        }
        else
        {
            WaitComSignal();
            SendResultCountdown(0);
            InitComSignal();
            DisableCountdown(); // カウントダウンを無効にする
        }
    }
}

// ボタンの状態確認
int _CheckButtonCountdown()
{
    return CheckButtonC(); // ボタンCの状態を返す
}

// 終了
void DisableCountdown()
{
    OverlayMatrix(offlight, 0, 6, 16, 10); // LEDマトリックスを消灯
    flagEnableCountdown = 0;               // カウントダウンを無効にする
}

#endif // COUNTDOWN_SLAVE_H
/**
 * @file LEDMatrix.h
 * @details 回路接続
 * ---------------------
 * 74HC595    -- AVR
 * SER(data)  -- PC0
 * RCK(latch) -- PC1
 * SCK(clock) -- PC2
 * ---------------------
 */

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <avr/io.h>

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

unsigned int zeroMatrix[5];
unsigned int oneMatrix[5];
unsigned int sevenMatrix[5];
unsigned int xMatrix[5];
unsigned int diamondMatrix[5];
unsigned int offlight[16];
unsigned int onlight[16];

unsigned int SMatrix[5];
unsigned int CMatrix[5];
unsigned int AMatrix[5];
unsigned int NMatrix[5];

unsigned int twoMatrix[5];
unsigned int threetrix[5];
unsigned int fourMatrix[5];
unsigned int fiveMatrix[5];

static unsigned int _pallet[16]; // 描画用マトリクス

// LEDマトリックスのセットアップを行う関数
void InitLEDMatrix();
// palletにobjを描く
void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m);
// LEDマトリックスにpalletを表示する
void LEDMatrixDisplay();
static void _SetBitsFromXtoY(unsigned int *y, unsigned int x, int p, unsigned char n);

// 16x16 LEDマトリックス用のビットパターン
unsigned int zeroMatrix[5] = {
    0b10001,
    0b10101,
    0b10101,
    0b10101,
    0b10001};

unsigned int oneMatrix[5] = {
    0b10011,
    0b11011,
    0b11011,
    0b11011,
    0b11011};
unsigned int sevenMatrix[5] = {
    0b10001,
    0b11101,
    0b11101,
    0b11101,
    0b11101};

unsigned int xMatrix[5] = {
    0b11111,
    0b10101,
    0b11011,
    0b10101,
    0b11111};

unsigned int diamondMatrix[5] = {
    0b11011,
    0b10001,
    0b00000,
    0b10001,
    0b11011};

unsigned int offlight[16] = {
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF};

unsigned int onlight[16] = {
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000};

unsigned int SMatrix[5] = {
    0b1000,
    0b0011,
    0b1001,
    0b1100,
    0b0001};
unsigned int CMatrix[5] = {
    0b1001,
    0b0010,
    0b0011,
    0b0010,
    0b1001};
unsigned int AMatrix[5] = {
    0b1001,
    0b0010,
    0b0010,
    0b0000,
    0b0010};
unsigned int NMatrix[5] = {
    0b0110,
    0b0010,
    0b0000,
    0b0100,
    0b0110};

unsigned int twoMatrix[5] = {
    0b10001,
    0b11101,
    0b10001,
    0b10111,
    0b10001};
unsigned int threetrix[5] = {
    0b10001,
    0b11101,
    0b10001,
    0b11101,
    0b10001};
unsigned int fourMatrix[5] = {
    0b10101,
    0b10101,
    0b10001,
    0b11101,
    0b11101};
unsigned int fiveMatrix[5] = {
    0b10001,
    0b10111,
    0b10001,
    0b11101,
    0b10001};

// 描画用マトリクス
static unsigned int _pallet[16] = {
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF};

// LEDマトリックスのセットアップを行う関数
void InitLEDMatrix()
{
    // SER(データ)=PC0 RCK(ラッチ)=PC1   SCK(クロック)=PC2
    DDRC |= 0b00000111; // PC0, PC1, PC2を出力に設定する
}

void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m)
{
    for (int i = q; i < m + q; i++)
    {
        if (i < 0)
            continue;
        if (i > 15)
            break;
        _SetBitsFromXtoY(&_pallet[i], obj[i - q], p, n);
    }
}

void LEDMatrixDisplay()
{
    for (int j = 0; j < 16; j++)
    {

        PORTC &= 0b11111101; // ラッチ(PC1)をLOWに設定

        unsigned int row = 0x8000 >> j;
        unsigned char colH = _pallet[j] >> 8;
        unsigned char colL = _pallet[j] & 0xFF;

        unsigned long data = ((unsigned long)row << 16) | ((unsigned long)colH << 8) | colL;

        for (int i = 0; i < 32; i++)
        {
            PORTC = (PORTC & 0b11111110) | (data & 0x01); // SER(PC0)にデータをセット
            data >>= 1;

            PORTC |= 0b00000100; // クロック(PC2)をHIGHに設定
            PORTC &= 0b11111011; // クロック(PC2)をLOWに設定
        }

        PORTC |= 0b00000010; // ラッチ(PC1)をHIGHに設定

        //_delay_ms(1); // 1ミリ秒待つ（上手く調節する）
    }
}

void _SetBitsFromXtoY(unsigned int *y, unsigned int x, int p, unsigned char n)
{
    if (p < 0)
    {
        // pが負の場合はyの0ビット目から(p+n-1)ビット目を使用
        n = n + p;
        if (n - 1 > 15)
            n = 16;                               // オーバーフローをチェックし、必要に応じてnを調整
        unsigned int mask = ((1 << n) - 1);       // nビットのマスクを作成し、aビット目にシフト
        *y = (*y & ~mask) | ((x >> (-p)) & mask); // yのa~(p+n-1)ビット目をクリアし、xの0~(n-1)ビット目をセット
    }
    else
    {
        if (p + n - 1 > 15)
            n = 16 - p;                          // オーバーフローをチェックし、必要に応じてnを調整
        unsigned int mask = ((1 << n) - 1) << p; // nビットのマスクを作成し、aビット目にシフト
        *y = (*y & ~mask) | ((x << p) & mask);   // yのa~(p+n-1)ビット目をクリアし、xの0~(n-1)ビット目をセット
    }
}
#endif // LEDMATRIX_H
#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "ButtonsSlave.h"
#include "CommunicationSlave.h"
#include "CardScanSlave.h"
#include "CountdownSlave.h"
#include "SlotSlave.h"

const char WAITING = 0; // 待機状態を表す定数
const char PLAYING = 1; // プレイ中状態を表す定数
const char SLOT = 2;	// スロットモードを表す定数
char mode = 0;			// 現在のモードを保持する変数

int main()
{
	InitTimer(); // タイマーの初期化
	InitUART(9600);
	InitButtons();	 // ボタンの初期化
	InitLEDMatrix(); // LEDマトリックスの初期化
	InitComSignal();
	mode = PLAYING; // 初期モードをプレイ中状態に設定

	while (1)
	{
		if (mode == PLAYING) // プレイ中の場合
		{
			OverlayMatrix(offlight, 0, 0, 16, 16); // LEDマトリックスをオフにする
			while (1)							   // 内部無限ループ
			{
				if (UARTCheck() > 0) // UARTにデータがあるかチェック
				{
					unsigned char RXdata = UARTReceive();		 // UARTからデータを受信
					if (OrderCheckCountdown(RXdata))			 // カウントダウン命令のチェック
						EnableCountdown();						 // カウントダウンを有効にする
					else if (OrderCheckCardScanner(RXdata) == 1) // カードスキャナー起動命令のチェック
						EnableCardScanner();					 // カードスキャナーを有効にする
					else if (OrderCheckCardScanner(RXdata) == 2) // カードスキャナー停止命令のチェック
						DisableCardScanner();					 // カードスキャナーを無効にする
					else if (OrderCheckSlot(RXdata))			 // スロットモード命令のチェック
					{
						mode = SLOT; // モードをスロットモードに変更
						break;		 // 内部ループを抜ける
					}
				}
				ChangePhaseCountdown(); // カウントダウンのフェーズを変更する
				LEDMatrixDisplay();		// LEDマトリックスに表示する
			}
		}
		if (mode == SLOT) // スロットモードの場合
		{
			Resetflag(); // フラグをリセット
			while (1)
			{
				UpdateflagButton();																// ボタンの状態を更新
				UpdateSlotMatrix();																// スロットマトリックスを更新
				OverlaySlotRoles();																// スロットの役をオーバーレイ
				LEDMatrixDisplay();																// LEDマトリックスに表示
				if (flagStopedSlotLine[0] + flagStopedSlotLine[1] + flagStopedSlotLine[2] == 3) // 全ての列が止まったか判定
					break;																		// 全ての列が止まっていたらループを抜ける
			}

			unsigned long timeSlotStopped = GetMillis(); // スロットが止まった時間を取得
			JudgeMatchingRoles();						 // 小役揃い判定を行う
			if (matchingRole != 4)						 // 揃っている場合
			{
				while ((GetMillis() - timeSlotStopped) < 2000) // 2秒間点滅表示を行う
				{
					OverlaySlotRoles();			   // 役をオーバーレイ
					if ((GetMillis() % 500) > 250) // 0.25秒ごとに点滅
						OverlayBlinkMask();		   // 点滅マスクをオーバーレイ
					LEDMatrixDisplay();			   // LEDマトリックスに表示
				}
				// Masterマイコンに結果を送信する処理をここに記述
				if (matchingRole == 0)
					SendResultSlot(2); // 大当たり
				else
					SendResultSlot(1); // 当たり
			}
			else
			{
				while ((GetMillis() - timeSlotStopped) < 2000) // 0.5秒静止表示を行う
					LEDMatrixDisplay();						   // LEDマトリックスに表示

				// Masterマイコンに結果を送信する処理をここに記述
				SendResultSlot(0); // ハズレ
			}

			mode = PLAYING; // モードをスロットモードに戻す
		}
	}
}
#ifndef SLOT_SLAVE_H_
#define SLOT_SLAVE_H_

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "ButtonsSlave.h"
#include "CommunicationSlave.h"
#include "LEDMatrix.h"


/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

void UpdateflagButton();     // ボタンの押下状態を更新する関数
void Resetflag();            // 揃いフラグをリセットする関数
void JudgeMatchingRoles();   // 揃っている役の番号を返す関数（揃っていない場合4を返す）
void OverlaySlotRoles();     // スロットの役をオーバーレイする関数
void UpdateSlotMatrix();     // スロットのマトリクスを更新する関数
void OverlayBlinkMask();     // 点滅マスクをオーバーレイする関数
void _CreatePatternMatrix(); // 役揃いを簡便に計算するための行列(rolesMatrix)を作成する関数

int slotMatrix[3][4] = {{0, 5, 10, -5}, {0, -5, 10, 5}, {0, 10, 5, -5}}; // スロットの行列
int rolesMatrix[3][4];                                                   // 役の行列
int flagPressedButtons[3] = {0};                                         // ボタンが押されていたら1となるフラグ
int flagStopedSlotLine[3] = {0};                                         // スロットの列が止まったら1となるフラグ
int matchingRole = 0;                                                    // 揃っている役の番号（揃っていない場合）
unsigned long timeChangedRolesPosition = 0;                              // 役の位置が変更された時間


int OrderCheckSlot(unsigned char RXdata)
{
    if (RXdata == 0b01000000)
        return 1; // 起動
    else
        return 0;
}

// result:0=Failure, 1=Success, 2=Jackpot
void SendResultSlot(unsigned char result)
{
    WaitComSignal();
    UARTTransmit(0b01000000 | result);
    InitComSignal();
}

void Resetflag() // 再スタートのためのフラグリセット
{
    for (int i = 0; i < 3; i++) // 各フラグをリセット
    {
        flagPressedButtons[i] = 0;
        flagStopedSlotLine[i] = 0;
    }
}

void UpdateflagButton() // ボタンの押下の検出
{
    if (CheckButtonL())            // ボタンLが押されたら
        flagPressedButtons[0] = 1; // ボタン0のフラグを1にする
    if (CheckButtonC())            // ボタンCが押されたら
        flagPressedButtons[1] = 1; // ボタン1のフラグを1にする
    if (CheckButtonR())            // ボタンRが押されたら
        flagPressedButtons[2] = 1; // ボタン2のフラグを1にする
}

void OverlaySlotRoles() // スロットの役をオーバーレイする関数
{
    for (int j = 0; j < 3; j++) // 各列について
    {
        OverlayMatrix(zeroMatrix, 11 - (5 * j), slotMatrix[j][0], 5, 5);    // 0の役をオーバーレイ
        OverlayMatrix(sevenMatrix, 11 - (5 * j), slotMatrix[j][1], 5, 5);   // 7の役をオーバーレイ
        OverlayMatrix(xMatrix, 11 - (5 * j), slotMatrix[j][2], 5, 5);       // Xの役をオーバーレイ
        OverlayMatrix(diamondMatrix, 11 - (5 * j), slotMatrix[j][3], 5, 5); // ダイヤの役をオーバーレイ
        OverlayMatrix(offlight, 0, 15, 16, 1);                              // オフライトをオーバーレイ
        OverlayMatrix(offlight, 0, 0, 1, 16);                               // オフライトをオーバーレイ
    }
}

void UpdateSlotMatrix() // スロットのマトリクスを更新する関数
{
    if (GetMillis() - timeChangedRolesPosition > 20) // 前回の役位置変更から20ms以上経過していたら
    {
        for (int i = 0; i < 3; i++) // 各列について
        {
            if ((flagPressedButtons[i] == 1) && (slotMatrix[i][0] % 5 == 0)) // ボタンが押されていて、スロットの位置が5の倍数なら
            {
                flagStopedSlotLine[i] = 1; // スロットの列を止める
            }
            else // それ以外の場合
            {
                for (int j = 0; j < 4; j++) // 各行について
                {
                    slotMatrix[i][j]++;        // 役を更新
                    if (slotMatrix[i][j] > 14) // 最大描画領域まで行ったとき
                        slotMatrix[i][j] = -5; // 最小値に戻す
                }
            }
        }
        timeChangedRolesPosition = GetMillis(); // 役の位置変更時間を更新
    }
}

void JudgeMatchingRoles() // 揃っている役の番号を返す（揃っていない場合4を返す）
{
    _CreatePatternMatrix(); // 役の行列を作成
    // 各行、各列、各斜めの揃い判定
    if ((rolesMatrix[0][0] == rolesMatrix[1][0]) && (rolesMatrix[0][0] == rolesMatrix[2][0])) // 0行目揃い判定
    {
        matchingRole = rolesMatrix[0][0];
    }
    else if ((rolesMatrix[0][1] == rolesMatrix[1][1]) && (rolesMatrix[0][1] == rolesMatrix[2][1])) // 1行目揃い判定
    {
        matchingRole = rolesMatrix[0][1];
    }
    else if ((rolesMatrix[0][2] == rolesMatrix[1][2]) && (rolesMatrix[0][2] == rolesMatrix[2][2])) // 2行目揃い判定
    {
        matchingRole = rolesMatrix[0][2];
    }
    else if ((rolesMatrix[0][2] == rolesMatrix[1][1]) && (rolesMatrix[0][2] == rolesMatrix[2][0])) // スラッシュ揃い判定
    {
        matchingRole = rolesMatrix[0][2];
    }
    else if ((rolesMatrix[0][0] == rolesMatrix[1][1]) && (rolesMatrix[0][0] == rolesMatrix[2][2])) // バックスラッシュ揃い判定
    {
        matchingRole = rolesMatrix[0][0];
    }
    else // 揃っていない場合
        matchingRole = 4;
}
void OverlayBlinkMask() // 点滅マスクをオーバーレイする関数
{
    for (int j = 0; j < 3; j++) // 各列について
    {
        OverlayMatrix(offlight, 11 - (5 * j), slotMatrix[j][matchingRole], 5, 5); // 点滅マスクをオーバーレイ
    }
}

void _CreatePatternMatrix() // 役揃いを簡便に計算するための行列(rolesMatrix)を作成する
{
    for (int h = 0; h < 3; h++) // 各列について
    {
        for (int j = 0; j < 4; j++) // 各行について
        {
            for (int i = 0; i < 4; i++) // 各柄番号について
            {
                if (slotMatrix[h][i] == -5 + ((((j + 1) * 5)) % 20)) // スロットの位置が一致していたら
                {
                    rolesMatrix[h][j] = i; // 役の行列を更新
                }
            }
        }
    }
}

#endif // SLOT_SLAVE_H_


