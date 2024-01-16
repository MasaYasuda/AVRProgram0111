// CountdownSlave.h
#ifndef COUNTDOWN_SLAVE_H
#define COUNTDOWN_SLAVE_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "ButtonsSlave.h"
#include "LEDMatrix.h"

// プロトタイプ宣言
int OrderCheckCountdown(unsigned char RXdata);  // 受信データからカウントダウン命令をチェック
void SendResultCountdown(unsigned char result); // 結果を送信する　result:0=Failure, 1=Success
void EnableCountdown();                         // カウントダウンを有効にする
void ChangePhaseCountdown();                    // フェーズを変更する
int _CheckButtonCountdown();                    // ボタンの状態をチェックする
void DisableCountdown();                        // カウントダウンを無効にする

// グローバル宣言
int flagEnableCountdown = 0;           // カウントダウンが有効かどうかのフラグ
unsigned long timeEnableCountdown = 0; // カウントダウン開始時刻
int flagSuccessedCountdown = 0;        // カウントダウン成功フラグ

int OrderCheckCountdown(unsigned char RXdata)
{
    if (RXdata == 0b00100000)
        return 1; // 起動
    else
        return 0;
}

void SendResultCountdown(unsigned char result)
{
    UARTTransmit(0b00100000 | result);
}

void EnableCountdown()
{
    if (flagEnableCountdown == 1)
        return;
    flagEnableCountdown = 1;               // カウントダウンを有効にする
    timeEnableCountdown = GetMillis();     // 現在の時刻を取得
    flagSuccessedCountdown = 0;            // 成功フラグをリセット
    OverlayMatrix(offlight, 0, 5, 16, 11); // LEDマトリックスを初期状態に設定
}

void ChangePhaseCountdown()
{
    if (flagEnableCountdown == 0) // カウントダウンが無効なら何もしない
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
            SendResultCountdown(1);
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
            SendResultCountdown(0);
            DisableCountdown(); // カウントダウンを無効にする
        }
    }
}

int _CheckButtonCountdown()
{
    return CheckButtonC(); // ボタンCの状態を返す
}

void DisableCountdown()
{
    if (flagEnableCountdown == 0)
        return;
    OverlayMatrix(offlight, 0, 6, 16, 10); // LEDマトリックスを消灯
    flagEnableCountdown = 0;               // カウントダウンを無効にする
}

#endif // COUNTDOWN_SLAVE_H