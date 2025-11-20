// 抵抗つまみを回してLEDの明るさを変えるプログラム　2025/11/20

// -------------------- ピン定義 --------------------
// 可変抵抗を接続するアナログピン (入力)
const int POT_PIN = A0; 
// LEDを接続するPWM対応デジタルピン (出力)
const int LED_PIN = 9; 

void setup() {
  // LEDピンを出力モードに設定 (可変抵抗ピンは analogRead のため設定不要)
  pinMode(LED_PIN, OUTPUT);
  
  // シリアルモニタを起動 (読み取った値を確認するため)
  Serial.begin(9600);
}

void loop() {
  // 1. 可変抵抗の入力値を読み取る (0〜1023)
  int sensorValue = analogRead(POT_PIN);
  
  // 2. 値の確認のためシリアルモニタに出力
  Serial.print("Sensor Value (0-1023): ");
  Serial.println(sensorValue);

  sensorValue = sensorValue / 255 * 255;//あえて十段階ぐらいに丸める。
  
  // 3. 入力値の範囲 (0-1023) を LED出力の範囲 (0-255) に変換する
  // map(変換元の値, 変換元の最小値, 変換元の最大値, 変換後の最小値, 変換後の最大値)
  int ledBrightness = map(sensorValue, 0, 1023, 55, 255); //完全に消えないように最低値を55にする。
  
  // 4. LEDピンに調光値を出力する
  // PWM信号を出力し、LEDの明るさを調整
  analogWrite(LED_PIN, ledBrightness);
  
  // 処理が速すぎるので少し待つ
  delay(10);
}