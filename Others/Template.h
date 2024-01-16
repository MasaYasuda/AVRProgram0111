// Template.h
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <avr/io.h>

int flagEnableXX = 0;
unsigned long timeXX = 0;

// 初期化関数
void Init();
// 開始(有効化)
void Enable();
// 動作の更新
void ChangePhase();
// ボタンの状態確認
void CheckButton();
// 終了(無効化)
void Disable();

#endif // TEMPLATE_H
