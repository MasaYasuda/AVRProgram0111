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
