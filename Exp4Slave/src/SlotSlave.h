#ifndef SLOT_SLAVE_H_
#define SLOT_SLAVE_H_

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "ButtonsSlave.h"
#include "LEDMatrix.h"

// プロトタイプ宣言
int OrderCheckSlot(unsigned char RXdata);  // 受信データからスロットの命令をチェック
void SendResultSlot(unsigned char result); // スロットの結果送信
void Resetflag();                          // 再スタートのためのフラグリセット
void UpdateflagButton();                   // ボタンの押下を検出してフラグを更新
void OverlaySlotRoles();                   // スロットの役をLEDマトリクスに描画する
void UpdateSlotMatrix();                   // スロットのマトリクスを更新する
void JudgeMatchingRoles();                 // 揃っている役の番号を判定する（揃っていない場合は4を返す）
void OverlayBlinkMask();                   // 点滅するマスクを描画する
void _CreateRolesMatrix();                 // 役揃いを計算するためのRolesMatrixを作成する

// グローバル変数
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

void SendResultSlot(unsigned char result)
{
    // result:0=Failure, 1=Success, 2=Jackpot
    UARTTransmit(0b01000000 | result);
}

void Resetflag()
{
    for (int i = 0; i < 3; i++) // 各フラグをリセット
    {
        flagPressedButtons[i] = 0;
        flagStopedSlotLine[i] = 0;
    }
}

void UpdateflagButton()
{
    if (CheckButtonL())            // ボタンLが押されたら
        flagPressedButtons[0] = 1; // ボタン0のフラグを1にする
    if (CheckButtonC())            // ボタンCが押されたら
        flagPressedButtons[1] = 1; // ボタン1のフラグを1にする
    if (CheckButtonR())            // ボタンRが押されたら
        flagPressedButtons[2] = 1; // ボタン2のフラグを1にする
}

void OverlaySlotRoles()
{
    for (int j = 0; j < 3; j++) // 各列について
    {
        OverlayMatrix(zeroMatrix, 11 - (5 * j), slotMatrix[j][0], 5, 5);    // 0の役を描画
        OverlayMatrix(sevenMatrix, 11 - (5 * j), slotMatrix[j][1], 5, 5);   // 7の役を描画
        OverlayMatrix(xMatrix, 11 - (5 * j), slotMatrix[j][2], 5, 5);       // Xの役を描画
        OverlayMatrix(diamondMatrix, 11 - (5 * j), slotMatrix[j][3], 5, 5); // ダイヤの役を描画
        OverlayMatrix(offlight, 0, 15, 16, 1);                              // オフライトを描画
        OverlayMatrix(offlight, 0, 0, 1, 16);                               // オフライトを描画
    }
}

void UpdateSlotMatrix()
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

void JudgeMatchingRoles() （揃っていない場合4を返す）
{
    _CreateRolesMatrix(); // 役の行列を作成
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

void OverlayBlinkMask()
{
    for (int j = 0; j < 3; j++) // 各列について
    {
        OverlayMatrix(offlight, 11 - (5 * j), slotMatrix[j][matchingRole], 5, 5); // 点滅マスクをオーバーレイ
    }
}

void _CreateRolesMatrix()
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