// CardScannerTest.c
#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "Speaker.h"
#include "CardScanner.h"

int main()
{
    OSCCAL = 0xAD;
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitCardScanner();

    Wait(2000);          // 2秒待つ
    EnableCardScanner(); // カードスキャナーを有効にする
    while (1)
    {
        MakePlayingSound();
        if (CheckCardScanner)     // カードスキャナーのボタンが押されたかチェック
            DisableCardScanner(); // カードスキャナーを無効にする
        ChangePhaseCardScanner(); // カードスキャナーのフェーズを変更する
    }
}