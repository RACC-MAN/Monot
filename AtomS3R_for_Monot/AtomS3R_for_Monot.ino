#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <M5Unified.h>


#define USE_ATOMS3R_CAM
// #define USE_ATOMS3R_M12

#define STA_MODE
// #define AP_MODE

// WiFi設定
const char* SSID = "ssid";
const char* PASSWORD = "pass";
const char* HOST = "192.168.11.4";  // UbuntuのIP

// 送信先PC
const int IMAGE_PORT = 9000;
const int IMU_PORT = 9001;

WiFiUDP udp_image;
WiFiUDP udp_imu;

#define PACKET_SIZE 1400
uint16_t frame_id = 0;

#define POWER_GPIO_NUM 18
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  21
#define SIOD_GPIO_NUM  12
#define SIOC_GPIO_NUM  9
#define Y9_GPIO_NUM    13
#define Y8_GPIO_NUM    11
#define Y7_GPIO_NUM    17
#define Y6_GPIO_NUM    4
#define Y5_GPIO_NUM    48
#define Y4_GPIO_NUM    46
#define Y3_GPIO_NUM    42
#define Y2_GPIO_NUM    3
#define VSYNC_GPIO_NUM 10
#define HREF_GPIO_NUM  14
#define PCLK_GPIO_NUM  40

static camera_config_t camera_config = {
    .pin_pwdn     = PWDN_GPIO_NUM,
    .pin_reset    = RESET_GPIO_NUM,
    .pin_xclk     = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7       = Y9_GPIO_NUM,
    .pin_d6       = Y8_GPIO_NUM,
    .pin_d5       = Y7_GPIO_NUM,
    .pin_d4       = Y6_GPIO_NUM,
    .pin_d3       = Y5_GPIO_NUM,
    .pin_d2       = Y4_GPIO_NUM,
    .pin_d1       = Y3_GPIO_NUM,
    .pin_d0       = Y2_GPIO_NUM,

    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href  = HREF_GPIO_NUM,
    .pin_pclk  = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

#ifdef USE_ATOMS3R_CAM
    .pixel_format = PIXFORMAT_RGB565,
    .frame_size   = FRAMESIZE_QVGA,
#endif

#ifdef USE_ATOMS3R_M12
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size   = FRAMESIZE_UXGA,
#endif

    .jpeg_quality  = 12,
    .fb_count      = 2,
    .fb_location   = CAMERA_FB_IN_PSRAM,
    .grab_mode     = CAMERA_GRAB_LATEST,
    .sccb_i2c_port = 0,
};

struct ImuPacket
{
  float ax;
  float ay;
  float az;

  float gx;
  float gy;
  float gz;
};

void send_image()
{
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

    udp_image.beginPacket(HOST, IMAGE_PORT);

    // ヘッダ
    uint16_t fid = htons(frame_id);
    uint16_t pid = htons(i);
    uint16_t tpk = htons(total_packets);

    udp_image.write((uint8_t*)&fid, 2);
    udp_image.write((uint8_t*)&pid, 2);
    udp_image.write((uint8_t*)&tpk, 2);

    // データ
    udp_image.write(fb->buf + offset, chunk);

    udp_image.endPacket();
    delayMicroseconds(100);
  }

  frame_id++;
  esp_camera_fb_return(fb);
}

void send_imu()
{
  ImuPacket pkt;

  auto imu_update = M5.Imu.update();

  if(!imu_update) return;

  auto data = M5.Imu.getImuData();

  pkt.ax = data.accel.x;
  pkt.ay = data.accel.y;
  pkt.az = data.accel.z;

  pkt.gx = data.gyro.x;
  pkt.gy = data.gyro.y;
  pkt.gz = data.gyro.z;

  udp_imu.beginPacket(HOST, IMU_PORT);

  udp_imu.write(
    (uint8_t*)&pkt,
    sizeof(pkt));

  udp_imu.endPacket();
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected.");

  pinMode(POWER_GPIO_NUM, OUTPUT);
  digitalWrite(POWER_GPIO_NUM, LOW);
  delay(500);
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.println("Camera Init Fail");
    delay(1000);
    esp_restart();
  } else {
    Serial.println("Camera Init Success");
  }

  udp_image.begin(IMAGE_PORT);
  udp_imu.begin(IMU_PORT);
}


void loop() {
  send_image();
  send_imu();

  delay(20);
}
