#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

#define WIFI_SSID       "WLAN Kabel"
#define WIFI_PASSWORD   "57002120109202250682"

//#define SERVER_HOSTNAME "192.168.178.62" // dockerpi
#define SERVER_HOSTNAME "linbook"
#define SERVER_PATHNAME "/api/image/upload"
#define SERVER_PORT     30100

// cycle time in seconds to upload an image
#define IMG_UPLOAD_DELAY_SEC 120


// Pin definition for CAMERA_MODEL_AI_THINKER
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

//LED setup 
#define LED_BUILTIN        4
#define FLASH          true

#define LED_DIM_FREQ    5000
#define LED_CHANNEL        1
#define LED_DIM_RES        8
#define LED_BRIGHTNESS     2

WiFiClient client;

/**
 * Arduino Settings:
 *   Boards           -> ESP32 Wrover Module
 *   Partition Scheme -> Huge App (3MB No OTA/1MB SPIFFS)
 */
void setup()
{
  Serial.begin(115200);

  /* ------------------- -------------------------- */
  pinMode(LED_BUILTIN, OUTPUT);
  ledcSetup(LED_CHANNEL, LED_DIM_FREQ, LED_DIM_RES);
  ledcAttachPin(LED_BUILTIN, LED_CHANNEL);

  /* ------------------- WIFI --------------------- */
  boolean isWifiConnected = connectWifi();
  if(!isWifiConnected) return;

  /* ---------------- CAMERA ---------------------- */
  boolean isCamConnected = connectCamera();
  if(!isCamConnected) return;
}

void loop()
{
  Serial.println("------------------------------------------------");
  
  /*****************************************************************
   * check Wifi and try to reconnect in case it's not connected 
   ******************************************************************/
  while(!isWifiConnected()) {
    // reconnect
    connectWifi();
    delay(10 * 1000); // 10 seconds
  }
  
  /*****************************************************************
   * take and upload image  
   ******************************************************************/
  // takeImage();
  camera_fb_t * frameBuffer = takeImage();
  uploadImage(frameBuffer);

  
  delay(IMG_UPLOAD_DELAY_SEC * 1000);   // take a picture every ... seconds
}

/**
 * connect to wlan. Return the connect status
 */
boolean connectWifi() {
  Serial.print("connecting to '"); Serial.print(WIFI_SSID); Serial.println("'");
  Serial.print("WiFi init...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long millisInTenSeconds = millis() + 10000;
  while (WiFi.status() != WL_CONNECTED && (millis() < millisInTenSeconds) ) {
    Serial.print(".");
    delay(1000);
  }

  if(isWifiConnected()) {
    Serial.print("connected to "); Serial.print(WIFI_SSID); Serial.print("; IP="); + Serial.println(WiFi.localIP());
  } else {
    Serial.println("connection failed");
  }

  return isWifiConnected();
}

/**
 * return the connect status
 */
boolean isWifiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

boolean connectCamera() {
  Serial.print("Camera init...");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("error 0x%x", err);
  } else {
    Serial.println("connected");
  }

  return err == ESP_OK;
}

/**
 * Taking a picture from camera and returning the buffer (binary JPEG)
 */
camera_fb_t * takeImage() {
  if(FLASH == true) {
    Serial.println("Turn on flash");
    ledcWrite(LED_CHANNEL, LED_BRIGHTNESS);
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }

  Serial.print("Capturing image...");
  camera_fb_t * frameBuffer = NULL;

  // Take Picture with Camera
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 10);
  frameBuffer = esp_camera_fb_get();
  
  Serial.println("done");

  if(FLASH == true) {
    Serial.println("Turn off flash");
    ledcWrite(LED_CHANNEL, 0);
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (!frameBuffer) {
    Serial.println("failed!");
    return NULL;
  } else {
    Serial.println("nice foto");
  }

  return frameBuffer;
}

/**
 * Uploading an image to a webserver receiving a POST request and the binary JPEG
 */
void uploadImage(camera_fb_t * frameBuffer) {
  String getAll;
  String getBody;

  /* ---------- connect to server ---------------- */
  Serial.print("Connect server (");Serial.print(SERVER_HOSTNAME);Serial.print(":");Serial.print(SERVER_PORT);Serial.print(") ...");
  if (!client.connect(SERVER_HOSTNAME, SERVER_PORT)) {
    Serial.println("failed!");
    return;
  }
  Serial.println("ok");


  /* ---------- uploading image ---------------- */
  Serial.print("Uploading image...");
  uint32_t totalLen = frameBuffer->len; // + head.length() + tail.length();

  client.println("POST " + String(SERVER_PATHNAME) + " HTTP/1.1");
  client.println("Host: " + String(SERVER_HOSTNAME));
  client.println("Content-Length: " + String(totalLen));
  client.println("Content-Type: image/jpeg");
  client.println();

  uint8_t *fbBuf = frameBuffer->buf;
  size_t fbLen = frameBuffer->len;
  for (size_t n = 0; n < fbLen; n = n + 1024) {
    // Serial.print(n); Serial.print(" -x ");Serial.println(fbLen);
    if (n + 1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    } else if (fbLen % 1024 > 0) {
      size_t remainder = fbLen % 1024;
      client.write(fbBuf, remainder);
    }
  }
  Serial.print("ok (");Serial.print(fbLen);Serial.println(" B buffer sent)");

  esp_camera_fb_return(frameBuffer);

  /* ---------- fetching response of server ---------------- */
  int timoutTimer = 10000;
  long startTimer = millis();
  boolean state = false;

  while ((startTimer + timoutTimer) > millis()) {
    delay(100);
    while (client.available()) {
      char c = client.read();
      if (c == '\n') {
        if (getAll.length() == 0) {
          state = true;
        }
        getAll = "";
      } else if (c != '\r') {
        getAll += String(c);
      }
      if (state == true) {
        getBody += String(c);
      }
      startTimer = millis();
    }
    if (getBody.length() > 0) {
      break;
    }
  }

  client.stop();
  Serial.println("Result:" + String(getBody));
}
