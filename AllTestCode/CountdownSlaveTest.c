#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "ButtonsSlave.h"
#include "CountdownSlave.h"

int main()
{
    InitTimer();
    InitUART(9600);
    InitButtons();
    InitLEDMatrix(); // LEDマトリックスの初期化
    while (1)
    {
        if (UARTCheck() > 0) // 受信バッファの確認
        {
            unsigned char RXdata = UARTReceive(); // //受信バッファからデータを受信
            if (OrderCheckCountdown(RXdata))      // カウントダウンの順番をチェック
                EnableCountdown();                // カウントダウンを有効にする
        }
        ChangePhaseCountdown(); // カウントダウンのフェーズを変更
        LEDMatrixDisplay();     // LEDマトリックスを表示
    }
}