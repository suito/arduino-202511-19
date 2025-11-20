// ピン設定
const int LED_PIN = 8;        // LEDの接続ピン
const int BUTTON_PIN = 2;     // 記録イベント用ボタン

// 状態管理用変数
int pressCount = 0;           // ボタンが押された回数を記録
unsigned long lastPressTime = 0; // 最後にボタンが押された時間 (millis()の値)
int lastButtonState = HIGH;   // 前回のボタンの状態を記憶 (チャタリング対策用)

// ----------------------------------------
// 1. Setup関数
// ----------------------------------------
void setup() {
  Serial.begin(9600); // シリアル通信を開始 (速度 9600 bps)
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 内部プルアップ抵抗を使用
  
  digitalWrite(LED_PIN, LOW);
  Serial.println("--- Event Recorder Ready ---");
  Serial.println("Time (ms), Count, Event");
}

// ----------------------------------------
// 2. Loop関数
// ----------------------------------------
void loop() {
  // 現在のボタンの状態を読み取る (内部プルアップなので LOW が押された状態)
  int currentButtonState = digitalRead(BUTTON_PIN);
  
  // チャタリング対策と「押された瞬間」の検出
  // ボタンが「押されていない状態 (HIGH)」から「押された状態 (LOW)」に変わった瞬間を検出する
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    // ------------------------------------
    // 【イベント発生時の処理】
    // ------------------------------------
    
    // 1. ノイズ防止のため、少し待つ (チャタリング対策)
    delay(20); 

    // 2. 再度ボタンの状態を確認し、本当に押されていたら処理を続行
    if (digitalRead(BUTTON_PIN) == LOW) {
      
      // 3. 記録と計測
      unsigned long eventTime = millis(); // イベント発生時の正確な時刻を取得
      pressCount++;                     // 回数をカウントアップ
      
      // 4. LEDの点灯
      digitalWrite(LED_PIN, HIGH); // LEDを点灯させる
      
      // 5. シリアルモニタへ出力 (IDEのテキスト出力)
      Serial.print(eventTime);      // 発生時刻
      Serial.print(", ");
      Serial.print(pressCount);     // 回数
      Serial.print(", Button Pressed!");
      // 最後に押されてから何ミリ秒経過したかを計算して出力
      if (lastPressTime != 0) {
        Serial.print(" (Interval: ");
        Serial.print(eventTime - lastPressTime);
        Serial.println("ms)");
      } else {
        Serial.println();
      }
      
      // 6. タイマーを更新
      lastPressTime = eventTime;
    }
  }

  // ------------------------------------
  // 【イベント終了時の処理】
  // ------------------------------------
  
  // ボタンが離されたらLEDを消灯
  if (currentButtonState == HIGH) {
    digitalWrite(LED_PIN, LOW);
  }
  
  // 次のループのために現在のボタンの状態を記憶
  lastButtonState = currentButtonState;
}