// JammerMotorTest.c
#define F_CPU 1000000L
#include "Timer.h"
#include "JammerMotor.h"

int main()
{
    InitTimer();
    InitSpeaker();
    InitJammerMotor();
    EnableJammerMotor(); // ジャマーモーターを有効にする
    while (1)
    {
        MakePlayingSound();
        if (CheckButtonJammerMotor()) // ジャマーモーターのボタンが押されたかチェック
            DisableJammerMotor();     // ジャマーモーターを無効にする
        ChangePhaseJammerMotor();     // ジャマーモーターのフェーズを変更する
    }
}