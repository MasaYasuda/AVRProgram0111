メモ

ピン｜
PB1(OC1A) スピーカー出力
PB2       ジャマーモーター出力
PB3(OC2A) コンベア出力
PB4　　　　モード選択スイッチ　
PB5　　　　コンベアボタン
PB6　　　　音直しボタン
PB7　　　　ジャマーモーターボタン

PC0　　　　LEDボリューム
PC1　　　　カードスキャン上
PC2　　　　カードスキャン下
PC3       落下センサー

PD0　　　　RX
PD1　　　　TX

PD3(OC2B) LED出力


イベント|
  変なスピーカー音
  妨害モーター
  LED消灯
  カードスキャン（LEDマトリクスで指示）
  LEDマトリクスカウントダウン
 
  その他|
 
  コンベア上昇
  
  景品ゲット
  スロットゲーム

# 実験4残タスク　Master

Event |       
WeirdSound        
JammerMotor        
Countdown          
CardScanner         
LightVariesLED 
SlotMaster           

MainMaster.c作成

DisableCardScanner();
DisableConveyor();
DisableCountdown();
DisableJammerMotor();
DisableLightVariesLED();
DisableWeirdSound();


# 実験4残タスク　Wifi

 