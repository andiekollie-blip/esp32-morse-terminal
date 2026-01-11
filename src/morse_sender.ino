#include <WiFi.h>
#include <esp_now.h>

// ================== PIN DEFINITIONS ==================
const int buttonPin = 13;   // INPUT_PULLUP
const int buzzerPin = 27;
const int ledPin    = 26;

// ================== MORSE TIMING ==================
const uint16_t DOT_MS        = 150;
const uint16_t LETTER_GAP_MS = DOT_MS * 3;
const uint16_t WORD_GAP_MS   = DOT_MS * 7;
const uint16_t DOT_DASH_THRESHOLD_MS = DOT_MS * 2;
const uint16_t DEBOUNCE_MS = 25;

// ================== BUZZER ==================
const uint16_t TONE_HZ = 5000;

// ================== ESP-NOW ==================
// ✅ CYD MAC (your example)
uint8_t RECEIVER_MAC[] = {0x4C, 0xC3, 0x82, 0xCF, 0x3A, 0x94};

typedef struct {
  char ch;
} MorsePacket;

MorsePacket packet;

// ================== BUTTON STATE ==================
bool lastStableState = HIGH;
bool lastRawState    = HIGH;
unsigned long lastDebounceChange = 0;

unsigned long pressStartMs  = 0;
unsigned long lastReleaseMs = 0;

String currentLetter = "";
bool letterActive = false;
// ================== MORSE TABLE ==================
struct MorseMap {
  const char* code;
  char ch;
};

MorseMap MORSE[] = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},  {".", 'E'},
  {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'}, {"..", 'I'},   {".---", 'J'},
  {"-.-", 'K'},  {".-..", 'L'}, {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},
  {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'}, {"-.--", 'Y'},
  {"--..", 'Z'},
  {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
  {"---..", '8'}, {"----.", '9'}
};

char decodeMorse(const String& code) {
  for (int i = 0; i < sizeof(MORSE) / sizeof(MORSE[0]); i++) {
    if (code.equals(MORSE[i].code)) {
      return MORSE[i].ch;
    }
  }
  return '#';
}


void beepOn() {
  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, TONE_HZ);
}

void beepOff() {
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);
}

void sendChar(char c) {
  packet.ch = c;
  esp_now_send(RECEIVER_MAC, (uint8_t*)&packet, sizeof(packet));
  Serial.print(c);
}

// ---------- ESP-NOW send callback (core-version compatible) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onSend(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? " ✔ sent" : " ✖ failed");
}
#else
void onSend(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? " ✔ sent" : " ✖ failed");
}
#endif

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init FAILED");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(onSend);

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, RECEIVER_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true) delay(1000);
  }

  Serial.println("Morse Sender Ready");
}

void loop() {
  unsigned long now = millis();
  bool raw = digitalRead(buttonPin);

  // Debounce
  if (raw != lastRawState) {
    lastDebounceChange = now;
    lastRawState = raw;
  }

  if ((now - lastDebounceChange) > DEBOUNCE_MS) {
    if (raw != lastStableState) {
      bool prev = lastStableState;
      lastStableState = raw;

      // Press
      if (prev == HIGH && raw == LOW) {
        pressStartMs = now;
        beepOn();
      }

      // Release
      if (prev == LOW && raw == HIGH) {
        beepOff();

        unsigned long pressTime = now - pressStartMs;
        lastReleaseMs = now;

        char symbol = (pressTime < DOT_DASH_THRESHOLD_MS) ? '.' : '-';
        currentLetter += symbol;
        sendChar(symbol);
        Serial.print(symbol);


        letterActive = true;
      }
    }
  }

  // Gap detection
  if (letterActive && lastStableState == HIGH && currentLetter.length() > 0) {
    unsigned long gap = now - lastReleaseMs;

    if (gap >= WORD_GAP_MS) {
      sendChar('/'); //end of letter
      sendChar('|'); //space between words 
      currentLetter = "";
      letterActive = false;
      Serial.print("  ");
    } 
    else if (gap >= LETTER_GAP_MS) {
      sendChar('/'); //end of letter
      currentLetter = "";
      letterActive = false;
      Serial.print(" ");
    }
  }
}


