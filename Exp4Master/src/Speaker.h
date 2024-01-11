/**
 * @details Timer1を使用（CTCモード）
 * PB1に出力　(一応変更可)
 * ボタン PB6
 */

#ifndef SPEAKER_H_
#define SPEAKER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer.h"
#include "Conveyor.h"

/*********************************************************************
 * プロトタイプ宣言
 *********************************************************************/
void SoundOutput(unsigned int frequency);										  // 周波数に応じた音を出力
void ResetMusic();																  // 音楽をリセット
void InitSpeaker();																  // スピーカーを初期化
void MakeWaitingSound();														  // 待機音を生成
void MakePlayingSound();														  // 再生音を生成
void SetSoundEffect(int length, unsigned int intervals[], unsigned int pitchs[]); // 効果音を設定
void EnableWeirdSound();														  // WeirdSoundを有効化
int CheckButtonWeirdSound();													  // WeirdSound用ボタンのチェック
void ChangePhaseWeirdSound();													  // WeirdSoundのフェーズ変更
void DisableWeirdSound();														  // WeirdSoundを無効化

// 音階の定義
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
int WAITING_MUSIC_INDEX_LENGTH = 4;								// モード選択中のデフォルト音楽の長さ
unsigned int wAITING_MUSIC_INTERVALS[4] = {500, 500, 500, 500}; // 音の間隔
unsigned int WAITING_MUSIC_PITCHS[4] = {F4, 0, C4, 0};			// 音の高さ

// PLAYING MODEのデフォルト音楽
int IndexPlayingMusic = 0;																									 // 再生音楽の現在のインデックス
unsigned long previousTimeSwitchedPlayingMusic = 0;																			 // 前回Indexを変更した時間
int PLAYING_MUSIC_INDEX_LENGTH = 16;																						 // プレイ中のデフォルト音楽の長さ
unsigned int PLAYING_MUSIC_INTERVALS[16] = {400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400}; // 音の間隔
unsigned int PLAYING_MUSIC_PITCHS[16] = {C4, C4, G4, G4, A4, A4, G4, 0, F4, F4, E4, E4, D4, D4, C4, 0};						 // きらきら星の音階

// 効果音関連
int flagEnableSoundEffect = 0;					   // 効果音を有効にするフラグ
int IndexSoundEffect = 0;						   // 効果音の現在のインデックス
unsigned long previousTimeSwitchedSoundEffect = 0; // 前回Indexを変更した時間
int soundEffectIndexLength = 0;					   // 効果音の長さ
unsigned int soundEffectIntervals[20] = {0};	   // 効果音の間隔
unsigned int soundEffectPitchs[20] = {0};		   // 効果音の高さ

// JammerMotor警告効果音
int SEJammerMotorLength = 7;												  // JammerMotor警告効果音の長さ
unsigned int SEJammerMotorIntervals[7] = {250, 250, 250, 250, 250, 250, 250}; // JammerMotor警告効果音の間隔
unsigned int SEJammerMotorPitchs[7] = {B5, F5, B5, F5, B5, F5, 0};			  // JammerMotor警告効果音のピッチ

// SlotJackpot効果音
int SEJackpotLength = 7;												  // SlotJackpot効果音の長さ
unsigned int SEJackpotIntervals[7] = {400, 400, 300, 100, 200, 600, 600}; // SlotJackpot効果音の間隔
unsigned int SEJackpotPitchs[7] = {D4, B4, A4, C5, 0, B4, 0};			  // SlotJackpot効果音のピッチ

// 成功時効果音
int SESuccessedLength = 4;									 // 成功時効果音の長さ
unsigned int SESuccessedIntervals[4] = {100, 100, 200, 100}; // 成功時効果音の間隔
unsigned int SESuccessedPitchs[4] = {G5, E4, C6, 0};		 // 成功時効果音の高さ

// 失敗時効果音
int SEFailedLength = 4;									  // 失敗時効果音の長さ
unsigned int SEFailedIntervals[4] = {100, 100, 200, 100}; // 失敗時効果音の間隔
unsigned int SEFailedPitchs[4] = {B4, A4, F4, 0};		  // 失敗時効果音の高さ

// WeirdSound関連
int flagEnableWeirdSound = 0;			  // WeirdSoundを有効にするフラグ
int timeEnableWeirdSound = 0;			  // WeirdSoundを有効にした時間
unsigned long limitTimeWeirdSound = 5000; // WeirdSoundの制限時間

/**
 * @brief 音を出力する関数
 * @param frequency 目標の周波数（約7.63Hzから250kHzまで）
 */
// 音を出力する関数を定義します
void SoundOutput(unsigned int frequency)
{
	if (frequency == 0) // 指定周波数が0のときタイマーの割込みを停止する
	{
		TIMSK1 = 0x00;		 // 割込みを停止する
		PORTB &= 0b11111101; // PB1の出力をLOWに設定する
	}
	else
	{
		// 周波数を2倍にします
		unsigned int tmp = frequency * 2;
		// OCR1AにCPUクロック周波数を周波数の2倍で割った値から1を引いた値を設定します。
		// これにより、目標の周波数に対応するビットが設定されます。
		OCR1A = F_CPU / tmp - 1;
		// タイマー1の比較Aマッチ割り込みを許可します
		TIMSK1 = 0b00000010;
		// 割り込みを有効にします
		sei();
	}
}

// タイマー割込みで実行される関数
ISR(TIMER1_COMPA_vect)
{
	PINB ^= 0b00000010; // PB1の出力を反転する
}

void ResetMusic()
{
	IndexPlayingMusic = 0;				  // 再生音楽のインデックスをリセット
	previousTimeSwitchedPlayingMusic = 0; // 再生音楽の時間をリセット
	IndexWaitingMusic = 0;				  // 待機音楽のインデックスをリセット
	previousTimeSwitchedWaitingMusic = 0; // 待機音楽の時間をリセット
}

// 初期化関数
void InitSpeaker()
{
	// クロック校正
	OSCCAL = 0b10000000; // クロック校正値を設定

	DDRB |= 0b00000010;	 // PB1を出力設定にする
	TCCR1A = 0b00000000; // CTCモードを設定する
	TCCR1B = 0b00001001; // CTCモードと分周比(1)を設定する

	DDRB &= 0b10111111;	 // PB6(ボタン)を入力設定にする
	PORTB |= 0b01000000; // PB6(ボタン)をプルアップする
	ResetMusic();		 // スピーカーの設定をリセット
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
			if (IndexSoundEffect == soundEffectIndexLength) // 効果音が最後まで再生されたか
			{
				pitch = 0;				   // 音を停止
				flagEnableSoundEffect = 0; // 効果音を無効にする
			}
		}
	}
	else // 通常の待機音を再生
	{
		pitch = WAITING_MUSIC_PITCHS[IndexWaitingMusic];												 // 待機音の高さを取得
		if (GetMillis() - previousTimeSwitchedWaitingMusic > wAITING_MUSIC_INTERVALS[IndexWaitingMusic]) // 次の音に移るタイミングか
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
			if (IndexSoundEffect == soundEffectIndexLength) // 効果音が最後まで再生されたか
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

		if (flagEnableWeirdSound == 1)								 // WeirdSoundが有効なとき
			if (pitch != 0)											 // pitchが0、すなわち元の音が"無音"ではないとき
				pitch = (unsigned int)(((float)pitch * 0.62) + 100); // WeirdSoundのピッチを変更
	}
	SoundOutput(pitch); // 音を出力
}

// SoundEffect部分
// 効果音を設定
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
//  開始
void EnableWeirdSound()
{
	if (flagEnableWeirdSound == 1) // 既にWeirdSoundが有効なら何もしない
		return;
	flagEnableWeirdSound = 1;			// WeirdSoundを有効にする
	timeEnableWeirdSound = GetMillis(); // WeirdSoundを有効にした時間を記録
}

// ボタンの状態確認
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
	if (flagEnableWeirdSound == 0) // WeirdSoundが無効なら0を返す
		return;
	flagEnableWeirdSound = 0; // WeirdSoundを無効にする
}

#endif // SPEAKER_H_
