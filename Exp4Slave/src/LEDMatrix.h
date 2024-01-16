/*
 * 回路接続
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

// プロトタイプ宣言

void InitLEDMatrix(); // LEDマトリックスのセットアップを行う関数
// yのpビット目からnビット分、xのビットをセットする関数
void _CopyRawToPallet(unsigned int *y, unsigned int x, int p, unsigned char n);
void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m); // palletにobjを描く
void LEDMatrixDisplay();                                            // LEDマトリックスにpalletを表示する

// 16x16 LEDマトリックス用のビットパターン群
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
unsigned int _pallet[16] = {
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

void InitLEDMatrix()
{
    DDRC |= 0b00000111; // PC0, PC1, PC2を出力に設定する
}

// *target(OverlayMatrix関数では_palletの要素)の(x)ビット座標pを始点として、コピー元(引数x)のビットをnだけコピーする。
void _CopyRawToPallet(unsigned int *target, unsigned int copySource, int p, unsigned char n)
{
    if (p < 0)
    {
        n = n + p;      // pが負の値であるため、nにpを加算して実際のコピーするビット数を計算する
        if (n - 1 > 15) // コピーするビット数が16ビットを超えないように調整
            n = 16;
        // nビット分のマスクを作成 (例:n=3...マスク=0b0000000000000111)
        unsigned int mask = ((1 << n) - 1);
        *target &= ~mask;                       // targetの最下位ビットからnビット分をクリア
        *target |= (copySource >> (-p)) & mask; // targetの0ビット目を始点としてcopySourceの(-p)ビット目からnビット分をセット
    }
    else
    {
        if (p + n - 1 > 15) // コピーするビット数が16ビットを超えないように調整
            n = 16 - p;
        // nビット分のマスクを作成し、pビット目にシフト (例:n=3,p=2...マスク=0b0000000000011100)
        unsigned int mask = ((1 << n) - 1) << p;
        *target &= ~mask;                    // targetのpビット目からnビット分をクリア
        *target |= (copySource << p) & mask; // targetのpビット目を始点としてcopySourceの最下位ビットからnビット分をセット
    }
}

void OverlayMatrix(unsigned int obj[], int p, int q, int n, int m)
{
    //_palletの座標(p,q)を起点として、objをn x m 部分だけ_palletにオーバーレイする
    for (int i = q; i < m + q; i++)
    {
        // y座標に関して描画範囲外のインデックスは無視する
        if (i < 0)
            continue;
        if (i > 15)
            break;
        // _palletのi番目にobjのビットパターンをセットする
        _CopyRawToPallet(&_pallet[i], obj[i - q], p, n);
    }
}

void LEDMatrixDisplay()
{
    for (int j = 0; j < 16; j++)
    {
        PORTC &= 0b11111101;                                                                 // ラッチ(PC1)をLOW
        unsigned int row = 0x8000 >> j;                                                      // アノード側の16ビットを生成(該当行だけ1)
        unsigned char colH = _pallet[j] >> 8;                                                // カソード側側の上位8ビットを取得
        unsigned char colL = _pallet[j] & 0xFF;                                              // カソード側側の下位8ビットを取得
        unsigned long data = ((unsigned long)row << 16) | ((unsigned long)colH << 8) | colL; // 行と列のデータを結合
        for (int i = 0; i < 32; i++)                                                         // 32回ループ
        {
            PORTC = (PORTC & 0b11111110) | (data & 0x01); // 最下位ビットをSER(PC0)に送信
            data >>= 1;                                   // データを1ビットシフト
            PORTC |= 0b00000100;                          // クロック(PC2)をHIGH
            PORTC &= 0b11111011;                          // クロック(PC2)をLOW
        }
        PORTC |= 0b00000010; // ラッチ(PC1)をHIGHにしてデータの更新
    }
}

#endif // LEDMATRIX_H
