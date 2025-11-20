/*
 * スケッチ名: Volume_Tone.ino
 * 概要: 可変抵抗器 (ボリューム) を回して、Cメジャーコードの構成音のみを鳴らす
 */

// ----------------------------------------------------
// 1. ピン設定と定数
// ----------------------------------------------------
const int SPEAKER_PIN = 4; // スピーカー接続ピン (D4)
const int POT_PIN = A0;    // 可変抵抗器 (ボリューム) 接続ピン (A0)

// ★ メジャーコードの構成音の周波数 (Cメジャー)
const int NOTE_C4 = 262; // ド (C)
const int NOTE_E4 = 330; // ミ (E)
const int NOTE_G4 = 392; // ソ (G)
const int NOTE_C5 = 523; // ド (C, 1オクターブ上)
const int NOTE_E5 = 659; // ミ (E, 1オクターブ上)
const int NOTE_G5 = 784; // ソ (G, 1オクターブ上)

// ★ 音階配列: つまみを回すとこの順に音が出る
const int chordTones[] = {
  NOTE_C4, NOTE_E4, NOTE_G4,
  NOTE_C5, NOTE_E5, NOTE_G5
};
const int NUM_TONES = sizeof(chordTones) / sizeof(chordTones[0]); // 音の数 (6種類)

// 無音にするアナログ値の閾値
const int MUTE_THRESHOLD = 20; 

// ----------------------------------------------------
// 2. Setup関数
// ----------------------------------------------------

void setup() {
  // スピーカーピンを出力に設定
  pinMode(SPEAKER_PIN, OUTPUT);

  // シリアル通信を開始 (デバッグ用)
  Serial.begin(9600); 
  Serial.println("--- C Major Tone Ready ---");
}

// ----------------------------------------------------
// 3. Loop関数
// ----------------------------------------------------

void loop() {
  // 1. アナログ値の読み取り (0〜1023)
  int potValue = analogRead(POT_PIN);
  long frequency;
  int toneIndex = -1; // デバッグ用

  // 2. 無音判定ロジック
  if (potValue <= MUTE_THRESHOLD) {
    noTone(SPEAKER_PIN);
    frequency = 0; 
  } else {
    // 3. ★ アナログ値から配列インデックスへの変換

    // MUTE_THRESHOLD+1 から 1023 の範囲を 0 から NUM_TONES-1 のインデックスにマッピング
    toneIndex = map(potValue, MUTE_THRESHOLD + 1, 1023, 0, NUM_TONES - 1);
    
    // map関数は値を制限するため、このチェックは必須ではないが、安全のため
    if (toneIndex < 0) toneIndex = 0;
    if (toneIndex >= NUM_TONES) toneIndex = NUM_TONES - 1;
    
    frequency = chordTones[toneIndex];

    // 4. スピーカーから音を鳴らす
    tone(SPEAKER_PIN, frequency);
  }

  // 5. デバッグ情報の出力
  Serial.print("Analog Value: ");
  Serial.print(potValue);
  Serial.print(" -> Index: ");
  Serial.print(toneIndex); 
  Serial.print(" -> Frequency (Hz): ");
  Serial.println(frequency);
  
  delay(10); 
}