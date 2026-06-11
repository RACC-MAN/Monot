#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi設定
const char* ssid = "ssid";
const char* password = "passwaord";
const char* host = "ip_address";  /

// 送信先PC
const int port = 9000;

WiFiUDP udp;
#define PACKET_SIZE 1400
uint16_t frame_id = 0;

// ================= Camera (OV3650) =================
// ※ ESP32-WROVER + 外付けOV3650想定
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  21
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    19
#define Y4_GPIO_NUM    18
#define Y3_GPIO_NUM    5
#define Y2_GPIO_NUM    4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22


bool setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // config.frame_size = FRAMESIZE_VGA;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  Serial.println("Camera init OK");
  return true;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected.");

  // ---- Camera ----
  if (!setupCamera()) {
    Serial.println("Camera failed. Stop.");
    while (1) delay(1000);
  }

  udp.begin(port);
  Serial.println("udp awaked.");
  frame_id = 0;
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Capture failed");
      delay(1000);
      return;
  }

  uint16_t total_packets = (fb->len + PACKET_SIZE - 1) / PACKET_SIZE;

  for (uint16_t i = 0; i < total_packets; i++) {
    int offset = i * PACKET_SIZE;
    int fb_size = fb->len - offset;
    int chunk = (PACKET_SIZE > fb_size) ? fb_size : PACKET_SIZE;

    udp.beginPacket(host, port);

    // ヘッダ
    uint16_t fid = htons(frame_id);
    uint16_t pid = htons(i);
    uint16_t tpk = htons(total_packets);

    udp.write((uint8_t*)&fid, 2);
    udp.write((uint8_t*)&pid, 2);
    udp.write((uint8_t*)&tpk, 2);

    // データ
    udp.write(fb->buf + offset, chunk);

    udp.endPacket();
    delayMicroseconds(100);
  }

  frame_id++;
  esp_camera_fb_return(fb);

  delay(10);
}