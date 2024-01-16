// SlotTest.c
#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "Slot.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    Wait(2000); // 少し待機

    EnableSlot(); // スロットを有効にする(命令送信)
    while (1)
    {
        MakeSlotSound();
        if (UARTCheck() > 0) // UARTにデータがあるかチェック
        {
            unsigned char RXdata = UARTReceive(); // UARTからデータを受信
            if (ResultCheckSlot(RXdata))          // スロットの結果をチェック
            {
                DisableSlot(); // スロットを無効にする
            }
        }
    }
}