#ifndef CARD_SCAN_SLAVE_H
#define CARD_SCAN_SLAVE_H

#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"

// プロトタイプ宣言
int OrderCheckCardScanner(unsigned char RXdata); // 受信データから命令をチェック (戻り値＝0:無関係 1:起動 2:終了)
void EnableCardScanner();                        // 命令描画開始
void DisableCardScanner();                       // 命令描画終了

// グローバル変数
int flagEnableCardScanner = 0;

int OrderCheckCardScanner(unsigned char RXdata)
{
    if (RXdata == 0b00010001)
        return 1; // 起動
    else if (RXdata == 0b00010000)
        return 2; // 停止
    else
        return 0;
}

void EnableCardScanner()
{
    if (flagEnableCardScanner == 1)
        return;
    flagEnableCardScanner = 1;
    OverlayMatrix(SMatrix, 12, 0, 4, 5);
    OverlayMatrix(CMatrix, 8, 0, 4, 5);
    OverlayMatrix(AMatrix, 4, 0, 4, 5);
    OverlayMatrix(NMatrix, 0, 0, 4, 5);
}

void DisableCardScanner()
{
    if (flagEnableCardScanner == 0)
        return;
    flagEnableCardScanner = 0;
    OverlayMatrix(offlight, 0, 0, 16, 5);
}

#endif // CARD_SCAN_SLAVE_H