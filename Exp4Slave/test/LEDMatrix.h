/**
 * @file LEDMatrix.h
 * @details 回路接続
 * ---------------------
 * 74HC595    -- AVR
 * SER(data)  -- PC0
 * RCK(latch) -- PC1
 * SCK(clock) -- PC2
 * ---------------------
 */

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <avr/io.h>

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

unsigned int zeroMatrix[5];
unsigned int oneMatrix[5];
unsigned int sevenMatrix[5];
unsigned int xMatrix[5];
unsigned int diamondMatrix[5];
unsigned int offlight[16];
unsigned int onlight[16];

unsigned int SMatrix[5];
unsigned int CMatrix[5];
unsigned int AMatrix[5];
unsigned int NMatrix[5];

unsigned int twoMatrix[5];
unsigned int threetrix[5];
unsigned int fourMatrix[5];
unsigned int fiveMatrix[5];

static unsigned int _pallet[16]; // 描画用マトリクス

// LEDマトリックスのセットアップを行う関数
void InitLEDMatrix();
// palletにobjを描く
void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m);
// LEDマトリックスにpalletを表示する
void LEDMatrixDisplay();
static void _SetBitsFromXtoY(unsigned int *y, unsigned int x, int p, unsigned char n);

// 16x16 LEDマトリックス用のビットパターン
unsigned int zeroMatrix[5] = {
    0b10001,
    0b10101,
    0b10101,
    0b10101,
    0b10001};

unsigned int oneMatrix[5] = {
    0b10011,
    0b11011,
    0b11011,
    0b11011,
    0b11011};
unsigned int sevenMatrix[5] = {
    0b10001,
    0b11101,
    0b11101,
    0b11101,
    0b11101};

unsigned int xMatrix[5] = {
    0b11111,
    0b10101,
    0b11011,
    0b10101,
    0b11111};

unsigned int diamondMatrix[5] = {
    0b11011,
    0b10001,
    0b00000,
    0b10001,
    0b11011};

unsigned int offlight[16] = {
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF};

unsigned int onlight[16] = {
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000};

unsigned int SMatrix[5] = {
    0b1000,
    0b0011,
    0b1001,
    0b1100,
    0b0001};
unsigned int CMatrix[5] = {
    0b1001,
    0b0010,
    0b0011,
    0b0010,
    0b1001};
unsigned int AMatrix[5] = {
    0b1001,
    0b0010,
    0b0010,
    0b0000,
    0b0010};
unsigned int NMatrix[5] = {
    0b0110,
    0b0010,
    0b0000,
    0b0100,
    0b0110};

unsigned int twoMatrix[5] = {
    0b10001,
    0b11101,
    0b10001,
    0b10111,
    0b10001};
unsigned int threetrix[5] = {
    0b10001,
    0b11101,
    0b10001,
    0b11101,
    0b10001};
unsigned int fourMatrix[5] = {
    0b10101,
    0b10101,
    0b10001,
    0b11101,
    0b11101};
unsigned int fiveMatrix[5] = {
    0b10001,
    0b10111,
    0b10001,
    0b11101,
    0b10001};

// 描画用マトリクス
static unsigned int _pallet[16] = {
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF};

// LEDマトリックスのセットアップを行う関数
void InitLEDMatrix()
{
    // SER(データ)=PC0 RCK(ラッチ)=PC1   SCK(クロック)=PC2
    DDRC |= 0b00000111; // PC0, PC1, PC2を出力に設定する
}

void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m)
{
    for (int i = q; i < m + q; i++)
    {
        if (i < 0)
            continue;
        if (i > 15)
            break;
        _SetBitsFromXtoY(&_pallet[i], obj[i - q], p, n);
    }
}

void LEDMatrixDisplay()
{
    for (int j = 0; j < 16; j++)
    {

        PORTC &= 0b11111101; // ラッチ(PC1)をLOWに設定

        unsigned int row = 0x8000 >> j;
        unsigned char colH = _pallet[j] >> 8;
        unsigned char colL = _pallet[j] & 0xFF;

        unsigned long data = ((unsigned long)row << 16) | ((unsigned long)colH << 8) | colL;

        for (int i = 0; i < 32; i++)
        {
            PORTC = (PORTC & 0b11111110) | (data & 0x01); // SER(PC0)にデータをセット
            data >>= 1;

            PORTC |= 0b00000100; // クロック(PC2)をHIGHに設定
            PORTC &= 0b11111011; // クロック(PC2)をLOWに設定
        }

        PORTC |= 0b00000010; // ラッチ(PC1)をHIGHに設定

        //_delay_ms(1); // 1ミリ秒待つ（上手く調節する）
    }
}

void _SetBitsFromXtoY(unsigned int *y, unsigned int x, int p, unsigned char n)
{
    if (p < 0)
    {
        // pが負の場合はyの0ビット目から(p+n-1)ビット目を使用
        n = n + p;
        if (n - 1 > 15)
            n = 16;                               // オーバーフローをチェックし、必要に応じてnを調整
        unsigned int mask = ((1 << n) - 1);       // nビットのマスクを作成し、aビット目にシフト
        *y = (*y & ~mask) | ((x >> (-p)) & mask); // yのa~(p+n-1)ビット目をクリアし、xの0~(n-1)ビット目をセット
    }
    else
    {
        if (p + n - 1 > 15)
            n = 16 - p;                          // オーバーフローをチェックし、必要に応じてnを調整
        unsigned int mask = ((1 << n) - 1) << p; // nビットのマスクを作成し、aビット目にシフト
        *y = (*y & ~mask) | ((x << p) & mask);   // yのa~(p+n-1)ビット目をクリアし、xの0~(n-1)ビット目をセット
    }
}
#endif // LEDMATRIX_H
