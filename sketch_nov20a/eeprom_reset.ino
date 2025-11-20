// ----------------------------------------------------
// eeprom_reset.ino ファイル: EEPROM管理関数を格納
// ----------------------------------------------------

// グローバル変数はメインスケッチファイルから参照されます。

// EEPROMからデータを読み込む
void loadDataFromEEPROM() {
  EEPROM.get(COUNT_ADDRESS, recordedEvents);
  if (recordedEvents < 0 || recordedEvents > MAX_EVENTS) {
    recordedEvents = 0; 
  }
  if (recordedEvents > 0) {
    // データ配列名が pressDurationData であることを確認
    EEPROM.get(DATA_START_ADDRESS, pressDurationData);
  }
}

// EEPROMにデータを書き込む
void saveDataToEEPROM() {
  Serial.println("--- EEPROMにデータを保存中... ---");
  
  EEPROM.put(COUNT_ADDRESS, recordedEvents);
  // データ配列名が pressDurationData であることを確認
  EEPROM.put(DATA_START_ADDRESS, pressDurationData); 
  
  Serial.println("--- データ保存 完了！ ---");
  
  // 保存完了を示すためにLEDを素早く3回点滅
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(100);
    digitalWrite(LED_PIN, LOW); delay(100);
  }
}

// EEPROM内の全データをリセットする関数 (システム起動時/同時長押しで実行)
void resetDataInEEPROM() {
  Serial.println("\n--- EEPROMデータ自動消去中... ---");
  
  // イベント数をゼロに設定し、EEPROMに書き込む
  EEPROM.put(COUNT_ADDRESS, 0); 
  
  // 変数もリセット
  recordedEvents = 0;
  
  // LEDを点滅させて完了を通知
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH); delay(50);
    digitalWrite(LED_PIN, LOW); delay(50);
  }
  
  Serial.println("--- 全イベントデータ 消去完了！ (システム起動時) ---");
}