#define F_CPU 1000000L
#include <avr/io.h>
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "ButtonsSlave.h"
#include "CommunicationSlave.h"
#include "SlotSlave.h"

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
	InitComSignal();
	mode = PLAYING; // 初期モードをプレイ中状態に設定

	while (1)
	{
		if (mode == PLAYING)
		{
			OverlayMatrix(offlight, 0, 0, 16, 16);
			while (1)
			{
				if (UARTCheck() > 0)
				{
					unsigned char RXdata = UARTReceive();
					if (OrderCheckSlot(RXdata))
					{
						mode = SLOT;
						break;
					}
				}
				LEDMatrixDisplay();
			}
		}
		if (mode == SLOT) // スロットモードの場合
		{
			Resetflag(); // フラグをリセット
			while (1)
			{
				UpdateflagButton();																// ボタンの状態を更新
				UpdateSlotMatrix();																// スロットマトリックスを更新
				OverlaySlotRoles();																// スロットの役をオーバーレイ
				LEDMatrixDisplay();																// LEDマトリックスに表示
				if (flagStopedSlotLine[0] + flagStopedSlotLine[1] + flagStopedSlotLine[2] == 3) // 全ての列が止まったか判定
					break;																		// 全ての列が止まっていたらループを抜ける
			}

			unsigned long timeSlotStopped = GetMillis(); // スロットが止まった時間を取得
			JudgeMatchingRoles();						 // 小役揃い判定を行う
			if (matchingRole != 4)						 // 揃っている場合
			{
				while ((GetMillis() - timeSlotStopped) < 2000) // 2秒間点滅表示を行う
				{
					OverlaySlotRoles();			   // 役をオーバーレイ
					if ((GetMillis() % 500) > 250) // 0.25秒ごとに点滅
						OverlayBlinkMask();		   // 点滅マスクをオーバーレイ
					LEDMatrixDisplay();			   // LEDマトリックスに表示
				}
				// Masterマイコンに結果を送信する処理をここに記述
				if (matchingRole == 0)
					SendResultSlot(2); // 大当たり
				else
					SendResultSlot(1); // 当たり
			}
			else
			{
				while ((GetMillis() - timeSlotStopped) < 2000) // 0.5秒静止表示を行う
					LEDMatrixDisplay();						   // LEDマトリックスに表示

				// Masterマイコンに結果を送信する処理をここに記述
				SendResultSlot(0); // ハズレ
			}

			mode = PLAYING; // モードをスロットモードに戻す
		}
	}
}