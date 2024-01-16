#ifndef ADCONVERSION_H
#define ADCONVERSION_H

#include <avr/io.h>

// プロトタイプ宣言
unsigned int AnalogInput(unsigned int pinnum); // アナログ値読みとり用関数

/**
 * |ADMUX|A/D多重器選択レジスタ
 * 0b0100xxxx
 *   ||||└┴┴┴ A/Dチャネル選択|ADCxの番号に応じて設定(例：ADC2(PC2)をチャネルとするなら2=0010)
 *   |||└──── 予約|
 *   ||└───── 左揃え選択|今回はしないので0
 *   └┴────── 基準電圧選択|AVCCを基準電圧とするので01
 * 参考資料:mega88.pdf (p.179)
 *
 * |ADCSRA|
 * 0b11xxxxxx
 *   │││││└┴┴ A/D変換クロック選択|デフォルト値で000->2分周
 *   ||||└─── A/D変換完了割り込み許可|
 *   |||└──── A/D変換完了割り込み要求フラグ|A/D変換が完了し、A/Dデータ レジスタが更新されると、1になる
 *   ||└───── A/D変換自動起動許可|デフォルト値で0
 *   |└────── A/D変換開始|開始するので1
 *   └─────── A/D許可|許可するので1
 * 参考資料:mega.pdf (p.179)
 *
 * |ADCL|A/D変換データ レジスタ下位…結果の上位9,8ビットが1,0ビット目に書き込まれる。
 * |ADCH|A/D変換データ レジスタ上位…結果の下位7~0ビットが書き込まれる。
 * 参考資料:mega88.pdf (p.181)
 */
unsigned int AnalogInput(unsigned int pinnum)
{
    ADMUX = 0b01000000 | pinnum; // AD変換ポート設定初期化(基準電圧はAVCC)
    ADCSRA |= 0b11000000;        // AD変換許可＆開始
    while (0 == ((ADCSRA >> 4) & 0b1))
    {
    }
    unsigned int result = ADCL;
    result |= ADCH << 8;
    return result;
}

#endif // ADCONVERSION_H