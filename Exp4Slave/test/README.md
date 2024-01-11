スロットのボタン

ボタンL PD4
ボタンC PD2
ボタンR PD3
---------------------
74HC595    -- AVR
SER(data)  -- PC0
RCK(latch) -- PC1
SCK(clock) -- PC2
---------------------

# Communication Protocol

## Master to Slave
Enable slot 
0b01000000
Enable countdown
0b00100000
Display Scan order
0b00010001
Stop Scan order
0b00010000

## Master to WifiModule
Confirm Connection(WaitingMode)
0b10000001

Tell Present Status
0b1abcdef0
----
a:Slot
b:Countdown
c:Scan
d:LighrVariesLED
e:WeirdSound
----

## Slave to Master
Tell Slot Failure
0b01000000
Tell Slot Success
0b01000001
Tell Slot Jackpot
0b01000010
Tell Countdown Failure
0b00100000
Tell Countdown Success
0b00100001

## WifiModule to Slave
beeing Connected 
0b10000000
Not Connected yet
0b10000001

Order Countdown
0b10100000
Order Scan
0b10010000
Order LightVariesLED
0b10001000
Order JammerMotor
0b10000100
Order WeirdSound
0b10000010
