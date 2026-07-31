#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

//================ WiFi ==================
const char* SSID = "ssid";
const char* PASSWORD = "password";
const char* HOST = "ipadress";

const int IMAGE_PORT = 9000;
const int CMD_PORT   = 9001;

WiFiUDP udp_image;
WiFiUDP udp_cmd_vel;


// CAMERA =================================
#define PACKET_SIZE 1400
uint16_t frame_id = 0;

camera_config_t camera_config;

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

bool setupCamera()
{
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;

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
    camera_config.frame_size = FRAMESIZE_QVGA;
    camera_config.jpeg_quality = 10;
    camera_config.fb_count = 2;
    camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    if (esp_camera_init(&camera_config) != ESP_OK)
        return false;

    return true;
}

// WHEEL CONTROLL ========================================
Servo leftServo;
Servo rightServo;

const int LEFT_SERVO_PIN = 13;
const int RIGHT_SERVO_PIN = 14;

const int SERVO_MIN = 900;
const int SERVO_STOP = 1500;
const int SERVO_MAX = 2100;
const int SERVO_RANGE = 600;

const float MAX_LINEAR_SPEED = 0.40f;      // m/s
const float ROTATION_GAIN = 0.10f;         // 回転→左右速度変換係数

struct VelocityCommand
{
    float linear;
    float angular;
};

VelocityCommand pre_cmd_;

void setMotorSpeed(float linear, float angular)
{
    float left = linear - angular * ROTATION_GAIN;
    float right = linear + angular * ROTATION_GAIN;

    left /= MAX_LINEAR_SPEED;
    right /= MAX_LINEAR_SPEED;

    left = constrain(left, -1.0f, 1.0f);
    right = constrain(right, -1.0f, 1.0f);

    int leftPulse = SERVO_STOP + left * SERVO_RANGE;
    int rightPulse = SERVO_STOP - right * SERVO_RANGE;

    leftPulse = constrain(leftPulse, SERVO_MIN, SERVO_MAX);
    rightPulse = constrain(rightPulse, SERVO_MIN, SERVO_MAX);

    leftServo.writeMicroseconds(leftPulse);
    rightServo.writeMicroseconds(rightPulse);

    Serial.print("leftPulse: ");
    Serial.print(leftPulse);
    Serial.print(", rightPulse: ");
    Serial.print(rightPulse);
    Serial.println();
}

void command_task(void *arg)
{
    VelocityCommand cmd;

    while (true)
    {
        int size = udp_cmd_vel.parsePacket();

        if (size == sizeof(cmd))
        {
            udp_cmd_vel.read((uint8_t *)&cmd, sizeof(cmd));
            if(cmd.linear != pre_cmd_.linear || cmd.angular != pre_cmd_.angular)
            {
                pre_cmd_.linear = cmd.linear;
                pre_cmd_.angular = cmd.angular;
                setMotorSpeed(cmd.linear, cmd.angular);
            }
        }

        vTaskDelay(1);
    }
}

// IMAGE PROCESSING ========================================

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
        udp_image.write((uint8_t *)&hdr, sizeof(hdr));
        udp_image.write(fb->buf + offset, chunk);
        udp_image.endPacket();
    }

    esp_camera_fb_return(fb);

    frame_id++;
}

void image_task(void *arg)
{
    while (true)
    {
        send_image();
    }
}

//========================================

void setup()
{
    Serial.begin(115200);

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.println();
    Serial.println("WiFi connected");

    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    if (!setupCamera())
    {
        while (true);
    }
    Serial.println();
    Serial.println("Camera connected");

    udp_image.begin(IMAGE_PORT);
    udp_cmd_vel.begin(CMD_PORT);

    pre_cmd_.linear  = 0.0;
    pre_cmd_.angular = 0.0;

    leftServo.setPeriodHertz(50);
    rightServo.setPeriodHertz(50);

    leftServo.attach(LEFT_SERVO_PIN, SERVO_MIN, SERVO_MAX);
    rightServo.attach(RIGHT_SERVO_PIN, SERVO_MIN, SERVO_MAX);

    leftServo.writeMicroseconds(SERVO_STOP);
    rightServo.writeMicroseconds(SERVO_STOP);

    xTaskCreatePinnedToCore(
        image_task, "image_task", 8192, NULL, 1, NULL, 1);

    xTaskCreatePinnedToCore(
        command_task, "command_task", 4096, NULL, 1, NULL, 0);
}

//========================================

void loop()
{
    vTaskDelay(portMAX_DELAY);
}