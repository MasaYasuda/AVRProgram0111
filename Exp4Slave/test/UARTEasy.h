#ifndef UART_EASY_H_
#define UART_EASY_H_

#include <avr/io.h>        // AVRの入出力を制御するためのヘッダファイル
#include <avr/interrupt.h> // AVRの割込みを制御するためのヘッダファイル

// プロトタイプ宣言
void InitUART(unsigned long baudrate);   // UARTの初期化
int UARTCheck(void);                     // 受信データの有無をチェックする関数
unsigned char UARTReceive(void);         // 受信データの取得
void UARTTransmit(unsigned char TxData); // データの送信

// 受信バッファと受信フラグ
static volatile unsigned char RxBuf;  // 受信バッファ
static volatile unsigned char flagRx; // 受信フラグ

void InitUART(unsigned long baudrate)
{
    // 通信方式設定
    UBRR0 = F_CPU / 8 / baudrate - 1; // ref:p128 ボーレートの設定　(1, 000, 000 / 8 / 9600 - 1)  ≈ 12.02
    UCSR0A |= 0b00000010;             // 倍速許可(U2X=1)
    UCSR0B = 0b10011000;              // 受信完了割込みと送信完了割込みを有効にする
    UCSR0C = 0b00000110;              // フレームフォーマットの設定 パリティ無効 , 8ビットデータモード
    sei();                            // 割込みを有効にする
}

int UARTCheck(void)
{
    return flagRx; // 受信フラグの状態を返す
}

unsigned char UARTReceive(void)
{
    flagRx = 0;   // 受信フラグをリセット
    return RxBuf; // 受信データを返す
}

// 受信時の割込み関数
ISR(USART_RX_vect)
{
    RxBuf = UDR0; // 受信データをバッファに格納
    flagRx = 1;   // 受信フラグをセット
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

#endif // UART_EASY_H_