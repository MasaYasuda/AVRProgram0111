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