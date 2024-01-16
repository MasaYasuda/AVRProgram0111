#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "ButtonsSlave.h"
#include "SlotSlave.h"

int main()
{
    // 各モジュール初期化
    InitTimer();
    InitUART(9600);
    InitButtons();
    InitLEDMatrix();

    // メインループ
    while (1)
    {
        // LEDマトリックスをオフにする
        OverlayMatrix(offlight, 0, 0, 16, 16);
        // スロットモード開始を待つループ
        while (1)
        {
            // UARTにデータがあるかチェック
            if (UARTCheck() > 0)
            {
                unsigned char RXdata = UARTReceive();
                // スロットモード開始命令が来たらループを抜ける
                if (OrderCheckSlot(RXdata))
                    break;
            }
            // LEDマトリックスに表示
            LEDMatrixDisplay();
        }

        Resetflag();
        // スロットが止まるまでのループ
        while (1)
        {
            UpdateflagButton();
            UpdateSlotMatrix();
            OverlaySlotRoles();
            LEDMatrixDisplay();
            // 全ての列が止まったか判定
            if (flagStopedSlotLine[0] + flagStopedSlotLine[1] + flagStopedSlotLine[2] == 3)
                break;
        }

        // スロットが止まった時間を取得
        unsigned long timeSlotStopped = GetMillis();
        // 小役揃い判定を行う
        JudgeMatchingRoles();
        // 揃っている場合の処理
        if (matchingRole != 4)
        {
            // 2秒間点滅表示を行うループ
            while ((GetMillis() - timeSlotStopped) < 2000)
            {
                OverlaySlotRoles();
                // 0.25秒ごとに点滅
                if ((GetMillis() % 500) > 250)
                    OverlayBlinkMask();
                // LEDマトリックスに表示
                LEDMatrixDisplay();
            }
            // Masterマイコンに結果を送信
            if (matchingRole == 0)
                SendResultSlot(2); // 大当たり
            else
                SendResultSlot(1); // 当たり
        }
        // 揃っていない場合の処理
        else
        {
            // 2秒間静止表示を行うループ
            while ((GetMillis() - timeSlotStopped) < 2000)
                LEDMatrixDisplay();
            SendResultSlot(0); // ハズレ
        }
    }
}
