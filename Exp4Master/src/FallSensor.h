/**
 * センサー PC3
 */

#ifndef FALL_SENSOR_H
#define FALL_SENSOR_H

#include <avr/io.h>

// プロトタイプ宣言
void InitFallSensor(); // 初期化
int CheckFallSensor(); // チェック

void InitFallSensor()
{
    DDRC &= 0b11110111;
    PORTC |= 0b00001000;
}

int CheckFallSensor()
{
    if (((PINC >> 3) & 0b1) == 0)
    {
        return 1;
    }
    return 0;
}

#endif // FALL_SENSOR_H