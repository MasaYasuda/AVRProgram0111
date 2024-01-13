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
    if (flagEnableCardScanner == 1)
        return;
    flagEnableCardScanner = 1;
    OverlayMatrix(SMatrix, 12, 0, 4, 5);
    OverlayMatrix(CMatrix, 8, 0, 4, 5);
    OverlayMatrix(AMatrix, 4, 0, 4, 5);
    OverlayMatrix(NMatrix, 0, 0, 4, 5);
}

// 終了
void DisableCardScanner()
{
    if (flagEnableCardScanner == 0)
        return;
    flagEnableCardScanner = 0;
    OverlayMatrix(offlight, 0, 0, 16, 5);
}

#endif // CARD_SCAN_SLAVE_H