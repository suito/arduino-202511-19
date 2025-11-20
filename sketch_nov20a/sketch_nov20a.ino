#include <EEPROM.h>

// ----------------------------------------------------
// 1. EEPROM関数プロトタイプ宣言 (別のタブにある関数)
// ----------------------------------------------------
void loadDataFromEEPROM();
void saveDataToEEPROM();
void resetDataInEEPROM();

// ----------------------------------------------------
// 2. ピン設定と定数、状態管理の定義
// ----------------------------------------------------
const int LED_PIN = 8;        
const int RECORD_BUTTON_PIN = 2; 
const int PLAY_SAVE_BUTTON_PIN = 3; 

// 状態管理
enum State { READY, RECORDING, PLAYBACK };
State currentState = READY;

// 記録データ用配列
const int MAX_EVENTS = 20;
unsigned long pressDurationData[MAX_EVENTS]; 
int recordedEvents = 0; 

// タイマー用変数
unsigned long currentPressStart = 0; // 現在の長押しが始まった時刻
unsigned long lastEventTime = 0;     // 再生時に最後のイベントが起こった時刻
unsigned long buttonHoldStart = 0;   // ボタン長押し開始時間を記録するタイマー
const long RESET_DURATION = 3000;    // リセットに必要な長押し時間 (3000ms = 3秒)

// EEPROMアドレス設定 (eeprom_reset.inoと共有)
const int COUNT_ADDRESS = 0; 
const int DATA_START_ADDRESS = sizeof(int); 

// 再生用変数
int playbackIndex = 0; 
enum PlayState { PS_LIGHT_ON, PS_INTERVAL_OFF }; // 再生中のサブ状態
PlayState currentPlayState = PS_LIGHT_ON;
const long FIXED_INTERVAL = 1000; // インターバル（無点灯時間）1秒固定


// ----------------------------------------------------
// 3. Setup関数
// ----------------------------------------------------

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RECORD_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(PLAY_SAVE_BUTTON_PIN, INPUT_PULLUP); 
  digitalWrite(LED_PIN, LOW);

  // setup時に自動でEEPROMをリセット (eeprom_reset.inoにある関数)
  resetDataInEEPROM(); 
  
  Serial.println("\n--- システム準備完了 ---");
  Serial.println("D2を押して新しい記録を開始してください。");
}

// ----------------------------------------------------
// 4. その他の関数
// ----------------------------------------------------

// ★ 修正後のボタン押しを一度だけ検出する関数 (短押し検出用)
bool checkButtonPress(int pin) {
  // ボタンが押されたら (LOW)
  if (digitalRead(pin) == LOW) {
    delay(20); // チャタリング対策
    if (digitalRead(pin) == LOW) {
      return true; // 押された瞬間を検出
    }
  }
  return false;
}

// ----------------------------------------------------
// 5. Loop関数と状態遷移ロジック (デバッグログ追加)
// ----------------------------------------------------

void loop() {
  
  unsigned long currentTime = millis(); // loop開始時の時刻を取得
  
  // ★ デバッグ用: currentTimeを常時出力
  // このメッセージが連続して流れていれば、プログラムはフリーズしていません。
  //Serial.print("CurrentTime: ");
  //Serial.println(currentTime); 
  
  switch (currentState) {
    

// ====================================================
    case READY: // 待機状態 (初期状態)
    // ====================================================
      digitalWrite(LED_PIN, LOW);
      
      // D2ボタン: 記録開始
      if (checkButtonPress(RECORD_BUTTON_PIN)) {
        // 連続検知を防ぐため、ボタンが離されるまで待つ
        while(digitalRead(RECORD_BUTTON_PIN) == LOW); 
        
        currentState = RECORDING;
        recordedEvents = 0; 
        currentPressStart = 0; 
        Serial.println("\n--- 記録モード 開始 (D2: 長押し記録, D3: 保存) ---"); 
      }

      // D3ボタン: 再生開始
      if (checkButtonPress(PLAY_SAVE_BUTTON_PIN)) {
        // ★ while ループは削除済み
        
        if (recordedEvents > 0) {
          // ★ デバッグログ追加: ここが表示されればフリーズしていない
          Serial.println("DEBUG: Playing..."); 
          
          currentState = PLAYBACK;
          playbackIndex = 0;
          currentPlayState = PS_LIGHT_ON; 
          
          // 再生開始直前の currentTime でタイマーを強制的に初期化
          lastEventTime = currentTime; 
          
          Serial.println("\n--- 再生モード 開始 ---");
          Serial.print("記録されたイベント数: ");
          Serial.println(recordedEvents); // この直後で停止している
          // ★ ここで処理を中断しない限り、次のloop()では case PLAYBACK が実行される
        } else {
          Serial.println("記録されたイベントがありません。D2を押して記録を開始してください。");
        }
      }
      
      // リセット処理のチェック（D2とD3を同時長押し）
      if (digitalRead(RECORD_BUTTON_PIN) == LOW && digitalRead(PLAY_SAVE_BUTTON_PIN) == LOW) {
        if (buttonHoldStart == 0) {
          buttonHoldStart = currentTime; 
          Serial.println("\n手動リセット操作開始... (3秒間押し続けてください)");
        }
        
        if (currentTime - buttonHoldStart >= RESET_DURATION) {
          resetDataInEEPROM(); 
          buttonHoldStart = 0; 
        }
      } else {
        buttonHoldStart = 0;
      }
      
      break;


    // ====================================================
    case RECORDING: // 記録状態 (長押し時間計測)
    // ====================================================

      // D2ボタンの状態を直接チェック
      bool isRecordingButtonPressed = (digitalRead(RECORD_BUTTON_PIN) == LOW);

      // ★ 長押し開始の検出 (ボタンが押された瞬間)
      if (isRecordingButtonPressed && currentPressStart == 0) {
        currentPressStart = currentTime; // 押し始めの時刻を記録
        digitalWrite(LED_PIN, HIGH); // 押し始めたらLED点灯
      }

      // ★ 長押し終了と記録の検出 (ボタンが離された瞬間)
      if (!isRecordingButtonPressed && currentPressStart != 0) {
        
        unsigned long duration = currentTime - currentPressStart;
        
        if (recordedEvents < MAX_EVENTS) {
          
          pressDurationData[recordedEvents] = duration; // 押し続けた時間を記録
          recordedEvents++;
          
          Serial.print("イベント ");
          Serial.print(recordedEvents);
          Serial.print("を記録。点灯時間 (Duration): ");
          Serial.print(duration);
          Serial.println("ms");
          
        } else {
          Serial.println("最大イベント数に達しました。D3を押して保存してください。");
        }
        
        // 記録完了とリセット
        currentPressStart = 0; // タイマーをリセット
        digitalWrite(LED_PIN, LOW); // LEDを消灯
        
        delay(50); // チャタリング防止
      }
      
      // D3ボタン: 保存
      if (checkButtonPress(PLAY_SAVE_BUTTON_PIN)) {
        while(digitalRead(PLAY_SAVE_BUTTON_PIN) == LOW); // 離されるのを待つ
        saveDataToEEPROM(); 
        currentState = READY; // 保存したら待機状態に戻る
      }
      break;


// ... (loop()関数内) ...

    // ====================================================
    case PLAYBACK: // 再生状態 (点灯と1秒インターバル)
    // ====================================================
      
      // D3ボタン: 再生中止
      if (checkButtonPress(PLAY_SAVE_BUTTON_PIN)) {
        while(digitalRead(PLAY_SAVE_BUTTON_PIN) == LOW); // 離されるのを待つ
        currentState = READY;
        currentPlayState = PS_LIGHT_ON; 
        digitalWrite(LED_PIN, LOW);
        Serial.println("--- 再生 中止 ---");
        break;
      }
      
      // 全てのイベントを再生し終えたら終了
      if (playbackIndex >= recordedEvents) {
        Serial.println("--- 再生 完了 ---");
        currentState = READY;
        currentPlayState = PS_LIGHT_ON; 
        digitalWrite(LED_PIN, LOW);
        break;
      }
      
      switch (currentPlayState) {
        
        case PS_LIGHT_ON: // ★ 点灯 (長押し時間) の再生
          unsigned long lightDuration = pressDurationData[playbackIndex];
          
          // ★ ロジック強化: 点灯処理がまだ開始されていない、または前回インターバルから戻ったばかり
          if (digitalRead(LED_PIN) == LOW && (currentTime - lastEventTime > 50 || playbackIndex == 0)) {
              digitalWrite(LED_PIN, HIGH);
              lastEventTime = currentTime; 
              
              Serial.print("再生中: ");
              Serial.print(playbackIndex + 1);
              Serial.print("/");
              Serial.print(recordedEvents);
              Serial.print(", [点灯]時間: ");
              Serial.print(lightDuration);
              Serial.println("ms");
          }
          
          // 点灯時間が経過したら、次のサブステート（インターバル）へ
          if (currentTime - lastEventTime >= lightDuration) {
            digitalWrite(LED_PIN, LOW); 
            lastEventTime = currentTime; 
            currentPlayState = PS_INTERVAL_OFF;
          }
          break;
          
        case PS_INTERVAL_OFF: // ★ インターバル (1秒固定) の再生
          
          // インターバル開始ログ出力
          if (digitalRead(LED_PIN) == LOW) { // ここではLOWをチェック（直前のPS_LIGHT_ONでLOWになっている）
              lastEventTime = currentTime;
              Serial.print("再生中: ");
              Serial.print(playbackIndex + 1);
              Serial.print("/");
              Serial.print(recordedEvents);
              Serial.print(", [インターバル]時間: ");
              Serial.print(FIXED_INTERVAL);
              Serial.println("ms");
          }
          
          // 1秒経過したら、次のイベントの点灯へ
          if (currentTime - lastEventTime >= FIXED_INTERVAL) {
            playbackIndex++; // 次のイベントへ進む
            currentPlayState = PS_LIGHT_ON; // サブステートを点灯に戻す
          }
          break;
      }
      break;


  }
}