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
    if (flagEnableCardScanner == 0)
        return ;
    UARTTransmit(0b00010000); // Slaveに描画終了送信
    flagEnableCardScanner = 0;
}

#endif // CARD_SCANNER_H