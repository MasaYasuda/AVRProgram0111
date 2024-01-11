#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "ButtonsSlave.h"
#include "SlotSlave.h"
#include "CardScanSlave.h"

const char WAITING = 0; // 待機状態を表す定数
const char PLAYING = 1; // プレイ中状態を表す定数
const char SLOT = 2;	// スロットモードを表す定数
char mode = 0;			// 現在のモードを保持する変数

int main()
{
	InitTimer(); // タイマーの初期化
	InitUART(9600);
	InitButtons();	 // ボタンの初期化
	InitLEDMatrix(); // LEDマトリックスの初期化
	mode = PLAYING; // 初期モードをプレイ中状態に設定

	while (1)
	{
		if (mode == PLAYING) // スロットモードの場合
		{
			while (1)
			{
				if (UARTCheck() > 0)
				{
					unsigned char RXdata = UARTReceive();
					if (OrderCheckCardScanner(RXdata) == 1)
						EnableCardScanner();
					else if (OrderCheckCardScanner(RXdata) == 2)
						DisableCardScanner();
				}
				LEDMatrixDisplay();
			}
		}
	}
}