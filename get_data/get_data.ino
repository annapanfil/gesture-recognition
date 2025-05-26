#include "esp_camera.h" //from ESP32 Camera
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char *ssid = "Lariat";
const char *password = "lariat2019";
const char* serverUrl = "http://10.2.119.172:4000/upload";

#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// 4 for flash led or 33 for back led
#define LED_GPIO_NUM 4


void initCamera(){
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
  config.frame_size = FRAMESIZE_UXGA;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST; //CAMERA_GRAB_WHEN_EMPTY;
  config.jpeg_quality = 12;
  config.fb_count = 1;


  if (esp_camera_init(&config) != ESP_OK) {
      Serial.println("Błąd inicjalizacji kamery");
      return;
  }

  // fix upside down image
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);

  Serial.println("Kamera zainicjalizowana");
}


void setup() {
  Serial.begin(9600);
  initCamera();
  
  // Połączenie z Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Pobierz obraz z kamery
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Wysyłanie obrazu do serwera
  String bodyStart = "------ESP32Boundary\r\nContent-Disposition: form-data; name=\"file\"; filename=\"photo.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String bodyEnd = "\r\n------ESP32Boundary--\r\n";

  int totalLen = bodyStart.length() + fb->len + bodyEnd.length();
  uint8_t *postData = (uint8_t *)malloc(totalLen);
  memcpy(postData, bodyStart.c_str(), bodyStart.length());
  memcpy(postData + bodyStart.length(), fb->buf, fb->len);
  memcpy(postData + bodyStart.length() + fb->len, bodyEnd.c_str(), bodyEnd.length());

  HTTPClient http;
  http.begin("http://10.2.119.172:4000/upload");
  http.addHeader("Content-Type", "multipart/form-data; boundary=----ESP32Boundary");
  int httpCode = http.POST(postData, totalLen);

  if (httpCode < 0 ){
    Serial.println("Cannot connect to the server, make sure it's running.");
  } else {
    Serial.printf("Response: %d\n", httpCode);
    Serial.println(http.getString());
  }

  free(postData);

  http.end();

  // Zwolnij pamięć po zdjęciu
  esp_camera_fb_return(fb);

  // Poczekaj przed zrobieniem kolejnego zdjęcia
  delay(2000);  // 2 sekundy
}
