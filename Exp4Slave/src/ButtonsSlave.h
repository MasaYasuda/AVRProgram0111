/**
 * スロットのボタン
 * ボタンL PD4
 * ボタンC PD2
 * ボタンR PD3
 */

#ifndef BUTTONS_SLAVE_H
#define BUTTONS_SLAVE_H

#include <avr/io.h>

// プロトタイプ宣言
void InitButtons(); // 初期化
int CheckButtonL(); // 左ボタンの押下の検出
int CheckButtonC(); // 中ボタンの押下の検出
int CheckButtonR(); // 右ボタンの押下の検出

void InitButtons()
{
    DDRD &= 0b11100011;  // 入力モード
    PORTD |= 0b00011100; // プルアップ
}

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
    if (((PIND >> 3) & 0b1) == 0)
    {
        return 1;
    }
    return 0;
}

#endif // BUTTONS_SLAVE_H