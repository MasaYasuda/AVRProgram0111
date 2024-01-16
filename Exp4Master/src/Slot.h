// Slot.h
/**
 * ボタン　Slave
 * 出力　　Slave
 */

#ifndef SLOT_H
#define SLOT_H

#include <avr/io.h>
#include "UART.h"
#include "Timer.h"
#include "Speaker.h"

// プロトタイプ宣言
void EnableSlot();                         // スロットを有効にする
void DisableSlot();                        // スロットを無効にする
int ResultCheckSlot(unsigned char RXdata); // スロットの結果チェック

// グローバル宣言
int flagEnableSlot = 0; // 有効フラグ

void EnableSlot()
{
    UARTTransmit(0b01000000);                                                   // Slaveに開始信号を送信
    SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
    flagEnableSlot = 1;                                                         // スロットを有効にする
}

int ResultCheckSlot(unsigned char RXdata)
{
    if (RXdata == 0b01000010) // 大当たりの場合
    {
        SetSoundEffect(SEJackpotLength, SEJackpotIntervals, SEJackpotPitchs); // Jackpot音を設定
        SetDutyRateConveyor(255);                                             // コンベアのデューティ比を最大に設定
        return 1;
    }
    else if (RXdata == 0b01000001) // アタリの場合
    {
        SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
        SetDutyRateConveyor(200);                                                   // コンベアのデューティ比を中間に設定
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

void DisableSlot()
{
    flagEnableSlot = 0; // スロットを無効にする
}

#endif // SLOT_H
