#ifndef ADCONVERSION_H
#define ADCONVERSION_H

#include <avr/io.h>

unsigned int AnalogInput(unsigned int pinnum)
{
    ADMUX = 0b01000000 | (0b1111&pinnum); // AD変換ポート設定初期化(基準電圧はAVCC)
    ADCSRA |= 0b11000000; // AD変換許可＆開始
    while (0 == ((ADCSRA >> 4) & 0b1))
    {
    }
    unsigned int result = ADCL;
    result |= ADCH << 8;
    return result;
}

#endif //ADCONVERSION_H