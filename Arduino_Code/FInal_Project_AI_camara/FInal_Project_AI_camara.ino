#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === WiFi ===
const char* ssid = "VALTEX";
const char* password = "udaebechi1zieh6U";

// === ChatGPT API ===
const String apiKey = "sk-B7v4GFzEfa3lD02e5odkenp1lndwuLj52q20mNgUv4";
const String endpoint = "https://free.yunwu.ai/v1/chat/completions";
const String question = "Summarize this image in 5-8 words.";

// === BUTTON & BUZZER ===
#define BUTTON_PIN 13
#define BUZZER_PIN 2

// === OLED (0.96 inch) ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 15
#define OLED_SCL 14
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === CAMERA PINS ===
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// === BASE64 Encoding ===
String encodeToBase64(const uint8_t* data, size_t length) {
  const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String result;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (length--) {
    char_array_3[i++] = *(data++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++) {
        result += base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; j < i + 1; j++) {
      result += base64_chars[char_array_4[j]];
    }

    while (i++ < 3) {
      result += '=';
    }
  }

  return result;
}

// === ChatGPT API Call ===
String sendToChatGPT(String base64Img) {
  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + apiKey);

  DynamicJsonDocument doc(4096);
  doc["model"] = "gpt-4o";
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";

  JsonArray content = userMsg.createNestedArray("content");

  JsonObject txt = content.createNestedObject();
  txt["type"] = "text";
  txt["text"] = question;

  JsonObject img = content.createNestedObject();
  img["type"] = "image_url";
  img["image_url"]["url"] = "data:image/jpeg;base64," + base64Img;
  img["image_url"]["detail"] = "low";

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);
  String response = http.getString();
  http.end();

  if (code == 200) {
    DynamicJsonDocument resDoc(4096);
    deserializeJson(resDoc, response);
    return resDoc["choices"][0]["message"]["content"].as<String>();
  } else {
    return "API ERROR " + String(code);
  }
}

// === Display Scrolling Variables ===
String fullResponse = "";
int currentStartLine = 0;
const int maxDisplayLines = 7; // 1 header + 6 content lines
const int charsPerLine = 21;
bool newResponseAvailable = false;

// === SETUP ===
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  // WiFi
  WiFi.begin(ssid, password);
  display.println("Connecting WiFi...");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.println("WiFi OK!");
  display.display();

  // Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count     = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Cam Fail");
    display.display();
    Serial.println("❌ Camera init failed!");
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ready! Press Btn");
  display.display();
}

void processResponse(String text) {
  fullResponse = text;
  currentStartLine = 0;
  newResponseAvailable = true;
  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Split the full response into displayable lines
  String lines[20];
  int totalLines = 0;
  
  // First split into words
  String words[100];
  int wordCount = 0;
  int lastSpace = -1;
  
  for (int i = 0; i < fullResponse.length(); i++) {
    if (fullResponse.charAt(i) == ' ') {
      words[wordCount++] = fullResponse.substring(lastSpace + 1, i);
      lastSpace = i;
      if (wordCount >= 99) break;
    }
  }
  // Add last word
  if (lastSpace < fullResponse.length() - 1) {
    words[wordCount++] = fullResponse.substring(lastSpace + 1);
  }

  // Now build wrapped lines
  String currentLine = "";
  for (int i = 0; i < wordCount; i++) {
    if (currentLine.length() + words[i].length() + 1 <= charsPerLine) {
      currentLine += (currentLine.length() ? " " : "") + words[i];
    } else {
      lines[totalLines++] = currentLine;
      currentLine = words[i];
      if (totalLines >= 19) break;
    }
  }
  if (currentLine.length() > 0 && totalLines < 20) {
    lines[totalLines++] = currentLine;
  }

  // Display header
  display.setCursor(0, 0);
  display.print("Response ");
  if (totalLines > maxDisplayLines - 1) {
    display.print((currentStartLine/(maxDisplayLines-1))+1);
    display.print("/");
    display.print((totalLines + (maxDisplayLines-2))/(maxDisplayLines-1));
  }

  // Display current lines
  for (int i = 0; i < maxDisplayLines - 1; i++) {
    if (currentStartLine + i < totalLines) {
      display.setCursor(0, 8 + i * 8);
      display.println(lines[currentStartLine + i]);
    }
  }

  // Show scroll indicator if there's more text
  if (totalLines > currentStartLine + maxDisplayLines - 1) {
    display.setCursor(110, 56);
    display.print("▼");
  }
  if (currentStartLine > 0) {
    display.setCursor(110, 8);
    display.print("▲");
  }

  display.display();
  newResponseAvailable = false;
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void captureAndSend() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Capturing...");
  display.display();

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Capture Failed");
    return;
  }

  String base64Img = encodeToBase64(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  String reply = sendToChatGPT(base64Img);
  processResponse(reply);
  beep();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(300); // Debounce
    
    if (newResponseAvailable) {
      updateDisplay();
    } else if (fullResponse.length() > 0) {
      int totalDisplayLines = fullResponse.length() / charsPerLine + 1;
      currentStartLine += maxDisplayLines - 1;
      if (currentStartLine >= totalDisplayLines) {
        currentStartLine = 0;
      }
      updateDisplay();
    } else {
      captureAndSend();
    }
  }
}