#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "Speaker.h"
#include "Conveyor.h"

// プロトタイプ宣言
void EnableCountdown();                         // 開始
int ResultCheckCountdown(unsigned char RXdata); // カウントダウンの結果をチェックする
void DisableCountdown();                        // 無効化

// グローバル変数
int flagEnableCountdown = 0;

void EnableCountdown()
{
     if (flagEnableCountdown == 1) // 既に有効なら何もしない
          return;
     UARTTransmit(0b00100000); // Slaveに開始命令を送信
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