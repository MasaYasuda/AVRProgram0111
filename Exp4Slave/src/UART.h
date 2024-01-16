// UART.h
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

/**
 * |UBRR0|USARTnボーレート レジスタ
 * ボーレートが9600(∵この値は通信速度と誤差率を考慮)、倍速モードのとき
 * 計算式より(F_CPU / 8 / baudrate - 1)=12に設定する
 * 参考資料:mega88.pdf (p.138,143) 試しながら学ぶavr入門 (p.90)
 *
 * |UCSR0A|USART0制御/状態レジスタA
 * 0bxxxxxx1x
 *   │││││││└ 倍速許可|倍速許可するので1（非同期動作でだけ有効）
 *   ||||||└─ パリティ誤りフラグ|
 *   |||||└── データ オーバーラン発生フラグ|
 *   ||||└─── フレーミング異常フラグ|
 *   |||└──── USART送信データ レジスタ空きフラグ|
 *   ||└───── USART送信データ レジスタ空きフラグ|
 *   |└────── USART送信完了フラグ|UDR0に新規データが現存しない時に1
 *   └─────── USART受信完了フラグ|未読データがある時に1
 * 参考資料:mega88.pdf (p.140)
 *
 * |UCSR0B|USART0制御/状態レジスタB
 * 0b10011000
 *   │││││││└ 送信データ ビット8|今回は8ビットの通信なので関係なし
 *   ||||||└─ 受信データ ビット8|今回は8ビットの通信なので関係なし
 *   |||||└── データ ビット長選択2| 8ビットなので0
 *   ||||└─── 送信許可|許可するので1
 *   |||└──── 受信許可|許可するので1
 *   ||└───── 送信データ レジスタ空き割り込み許可|許可しないので0
 *   |└────── 送信完了割り込み許可|許可しないので0
 *   └─────── 受信完了割り込み許可|許可するので1
 * 参考資料:mega88.pdf (p.141)
 *
 * |UCSR0C|
 * 0b00000110
 *   │││││││└ クロック極性選択|非同期動作を使用するので0
 *   ||||||└─ データ順選択|8ビットなので0
 *   |||||└── データビット長選択|8ビットなので0
 *   ||||└─── 停止ビット選択|1ビットなので0
 *   ||└┴──── パリティ選択|使用しないので00
 *   └┴────── USART動作選択|非同期動作(UART通信)なので00
 * 参考資料:mega88.pdf (p.142)
 */
void InitUART(unsigned long baudrate)
{
    UBRR0 = unsigned int(F_CPU / 8 / baudrate - 1); // ボーレートの設定
    UCSR0A |= 0b00000010;                           // 倍速モードを許可(U2X=1)
    UCSR0B = 0b10011000;                            // 受信完了割込みと送信完了割込みを有効にする
    UCSR0C = 0b00000110;                            // フレームフォーマットの設定 パリティ無効 , 8ビットデータモード
    sei();                                          // 割込み有効化
}

int UARTCheck(void)
{
    return NumUnreadData; // 未読データ数を返す
}

/**
 * USART_RX_vec｜受信完了時の割込みベクタ  参考資料:mega88.pdf (p.46)
 * |UDR0|USART0データレジスタ
 * …USART送受信データ|送信データ レジスタ空きフラグが1の時に、ここに書き込まれると送信。
 *                   また、受信データもここに入る。
 * 参考資料:mega88.pdf (p.140)
 */
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
    UDR0 = TxData; // データをデータレジスタに格納
    return;
}

#endif // UART_H_
