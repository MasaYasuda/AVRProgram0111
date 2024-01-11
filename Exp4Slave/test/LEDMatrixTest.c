#include <avr/io.h>
#include "LEDMatrix.h"

int main()
{
	// LEDマトリックスのセットアップを行う
	InitLEDMatrix();
	OverlayMatrix(onlight,0,0,16,16);
	while (1)
	{
		LEDMatrixDisplay();
	}
}


