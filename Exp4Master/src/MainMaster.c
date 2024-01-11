#define F_CPU 1000000L
#include <stdlib.h>
#include "Timer.h"
#include "UART.h"
#include "Speaker.h"
#include "Conveyor.h"
#include "FallSensor.h"
#include "SlotMaster.h"
#include "JammerMotor.h"
#include "Countdown.h"
#include "CardScanner.h"
#include "LightVariesLED.h"
#include "WifiSwitch.h"

const char WAITING = 0; // 待機状態を表す定数
const char PLAYING = 1; // プレイ中状態を表す定数
const char SLOT = 2;    // スロットモードを表す定数
char mode = 0;          // 現在のモードを保持する変数

unsigned long timeStartPlaying = 0; // プレイ開始時間を保持する変数

int flagRX = 0;           // 受信フラグ、0で初期化
unsigned char RXdata = 0; // 受信データ、0で初期化

int main()
{
    OSCCAL=0b01101010;
    InitTimer();
    InitUART(9600);
    InitSpeaker();
    InitConveyor();
    InitFallSensor();
    InitJammerMotor();
    InitCardScanner();
    InitLightVariesLED();

    mode = WAITING; // 初期モードをWAITING状態に設定

    while (1)
    {
        if (mode == WAITING) // 現在のモードがWAITINGの場合
        {
            while (1)
            {
                MakeWaitingSound();
                if (CheckButtonConveyor()) // ボタンが押されたら開始準備
                {
                    timeStartPlaying = GetMillis();        // 現在の時間を取得し、プレイ開始時間として設定
                    srand((unsigned int)timeStartPlaying); // プレイ開始時間を元に乱数生成器を初期化
                    mode = SLOT;                           // モードをSLOTに変更
                    break;                                 // ループを抜ける
                }
            }
        }

        if (mode == PLAYING) // 現在のモードがプレイ中の場合
        {
            ResetMusic();     // 音楽をリセット
            EnableConveyor(); // コンベアを有効にする
            while (1)
            {
                MakePlayingSound(); // プレイ中の音を作成

                // データ受信
                flagRX = 0;          // 受信フラグをリセット
                if (UARTCheck() > 0) // UARTにデータがあるかチェック
                {
                    flagRX = 1;             // 受信フラグをセット
                    RXdata = UARTReceive(); // UARTからデータを受信
                }

                // イベントの生成
                int randEventNum = rand() % 1000000; // ランダムなイベント番号を生成
                if (randEventNum < 10)
                    EnableWeirdSound(); // WeirdSoundを有効にする
                else if (randEventNum < 20)
                    EnableJammerMotor(); // ジャマモーターを有効にする
                else if (randEventNum < 30)
                    EnableCountdown(); // カウントダウンを有効にする
                else if (randEventNum < 40)
                    EnableCardScanner(); // カードスキャナーを有効にする
                else if (randEventNum < 50)
                    EnableLightVariesLED(); // LEDの光を有効にする

                //  イベント解除の確認
                if (CheckButtonWeirdSound())                  // WeirdSoundのボタンをチェック
                    DisableWeirdSound();                      // WeirdSoundを無効にする
                if (CheckButtonJammerMotor())                 // ジャマモーターのボタンをチェック
                    DisableJammerMotor();                     // ジャマモーターを無効にする
                if (CheckCardScanner())                       // カードスキャナーをチェック
                    DisableCardScanner();                     // カードスキャナーを無効にする
                if (CheckVolumeLED())                         // LEDのボリュームをチェック
                    DisableLightVariesLED();                  // LEDの点灯を停止する
                if (ResultCheckCountdown(RXdata) && (flagRX)) // カウントダウンの結果と受信フラグをチェック
                    DisableCountdown();                       // カウントダウンを無効にする

                // イベントの状態遷移
                ChangePhaseWeirdSound();     // WeirdSoundのフェーズを変更
                ChangePhaseJammerMotor();    // ジャマモーターのフェーズを変更
                ChangePhaseCardScanner();    // カードスキャナーのフェーズを変更
                ChangePhaseLightVariesLED(); // LEDの光のフェーズを変更

                // コンベア上昇
                if (CheckButtonConveyor())
                    OutputConveyor(dutyRateConveyer);
                else
                    OutputConveyor(0);

                // 落下センサーが反応したかチェック
                if (CheckFallSensor()) // 落下センサーをチェック
                {
                    OutputConveyor(0);
                    DisableWeirdSound();     // WeirdSoundを無効にする
                    DisableJammerMotor();    // ジャマモーターを無効にする
                    DisableCountdown();      // カウントダウンを無効にする
                    DisableCardScanner();    // カードスキャナーを無効にする
                    DisableLightVariesLED(); // LEDの光を無効にする
                    DisableConveyor();       // コンベアを無効にする

                    SetSoundEffect(SESuccessedLength, SESuccessedIntervals, SESuccessedPitchs); // 成功音を設定
                    mode = SLOT;                                                                // モードをスロットモードに変更
                    break;
                }

                // プレイの制限時間を超過した場合
                if (GetMillis() - timeStartPlaying > 120000) // プレイ開始から120秒以上経過したかチェック
                {
                    OutputConveyor(0);
                    DisableWeirdSound();     // WeirdSoundを無効にする
                    DisableJammerMotor();    // ジャマモーターを無効にする
                    DisableCountdown();      // カウントダウンを無効にする
                    DisableCardScanner();    // カードスキャナーを無効にする
                    DisableLightVariesLED(); // LEDの光を無効にする

                    SetSoundEffect(SEFailedLength, SEFailedIntervals, SEFailedPitchs); // 失敗音を設定
                    while (1)
                    {
                        OutputConveyor(dutyRateConveyer); // コンベアの出力を設定
                        if (CheckFallSensor())            // 落下センサーをチェック
                            break;
                        MakeWaitingSound(); // 待機音を作成
                    }
                    DisableConveyor(); // コンベアを無効にする
                    mode = WAITING;    // モードを待機モードに戻る
                    break;
                }
            }
        }

        if (mode == SLOT) // 現在のモードがスロットモードの場合
        {
            ResetMusic();
            EnableSlot(); // スロットを有効にする
            while (1)
            {
                MakeWaitingSound();
                if (UARTCheck() > 0) // UARTにデータがあるかチェック
                {
                    unsigned char RXdata = UARTReceive(); // UARTからデータを受信
                    if (ResultCheckSlot(RXdata))          // スロットの結果をチェック
                    {
                        DisableSlot();  // スロットを無効にする
                        mode = PLAYING; // モードをプレイ中状態に戻す
                        break;          // 内側のwhileループを抜ける
                    }
                }
            }
        }
    }
}