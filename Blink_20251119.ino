// ピンの設定
const int LED_PIN = 8;
const int BUTTON_PIN = 2;

// 状態管理用の変数 (0:消灯, 1:点灯, 2:遅い点滅, 3:速い点滅)
int currentMode = 0; 

// チャタリング防止用の変数
int lastButtonState = HIGH;

// 点滅制御用の変数 (millis()用)
unsigned long previousMillis = 0; 
int blinkState = LOW; // 点滅中のLEDのON/OFF状態

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // 1. ボタンの状態検出
  int currentButtonState = digitalRead(BUTTON_PIN);

  // 「押された瞬間 (HIGH -> LOW)」を検出
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    
    // ★★ 修正箇所：厳密なチャタリング確認 ★★
    // わずかに待つ（20ms）
    delay(20); 
    
    // 20ms後もまだボタンが押されている（LOW）か再確認する
    if (digitalRead(BUTTON_PIN) == LOW) {
      
      // 2. モードを次の状態へ進める
      currentMode++;
      
      // 4番目の状態（値が4）になったら、0（消灯）に戻す
      if (currentMode > 3) {
        currentMode = 0;
      }
      
      // モード変更時の初期処理と即時反映
      if (currentMode == 0) {
        digitalWrite(LED_PIN, LOW); // モード0: 消灯
      } else if (currentMode == 1) {
        digitalWrite(LED_PIN, HIGH); // モード1: 点灯
      }
    }
    
    // 最後にlastButtonStateを更新する前に、現在のcurrentButtonStateを読み取る
    // この行を下に移動することで、最後のボタン状態の記録をループの最後で行います
    
  } 
  
  // 3. モードに応じた処理の実行
  
  if (currentMode == 2 || currentMode == 3) {
    // 点滅モードの場合のみ、以下のmillis()タイマー処理を実行

    // 現在の点滅間隔を設定
    long blinkInterval;
    if (currentMode == 2) {
      blinkInterval = 1000; // 遅い点滅 (1秒間隔)
    } else { // currentMode == 3
      blinkInterval = 200;  // 速い点滅 (0.2秒間隔)
    }
    
    // 設定時間以上経過したかチェック
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis; // 時刻をリセット

      // LEDの状態を反転
      if (blinkState == LOW) {
        blinkState = HIGH;
      } else {
        blinkState = LOW;
      }
      
      // 反転させた状態を出力
      digitalWrite(LED_PIN, blinkState);
    }
    
  } 

  // 4. 状態の更新 (ループの最後で実行)
  // 今回読み取ったボタン状態を、次回のループのために記憶する
  lastButtonState = currentButtonState;
}