/*
 * Timer1を使用（CTCモード）
 * PB1に出力（変更可能）
 * ボタン PB6
 */

#ifndef SPEAKER_H_
#define SPEAKER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer.h"
#include "Conveyor.h"

// プロトタイプ宣言
void InitSpeaker();																  // スピーカーの初期設定を行う
void ResetMusic();																  // 音楽再生に関する変数を初期化する
void SoundOutput(unsigned int frequency);										  // 指定された周波数で音を出力する関数
void MakeWaitingSound();														  // 待機時の音を生成する
void MakePlayingSound();														  // 再生中の音を生成する
void MakeSlotSound();															  // スロット音を生成する
void SetSoundEffect(int length, unsigned int intervals[], unsigned int pitchs[]); // 効果音のパラメータを設定する
void EnableWeirdSound();														  // WeirdSound機能を有効にする
int CheckButtonWeirdSound();													  // WeirdSound用ボタンチェックする
void ChangePhaseWeirdSound();													  // WeirdSoundの状態を変更する
void DisableWeirdSound();														  // WeirdSound機能を無効にする

// 音階の周波数の定義
#define C4 262	// ド
#define D4 294	// レ
#define E4 330	// ミ
#define F4 349	// ファ
#define G4 392	// ソ
#define A4 440	// ラ
#define B4 494	// シ
#define C5 523	// ド（1オクターブ上）
#define D5 587	// レ（1オクターブ上）
#define E5 659	// ミ（1オクターブ上）
#define F5 698	// ファ（1オクターブ上）
#define G5 784	// ソ（1オクターブ上）
#define A5 880	// ラ（1オクターブ上）
#define B5 988	// シ（1オクターブ上）
#define C6 1046 // ド（2オクターブ上）

// WAITING MODEのデフォルト音楽
int IndexWaitingMusic = 0;										// 待機音楽の現在のインデックス
unsigned long previousTimeSwitchedWaitingMusic = 0;				// 前回Indexを変更した時間
int WAITING_MUSIC_INDEX_LENGTH = 4;								// デフォルト音楽の長さ
unsigned int WAITING_MUSIC_INTERVALS[4] = {500, 500, 500, 500}; // 間隔
unsigned int WAITING_MUSIC_PITCHS[4] = {F4, 0, C4, 0};			// 高さ

// SLOT MODEのデフォルト音楽
int IndexSlotMusic = 0;										 // スロット音楽の現在のインデックス
unsigned long previousTimeSwitchedSlotMusic = 0;			 // 前回Indexを変更した時間
int SLOT_MUSIC_INDEX_LENGTH = 4;							 // スロット音楽の長さ
unsigned int SLOT_MUSIC_INTERVALS[4] = {500, 500, 500, 500}; // 間隔
unsigned int SLOT_MUSIC_PITCHS[4] = {A4, 0, G4, 0};			 // 高さ

// PLAYING MODEのデフォルト音楽
int IndexPlayingMusic = 0;							// 再生音楽の現在のインデックス
unsigned long previousTimeSwitchedPlayingMusic = 0; // 前回Indexを変更した時間
int PLAYING_MUSIC_INDEX_LENGTH = 16;				// デフォルト音楽の長さ
unsigned int PLAYING_MUSIC_INTERVALS[16] =
	{400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400}; // 間隔
unsigned int PLAYING_MUSIC_PITCHS[16] =
	{C4, C4, G4, G4, A4, A4, G4, 0, F4, F4, E4, E4, D4, D4, C4, 0}; // きらきら星の音階

// 効果音関連
int flagEnableSoundEffect = 0;					   // 効果音を有効にするフラグ
int IndexSoundEffect = 0;						   // 効果音の現在のインデックス
unsigned long previousTimeSwitchedSoundEffect = 0; // 前回Indexを変更した時間
int soundEffectIndexLength = 0;					   // 効果音の長さ
unsigned int soundEffectIntervals[20] = {0};	   // 効果音の間隔
unsigned int soundEffectPitchs[20] = {0};		   // 効果音の高さ

int SEJammerMotorLength = 7;												  // JammerMotor警告効果音の長さ
unsigned int SEJammerMotorIntervals[7] = {250, 250, 250, 250, 250, 250, 250}; // JammerMotor警告効果音の間隔
unsigned int SEJammerMotorPitchs[7] = {B5, F5, B5, F5, B5, F5, 0};			  // JammerMotor警告効果音のピッチ

int SEJackpotLength = 7;												  // SlotJackpot効果音の長さ
unsigned int SEJackpotIntervals[7] = {400, 400, 300, 100, 200, 600, 600}; // SlotJackpot効果音の間隔
unsigned int SEJackpotPitchs[7] = {D4, B4, A4, C5, 0, B4, 0};			  // SlotJackpot効果音のピッチ

int SESuccessedLength = 4;									 // 成功時効果音の長さ
unsigned int SESuccessedIntervals[4] = {100, 100, 200, 100}; // 成功時効果音の間隔
unsigned int SESuccessedPitchs[4] = {G5, E4, C6, 0};		 // 成功時効果音の高さ

int SEFailedLength = 4;									  // 失敗時効果音の長さ
unsigned int SEFailedIntervals[4] = {100, 100, 200, 100}; // 失敗時効果音の間隔
unsigned int SEFailedPitchs[4] = {B4, A4, F4, 0};		  // 失敗時効果音の高さ

// WeirdSound関連
int flagEnableWeirdSound = 0;			// WeirdSoundを有効にするフラグ
unsigned long timeEnableWeirdSound = 0; // WeirdSoundを有効にした時間

/**
 * |TCCR1A| タイマ/カウンタ1制御レジスタA
 * 0b00000000
 *   ││││││└┴ 波形生成種別|比較一致タイマ/カウンタ解除(CTC)動作動作なので00
 *   ||||└┴── 予約|
 *   ||└┴──── 比較B出力選択|00で標準ポート動作(OC1B切断)
 *   └┴────── 比較A出力選択|00で標準ポート動作(OC1A切断)
 * 参考資料:mega88.pdf (p.96)
 *
 *  |TCCR1B| タイマ/カウンタ1制御レジスタB
 * 0b00001001
 *   │││││└┴┴ クロック選択|分周なしにするので001
 *   |||└┴─── 波形生成種別|比較一致タイマ/カウンタ解除(CTC)動作動作なので01
 *   ||└───── 予約|
 *   |└────── 捕獲起動入力端選択|特に許可しないのでデフォルト値で0のまま
 *   └─────── 捕獲起動入力1雑音消去許可|特に許可しないのでデフォルト値で0のまま
 * 参考資料:mega88.pdf (p.97)
 */
void InitSpeaker()
{
	DDRB |= 0b00000010;	 // PB1を出力設定にする
	TCCR1A = 0b00000000; // CTCモードを設定する
	TCCR1B = 0b00001001; // CTCモードと分周比(1)を設定する

	DDRB &= 0b10111111;	 // PB6(ボタン)を入力設定にする
	PORTB |= 0b01000000; // PB6(ボタン)をプルアップする
	ResetMusic();		 // スピーカーの設定をリセット
}

void ResetMusic()
{
	// 変数をリセットする(モード変化時に最初から再生させるため)
	IndexPlayingMusic = 0;
	previousTimeSwitchedPlayingMusic = 0;
	IndexWaitingMusic = 0;
	previousTimeSwitchedWaitingMusic = 0;
	IndexSlotMusic = 0;
	previousTimeSwitchedSlotMusic = 0;
}

// タイマ/カウンタ1比較A一致で発生する割込み 参考資料:mega88.pdf (p.46)
ISR(TIMER1_COMPA_vect)
{
	PORTB ^= 0b00000010; // 出力反転
}

/**
 * |OCR1A|タイマ/カウンタ1比較Aレジスタ（16ビット レジスタ）
 * 参考資料:mega88.pdf (p.85,99)
 *
 * |TIMSK1|タイマ/カウンタ1割り込み許可レジスタ
 * 0b000000x0
 *   │││││││└ タイマ/カウンタ1溢れ割り込み許可|使用しないので0
 *   ||||||└─ タイマ/カウンタ1比較A割り込み許可|必要に応じて1にする
 *   |||||└── タイマ/カウンタ1比較B割り込み許可|使用しないので0
 *   |||└┴─── 予約|
 *   ||└───── タイマ/カウンタ1捕獲割り込み許可|使用しないので0
 *   └┴────── 予約|
 * 参考資料:mega88.pdf (p.100)
 */
void SoundOutput(unsigned int frequency)
{
	if (frequency == 0) // 指定周波数が0のときタイマーの割込みを停止する
	{
		TIMSK1 = 0x00;		 // 割込みを停止する
		PORTB &= 0b11111101; // PB1の出力をLOWに設定する
	}
	else
	{
		OCR1A = (unsigned int)F_CPU / (frequency * 2) - 1; // 周波数から値を逆算
		TIMSK1 = 0b00000010;							   // 比較Aマッチ割り込み許可
		sei();											   // 割り込みを有効
	}
}

void MakeWaitingSound()
{
	int pitch = 0; // 再生する音の高さ

	if (flagEnableSoundEffect == 1) // 効果音を鳴らすとき
	{
		pitch = soundEffectPitchs[IndexSoundEffect];												// 効果音の高さを取得
		if (GetMillis() - previousTimeSwitchedSoundEffect > soundEffectIntervals[IndexSoundEffect]) // 次の効果音に移るタイミングか
		{
			IndexSoundEffect++;								// 効果音のインデックスを進める
			previousTimeSwitchedSoundEffect = GetMillis();	// 時間を更新
			pitch = soundEffectPitchs[IndexSoundEffect];	// 新しい効果音の高さを取得
			if (IndexSoundEffect >= soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常の待機音を再生
	{
		pitch = WAITING_MUSIC_PITCHS[IndexWaitingMusic];												 // 待機音の高さを取得
		if (GetMillis() - previousTimeSwitchedWaitingMusic > WAITING_MUSIC_INTERVALS[IndexWaitingMusic]) // 次の音に移るタイミングか
		{
			IndexWaitingMusic = (IndexWaitingMusic + 1) % WAITING_MUSIC_INDEX_LENGTH; // 待機音のインデックスを進める
			previousTimeSwitchedWaitingMusic = GetMillis();							  // 時間を更新
			pitch = WAITING_MUSIC_PITCHS[IndexWaitingMusic];						  // 新しい待機音の高さを取得
		}
	}
	SoundOutput(pitch); // 音を出力
}

void MakePlayingSound()
{
	int pitch = 0; // 再生する音の高さ

	if (flagEnableSoundEffect == 1) // 効果音を鳴らすとき
	{
		pitch = soundEffectPitchs[IndexSoundEffect];												// 効果音の高さを取得
		if (GetMillis() - previousTimeSwitchedSoundEffect > soundEffectIntervals[IndexSoundEffect]) // 次の効果音に移るタイミングか
		{
			IndexSoundEffect++;								// 効果音のインデックスを進める
			previousTimeSwitchedSoundEffect = GetMillis();	// 時間を更新
			pitch = soundEffectPitchs[IndexSoundEffect];	// 新しい効果音の高さを取得
			if (IndexSoundEffect >= soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常の再生音を再生
	{
		pitch = PLAYING_MUSIC_PITCHS[IndexPlayingMusic];												 // 再生音の高さを取得
		if (GetMillis() - previousTimeSwitchedPlayingMusic > PLAYING_MUSIC_INTERVALS[IndexPlayingMusic]) // 次の音に移るタイミングか
		{
			IndexPlayingMusic = (IndexPlayingMusic + 1) % PLAYING_MUSIC_INDEX_LENGTH; // 再生音のインデックスを進める
			previousTimeSwitchedPlayingMusic = GetMillis();							  // 時間を更新
			pitch = PLAYING_MUSIC_PITCHS[IndexPlayingMusic];						  // 新しい再生音の高さを取得
		}

		if (flagEnableWeirdSound == 1) // WeirdSoundが有効なとき
			if (pitch != 0)			   // pitchが0、すなわち元の音が"無音"ではないとき
			{
				// WeirdSoundのピッチを変更 (C4の音程を基準に、高くなるほど元の音より低くなる)
				pitch = (unsigned int)(((float)pitch * 0.62) + 100);
			}
	}
	SoundOutput(pitch); // 音を出力
}

void MakeSlotSound()
{
	int pitch = 0; // 再生する音の高さ

	if (flagEnableSoundEffect == 1) // 効果音を鳴らすとき
	{
		pitch = soundEffectPitchs[IndexSoundEffect];												// 効果音の高さを取得
		if (GetMillis() - previousTimeSwitchedSoundEffect > soundEffectIntervals[IndexSoundEffect]) // 次の効果音に移るタイミングか
		{
			IndexSoundEffect++;								// 効果音のインデックスを進める
			previousTimeSwitchedSoundEffect = GetMillis();	// 時間を更新
			pitch = soundEffectPitchs[IndexSoundEffect];	// 新しい効果音の高さを取得
			if (IndexSoundEffect >= soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常のスロット音を再生
	{
		pitch = SLOT_MUSIC_PITCHS[IndexSlotMusic];												// スロット音の高さを取得
		if (GetMillis() - previousTimeSwitchedSlotMusic > SLOT_MUSIC_INTERVALS[IndexSlotMusic]) // 次の音に移るタイミングか
		{
			IndexSlotMusic = (IndexSlotMusic + 1) % SLOT_MUSIC_INDEX_LENGTH; // スロット音のインデックスを進める
			previousTimeSwitchedSlotMusic = GetMillis();					 // 時間を更新
			pitch = SLOT_MUSIC_PITCHS[IndexSlotMusic];						 // 新しいスロット音の高さを取得
		}
	}
	SoundOutput(pitch); // 音を出力
}

void SetSoundEffect(int length, unsigned int intervals[], unsigned int pitchs[])
{
	IndexSoundEffect = 0;							 // 効果音のインデックスをリセット
	flagEnableSoundEffect = 1;						 // 効果音を有効にする
	soundEffectIndexLength = length;				 // 効果音の長さを設定
	for (int i = 0; i < soundEffectIndexLength; i++) // 効果音の配列を設定
	{
		soundEffectIntervals[i] = intervals[i]; // 効果音の間隔を設定
		soundEffectPitchs[i] = pitchs[i];		// 効果音の高さを設定
	}
	previousTimeSwitchedSoundEffect = GetMillis(); // 時間を更新
}

// WeiredSound部分

void EnableWeirdSound()
{
	if (flagEnableWeirdSound == 1) // 既にWeirdSoundが有効なら何もしない
		return;
	flagEnableWeirdSound = 1;			// WeirdSoundを有効にする
	timeEnableWeirdSound = GetMillis(); // WeirdSoundを有効にした時間を記録
}

int CheckButtonWeirdSound()
{
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return 0;
	if (((PINB >> 6) & 0b1) == 0) // ボタンが押されていたら1を返す
	{
		SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs);
		return 1;
	}

	return 0; // それ以外は0を返す
}

void ChangePhaseWeirdSound()
{
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return;
	if (GetMillis() - timeEnableWeirdSound > 5000) // 失敗
	{
		TmpDecelerateConveyor();										   // コンベアを減速
		SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 効果音を流す
		DisableWeirdSound();											   // WeirdSoundを無効にする
	}
}

void DisableWeirdSound()
{
	flagEnableWeirdSound = 0; // WeirdSoundを無効にする
}

#endif // SPEAKER_H_
