#ifndef UART_H_
#define UART_H_

// バッファの長さの定義
#define TX_BUF_LEN 16 // 送信バッファの長さ
#define RX_BUF_LEN 16 // 受信バッファの長さ

#include <avr/io.h>        // AVRの入出力を制御するためのヘッダファイル
#include <avr/interrupt.h> // AVRの割込みを制御するためのヘッダファイル

// 受信バッファと送信バッファのインデックス
volatile unsigned int RxBufHead; // 受信バッファの中の一番古いデータの要素番号
volatile unsigned int RxBufTail; // 受信バッファの中の一番新しいデータの要素番号
volatile unsigned int TxBufHead; // 送信バッファの中の一番古いデータの要素番号
volatile unsigned int TxBufTail; // 送信バッファの中の一番新しいデータの要素番号

// 受信バッファと送信バッファ
volatile unsigned char RxBuf[RX_BUF_LEN];
volatile unsigned char TxBuf[TX_BUF_LEN];

/**
 * @brief UARTの設定を行う関数
 * @param baudrate ボーレート。基本は9600を代入
 */
void InitUART(unsigned long baudrate)
{
    // Try u2x mode first
    uint16_t baudrateSetting = (F_CPU / 4 / baudrate - 1) / 2; // ボーレートの設定
    UCSR0A = 0b00000011;                                       // MPCMは0でも良い。

    UBRR0H = baudrateSetting >> 8; // ボーレートレジスタの上位バイト
    UBRR0L = baudrateSetting;      // ボーレートレジスタの下位バイト

    UCSR0C = 0b00000110; // フレームフォーマットの設定

    UCSR0B |= 0b10011000; // 受信完了割込みと送信完了割込みを有効にする
    UCSR0B &= 0b11011111; // 9ビットデータモードを無効にする

    sei(); // 割込みを有効にする
}

/**
 * @brief 受信バッファにデータがあるかを確認する関数
 * @return int バッファにあるバイト数
 */
int UARTCheck(void)
{
    return ((unsigned int)(RX_BUF_LEN + RxBufHead - RxBufTail)) % RX_BUF_LEN;
}

/**
 * @brief 受信バッファからデータを取得する関数
 * @return unsigned char
 * @note 必ずUARTCheckで受信バッファが空でないことを確認してから実行
 */
unsigned char UARTReceive(void)
{
    unsigned char c = RxBuf[RxBufTail];                // データを取得
    RxBufTail = (unsigned char)(RxBufTail + 1) % RX_BUF_LEN; // インデックスを更新
    return c;                                          // データを返す
}

// 新たにUART受信した時の割込み関数
ISR(USART_RX_vect)
{
    if (0 == ((UCSR0A >> 2) & 0x01)) // フレームエラーがない場合
    {
        unsigned char c = UDR0;                                      // データレジスタからデータを取得
        unsigned int i = (unsigned int)(RxBufHead + 1) % RX_BUF_LEN; // 次のインデックスを計算

        if (RxBufTail != i) // バッファが満杯でない場合
        {
            RxBuf[RxBufHead] = c; // データをバッファに格納
            RxBufHead = i;        // インデックスを更新
        }
    }
    else
    {
        UDR0; // フレームエラーがある場合、データを読み捨てる
    };
}

/**
 * @brief データを送信バッファに格納する関数
 * @param c 送信する1バイトデータ
 */
void UARTTransmit(unsigned char c)
{
    if (TxBufHead == TxBufTail && 1 == ((UCSR0A >> 5) & 0x01)) // バッファが空で、送信可能な場合
    {
        /**
         * 以下の4行に関して。データをデータレジスタに格納(UDR0 = c;)し、
         * 送信を開始(UCSR0A |= 0b01000011;)するこの2つの操作は
         * 、一連の操作としてアトミック（不可分）に行いたいため、割込みが発生しないようにしています。
         */
        cli();                // 割込みを無効にする
        UDR0 = c;             // データをデータレジスタに格納
        UCSR0A |= 0b01000000; // 送信を開始
        sei();                // 割込みを有効にする
    }
    else
    {
        unsigned char i = (TxBufHead + 1) % TX_BUF_LEN; // 次のインデックスを計算
        TxBuf[TxBufHead] = c;                     // データをバッファに格納

        cli();                // 割込みを無効にする
        TxBufHead = i;        // インデックスを更新
        UCSR0B |= 0b00100000; // 送信完了割込みを有効にする
        sei();                // 割込みを有効にする
    }

    return; // 送信成功
}

// 送信バッファが空いたときに実行する割込み関数
ISR(USART_UDRE_vect)
{
    unsigned char c = TxBuf[TxBufTail];       // バッファからデータを取得
    TxBufTail = (TxBufTail + 1) % TX_BUF_LEN; // インデックスを更新

    UDR0 = c;             // データをデータレジスタに格納
    UCSR0A |= 0b01000000; // 送信を開始

    if (TxBufHead == TxBufTail) // バッファが空になった場合
    {
        UCSR0B &= 0b11011111; // 送信完了割込みを無効にする
    }
}

#endif // UART_H_