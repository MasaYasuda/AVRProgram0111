#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <avr/io.h>

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

/**
 * |register name|
 * 0b11111111
 *   │││││││└ |
 *   ||||||└─ |
 *   |||||└── |
 *   ||||└─── |
 *   |||└──── |
 *   ||└───── |
 *   |└────── |
 *   └─────── |
 * 参考資料:mega88.pdf (p.)
 */
// ┴