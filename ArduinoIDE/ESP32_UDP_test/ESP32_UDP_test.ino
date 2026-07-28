#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi設定
const char* SSID = "ssid";
const char* PASSWORD = "password";
const char* HOST = "ipadress";

// 送信先PC
const int IMAGE_PORT = 9000;

WiFiUDP udp_image;
#define PACKET_SIZE 1400
uint16_t frame_id = 0;

camera_config_t camera_config;
const int  JPEG_QUORITY =12;
uint8_t *jpeg_buf = NULL;
size_t jpeg_len = 0;

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

struct ImageHeader
{
    uint16_t frame_id;
    uint16_t packet_id;
    uint16_t total_packets;
    uint64_t timestamp_us;
};

bool setupCamera() {
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer   = LEDC_TIMER_0;

  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;

  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sscb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sscb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;

  camera_config.xclk_freq_hz = 10000000;
  camera_config.pixel_format = PIXFORMAT_JPEG;

  // config.frame_size = FRAMESIZE_VGA;
  camera_config.frame_size = FRAMESIZE_QVGA;
  camera_config.jpeg_quality = 10;
  camera_config.fb_count = 2;
  camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  Serial.println("Camera init OK");
  return true;
}

void send_image()
{
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
    return;

  uint16_t total_packets =
    (fb->len + PACKET_SIZE - 1) / PACKET_SIZE;

  for (uint16_t i = 0; i < total_packets; i++)
  {
    int offset = i * PACKET_SIZE;
    int chunk = min((int)fb->len - offset, PACKET_SIZE);

    ImageHeader hdr;
    hdr.frame_id = htons(frame_id);
    hdr.packet_id = htons(i);
    hdr.total_packets = htons(total_packets);
    hdr.timestamp_us = esp_timer_get_time();

    udp_image.beginPacket(HOST, IMAGE_PORT);
    udp_image.write((uint8_t*)&hdr, sizeof(hdr));
    udp_image.write(fb->buf + offset, chunk);
    udp_image.endPacket();
  }

  esp_camera_fb_return(fb);

  frame_id++;
}

void image_task(void *arg)
{
  while(true)
  {
    send_image();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Started Setup");

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi Success");

  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(500);

  if (!setupCamera()) {
    while(true);
  }

  udp_image.begin(IMAGE_PORT);

  xTaskCreatePinnedToCore(image_task, "image_task", 8192,
    NULL, 1, NULL, 1);
}


void loop() {
    vTaskDelay(portMAX_DELAY);
}
