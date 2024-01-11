#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "Conveyor.h"
#include "FallSensor.h"
#include "SlotMaster.h"

const char WAITING = 0; // 待機状態を表す定数
const char PLAYING = 1; // プレイ中状態を表す定数
const char SLOT = 2;    // スロットモードを表す定数
char mode = 0;          // 現在のモードを保持する変数

int main()
{
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    mode = PLAYING; // 初期モードをプレイ中状態に設定
    while (1)
    {
        if (mode == PLAYING) // 現在のモードがプレイ中の場合
        {
            EnableConveyor(); // コンベアを有効にする
            while (1)
            {
                OutputConveyor(255); // コンベアの出力を最大にする
                if (CheckFallSensor()) // 落下センサーが反応したかチェック
                {
                    mode = SLOT; // モードをスロットモードに変更
                    break; // 内側のwhileループを抜ける
                }
            }
        }
        if (mode == SLOT) // 現在のモードがスロットモードの場合
        {
            DisableConveyor(); // コンベアを無効にする
            EnableSlot(); // スロットを有効にする
            while (1)
            {
                if (UARTCheck() > 0) // UARTにデータがあるかチェック
                {
                    unsigned char RXdata = UARTReceive(); // UARTからデータを受信
                    if (ResultCheckSlot(RXdata))          // スロットの結果をチェック
                    {
                        DisableSlot(); // スロットを無効にする
                        mode = PLAYING; // モードをプレイ中状態に戻す
                        break; // 内側のwhileループを抜ける
                    }
                }
            }
        }
    }
}