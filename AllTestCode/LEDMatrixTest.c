// LEDMatrixTest.c
#define F_CPU 1000000L
#include "LEDMatrix.h"

int main()
{

	InitLEDMatrix(); // LEDマトリックスの初期化
	// 市松模様
	OverlayMatrix(onlight, 0, 0, 16, 16);
	OverlayMatrix(offlight, -8, -8, 16, 16);
	OverlayMatrix(offlight, 8, 8, 16, 16);
	while (1)
	{
		LEDMatrixDisplay();
	}
}
