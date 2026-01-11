#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>

// ================== TFT ==================
TFT_eSPI tft = TFT_eSPI();

// ================== BUTTON ==================
#define CLEAR_BUTTON 0   // BOOT button

// ================== DATA ==================
typedef struct {
  char ch;
} MorsePacket;

// ================== BUFFERS ==================
String rawBuffer  = "";
String textBuffer = "";
String currentMorse = "";

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

// ================== DISPLAY ==================
void redraw() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(0, 10);
  tft.print("RAW:");
  tft.setCursor(0, 35);
  tft.print(rawBuffer);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 80);
  tft.print("TXT:");
  tft.setCursor(0, 105);
  tft.print(textBuffer);
}

void clearDisplay() {
  rawBuffer = "";
  textBuffer = "";
  currentMorse = "";
  redraw();
}

// ================== ESP-NOW RECEIVE ==================
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  MorsePacket packet;
  memcpy(&packet, data, sizeof(packet));

  char c = packet.ch;

  // ---------- RAW ----------
  rawBuffer += c;

  // ---------- DECODE ----------
  if (c == '.' || c == '-') {
    currentMorse += c;
  }
  else if (c == '/') {  // end of letter
    char decoded = decodeMorse(currentMorse);
    textBuffer += decoded;
    currentMorse = "";
  }
  else if (c == '|') {  // word space
    textBuffer += ' ';
  }

  redraw();
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  // Backlight
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  pinMode(CLEAR_BUTTON, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.setTextSize(2);

  clearDisplay();

  WiFi.mode(WIFI_STA);
  Serial.print("CYD MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    tft.setTextColor(TFT_RED);
    tft.println("ESP-NOW FAILED");
    while (true);
  }

  esp_now_register_recv_cb(onReceive);

  tft.setTextColor(TFT_CYAN);
  tft.setCursor(0, 200);
  tft.println("BOOT = CLEAR");
}

// ================== LOOP ==================
void loop() {
  if (digitalRead(CLEAR_BUTTON) == LOW) {
    delay(300);
    clearDisplay();
  }
}
