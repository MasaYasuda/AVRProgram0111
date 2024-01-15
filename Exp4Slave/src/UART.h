#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>

// 関数のプロトタイプ宣言
void InitUART(unsigned long baudrate);   // UARTの初期化関数
int UARTCheck(void);                     // 受信データの有無を確認する関数
unsigned char UARTReceive(void);         // 受信データを取得する関数
void UARTTransmit(unsigned char TxData); // データを送信する関数

// グローバル変数
volatile unsigned char RxBuf[2] = {0};       // 受信データ格納バッファ
volatile unsigned char NumUnreadData = 0;    // 未読のデータ数
volatile unsigned char IndexRxInterrupt = 0; // 割り込み時のインデックス
unsigned char IndexRxReceive = 0;            // 受信時のインデックス

void InitUART(unsigned long baudrate)
{
    UBRR0 = 12;           // ボーレートの設定　(1, 000, 000 / 8 / 9600 - 1)  ≈ 12.02
    UCSR0A |= 0b00000010; // 倍速モードを許可(U2X=1)
    UCSR0B = 0b10011000;  // 受信完了割込みと送信完了割込みを有効にする
    UCSR0C = 0b00000110;  // フレームフォーマットの設定 パリティ無効 , 8ビットデータモード
    sei();                // 割込み有効化
}

int UARTCheck(void)
{
    return NumUnreadData; // 未読データ数を返す
}

// 受信時の割込みハンドラ
ISR(USART_RX_vect)
{
    if (NumUnreadData < 2)
    {
        RxBuf[IndexRxInterrupt] = UDR0; // 受信データをバッファに格納
        IndexRxInterrupt = 1 - IndexRxInterrupt;
        NumUnreadData++;
    }
    else
        UDR0; // データを読み捨て(∵読まないとフラグがリセットされないため)
}

unsigned char UARTReceive(void)
{
    unsigned char tmpData = RxBuf[IndexRxReceive]; // 受信バッファからデータを取り出す
    IndexRxReceive = 1 - IndexRxReceive;
    NumUnreadData--;
    return tmpData;
}

void UARTTransmit(unsigned char TxData)
{
    while (0 == ((UCSR0A >> 5) & 0b1))
    {
        // 送信バッファが空になるまで待機 = 送信予定データの上書きを防ぐ
    }
    UDR0 = TxData;        // データをデータレジスタに格納
    UCSR0A |= 0b01000000; // 送信を開始
    return;
}

#endif // UART_H_
