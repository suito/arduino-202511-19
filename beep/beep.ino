/*
 * スケッチ名: Volume_Tone.ino
 * 概要: 可変抵抗器 (ボリューム) を回して配列の音階を鳴らし、ボタンでメジャー/沖縄音階を切り替える
 */

// ----------------------------------------------------
// 1. ピン設定と定数
// ----------------------------------------------------
const int SPEAKER_PIN = 4;      // スピーカー接続ピン (D4)
const int POT_PIN = A0;         // 可変抵抗器 (ボリューム) 接続ピン (A0)
const int MODE_BUTTON_PIN = 5;  // モード切り替えボタン (D5, INPUT_PULLUPを使用)

// ----------------------------------------------------
// 2. 音階の周波数定義 (Hz)
// ----------------------------------------------------
const int NOTE_C4 = 262; 
const int NOTE_D4 = 294; 
const int NOTE_E4 = 330; 
const int NOTE_F4 = 349; 
const int NOTE_G4 = 392; 
const int NOTE_A4 = 440; 
const int NOTE_B4 = 494; 
const int NOTE_C5 = 523; 
const int NOTE_E5 = 659; // ミ(高)

// ----------------------------------------------------
// 3. 音階配列の定義
// ----------------------------------------------------

// メジャーコード音階配列 (Cメジャー)
const int TONES_MAJOR[] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, 
  NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, 
  NOTE_E5
};
const int NUM_MAJOR_TONES = sizeof(TONES_MAJOR) / sizeof(TONES_MAJOR[0]); 

// 沖縄音階配列 (Cを基準とした琉球音階: C, E, F, G, B)
const int TONES_OKINAWA[] = {
  NOTE_C4, 
  NOTE_E4, 
  NOTE_F4, 
  NOTE_G4, 
  NOTE_B4, 
  NOTE_C5,
  NOTE_E5
};
const int NUM_OKINAWA_TONES = sizeof(TONES_OKINAWA) / sizeof(TONES_OKINAWA[0]); 

// ----------------------------------------------------
// 4. 状態管理変数
// ----------------------------------------------------

// 0: メジャー, 1: 沖縄音階
int currentMode = 0; 

// 無音にするアナログ値の閾値
const int MUTE_THRESHOLD = 20; 

// ボタンチャタリング・短押し検知用
unsigned long lastButtonPressTime = 0;
const long DEBOUNCE_DELAY = 200; // 200msのディレイ
bool modeButtonLastState = HIGH; // 前回のloop()でのボタン状態 (プルアップなので初期値はHIGH)

// ----------------------------------------------------
// 5. Setup関数
// ----------------------------------------------------

void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
  // モード切り替えボタンを入力プルアップに設定
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP); 
  
  Serial.begin(9600); 
  Serial.println("--- Array Tone Ready ---");
  Serial.println("初期モード: MAJOR (メジャーコード)");
}

// ----------------------------------------------------
// 6. Loop関数
// ----------------------------------------------------

void loop() {
  unsigned long currentTime = millis();
  
  // 1. アナログ値の読み取り (0〜1023)
  int potValue = analogRead(POT_PIN);
  long frequency;
  int toneIndex = -1; 
  
  // 2. モード切り替えボタンの判定 (エッジ検出ロジックによる安全な切り替え)
  int modeButtonCurrentState = digitalRead(MODE_BUTTON_PIN);
  
  // ボタンが離された瞬間 (LOW -> HIGH) かつ、チャタリングディレイを過ぎていれば処理を実行
  if (modeButtonLastState == LOW && modeButtonCurrentState == HIGH) {
    if (currentTime - lastButtonPressTime > DEBOUNCE_DELAY) {
      
      // モードを切り替える (0 -> 1, 1 -> 0)
      currentMode = 1 - currentMode; 
      lastButtonPressTime = currentTime; // 最後の処理時刻を更新
      
      // モード切り替え時の通知と音
      if (currentMode == 0) {
        Serial.println("--- MODE: MAJOR (メジャーコード) に切り替えました ---");
        tone(SPEAKER_PIN, NOTE_C5, 50); 
      } else {
        Serial.println("--- MODE: OKINAWA (沖縄音階) に切り替えました ---");
        tone(SPEAKER_PIN, NOTE_C4, 50); 
      }
    }
  }
  
  // 次のloop()のために現在の状態を保存
  modeButtonLastState = modeButtonCurrentState;

  // 3. 無音判定ロジック
  if (potValue <= MUTE_THRESHOLD) {
    noTone(SPEAKER_PIN);
    frequency = 0; 
  } else {
    // 4. 現在のモードに基づいた配列の選択
    const int* currentTones;
    int currentNumTones;
    
    if (currentMode == 0) {
      currentTones = TONES_MAJOR;
      currentNumTones = NUM_MAJOR_TONES;
    } else {
      currentTones = TONES_OKINAWA;
      currentNumTones = NUM_OKINAWA_TONES;
    }

    // 5. アナログ値から配列インデックスへの変換
    // アナログ値を 0 から (要素数 - 1) のインデックス範囲にマッピングする
    toneIndex = map(potValue, MUTE_THRESHOLD + 1, 1023, 0, currentNumTones - 1);
    
    // 配列から対応する周波数を取得
    frequency = currentTones[toneIndex];

    // 6. スピーカーから音を鳴らす
    tone(SPEAKER_PIN, frequency);
  }

  // 7. デバッグ情報の出力
  Serial.print("Mode: ");
  Serial.print(currentMode == 0 ? "メジャー" : "沖縄");
  Serial.print(" | Value: ");
  Serial.print(potValue);
  Serial.print(" -> Index: ");
  Serial.print(toneIndex); 
  Serial.print(" -> Freq (Hz): ");
  Serial.println(frequency);
  
  delay(10); 
}