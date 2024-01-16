#define F_CPU 1000000L
#include "Timer.h"
#include "UART.h"
#include "LEDMatrix.h"
#include "CardScannerSlave.h"

int main()
{
	InitTimer(); // タイマーの初期化
	InitUART(9600);
	InitLEDMatrix(); // LEDマトリックスの初期化

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