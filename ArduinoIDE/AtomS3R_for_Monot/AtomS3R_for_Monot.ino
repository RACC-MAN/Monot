
#include "camera_pins.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <M5Unified.h>


#define STA_MODE
// #define AP_MODE

// WiFi設定
const char* SSID = "ssid";
const char* PASSWORD = "password";
const char* HOST = "ipadress";

// 送信先PC
const int IMAGE_PORT = 9000;
const int IMU_PORT = 9001;

WiFiUDP udp_image;
WiFiUDP udp_imu;

#define PACKET_SIZE 1400
uint16_t frame_id = 0;
const int  JPEG_QUORITY = 12;

uint8_t *jpeg_buf = NULL;
size_t jpeg_len = 0;

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

    .pixel_format = PIXFORMAT_YUV422,
    .frame_size   = FRAMESIZE_QVGA,

    .jpeg_quality  = JPEG_QUORITY,
    .fb_count      = 2,
    .fb_location   = CAMERA_FB_IN_PSRAM,
    .grab_mode     = CAMERA_GRAB_LATEST,
    .sccb_i2c_port = 0,
};

struct ImuData
{
  float ax;
  float ay;
  float az;

  float gx;
  float gy;
  float gz;
};
ImuData imu_bias;

void send_image()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Capture failed");
      delay(1000);
      return;
  }

  bool complete = frame2jpg(fb, JPEG_QUORITY, &jpeg_buf, &jpeg_len);
  if(!complete)
  {
    Serial.println("Frame to JPEG Failed");
    return;
  }

  uint16_t total_packets = (jpeg_len + PACKET_SIZE - 1) / PACKET_SIZE;

  for (uint16_t i = 0; i < total_packets; i++) {
    int offset = i * PACKET_SIZE;
    int fb_size = jpeg_len - offset;
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
    udp_image.write(jpeg_buf + offset, chunk);

    udp_image.endPacket();
    delayMicroseconds(100);
  }

  frame_id++;
  esp_camera_fb_return(fb);
  fb = NULL;
  free(jpeg_buf);
  jpeg_buf = NULL;
}

void send_imu()
{
  ImuData pkt;

  auto imu_update = M5.Imu.update();
  if(!imu_update)
  {
    Serial.println("Failed to get Imu Data.");
    return;
  }

  auto data = M5.Imu.getImuData();

  pkt.ax = data.accel.x;
  pkt.ay = data.accel.y;
  pkt.az = data.accel.z;

  pkt.gx = data.gyro.x - imu_bias.gx;
  pkt.gy = data.gyro.y - imu_bias.gy;
  pkt.gz = data.gyro.z - imu_bias.gz;

  udp_imu.beginPacket(HOST, IMU_PORT);

  udp_imu.write(
    (uint8_t*)&pkt,
    sizeof(pkt));

  udp_imu.endPacket();
  Serial.printf("ax:%f  ay:%f  az:%f\r\n", data.accel.x, data.accel.y, data.accel.z);
  Serial.printf("gx:%f  gy:%f  gz:%f\r\n", data.gyro.x, data.gyro.y, data.gyro.z);
}

void calibrate_gyro()
{
  Serial.println("Start Gyro-Caribration");

  const int N = 100;
  float sum_x = 0;
  float sum_y = 0;
  float sum_z = 0;

  for(int i=0; i<N; i++)
  {
    bool updated = M5.Imu.update();
    auto data = M5.Imu.getImuData();

    sum_x += data.gyro.x;
    sum_y += data.gyro.y;
    sum_z += data.gyro.z;
    delay(10);
  }

  imu_bias.gx = sum_x/N;
  imu_bias.gy = sum_y/N;
  imu_bias.gz = sum_z/N;

  Serial.printf("Gyro Bias : %.5f %.5f %.5f\n", imu_bias.gx, imu_bias.gy, imu_bias.gz);
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

  pinMode(POWER_GPIO_NUM, OUTPUT);
  digitalWrite(POWER_GPIO_NUM, LOW);
  delay(500);

  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera Init Fail: 0x%x\n", err);
    while(true) delay(1000);
  }
  Serial.println("Camera Init Success");

  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  M5.Imu.begin();
  Serial.printf("IMU Enabled = %d\n", M5.Imu.isEnabled());
  Serial.println("M5 Init Success");

  udp_image.begin(IMAGE_PORT);
  udp_imu.begin(IMU_PORT);

  calibrate_gyro();
}


void loop() {
  send_image();
  send_imu();

  delay(10);
}
