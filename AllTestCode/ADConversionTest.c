#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "Speaker.h"
#include "ADConversion.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitSpeaker();
    DDRC &= 0b11111110; // PC0を入力に設定
    while (1)
    {
        unsigned int VolumeValue = AnalogInput(0);
        SoundOutput(VolumeValue); // PC0の入力値によって音の高さが変化する
    }
}