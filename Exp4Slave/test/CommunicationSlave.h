#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <avr/io.h>

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/

/**
 * COMSignal  PB0
 * ...プルアップ
 * 確認時...LOWになってなければLOWを出力。送信終了したらプルアップに戻す
 * 
 */

void InitComSignal(){
    DDRB &= 0b11111110;  // 入力モード
    PORTB |= 0b00000001; // プルアップ
}
void WaitComSignal(){
    while((PINB&0b1)==0){
    }
    DDRB |= 0b00000001; // 出力モード
    PORTB &= 0b11111110;  // LOWを出力
}

#endif // COMMUNICATION_H