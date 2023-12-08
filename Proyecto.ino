/*
  Universidad Nacional Autónoma de México
  Facultad de Ciencias
  Licenciatura en Ciencias de la Computación
  Seminario de Ciencias de la Computación A: Introducción al Internet de las Cosas

  Proyecto Bot Guardian IOT ESP32CAM with sensor PIR and LDR
  @autor: Pedro Méndez José Manuel 
  contact me: jose_manuel@ciencias.unam.mx
*/

//librerias ocupadas
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Credenciales de nuestro wifi
const char* ssid = "tu info va aqui";
const char* password = "tu info va aqui";

// Credenciales para la inicialización de Telegram BOT (token y chatId)
String BOTtoken = "tu info va aqui";  
String CHAT_ID = "tu info va aqui";

#define PIR_SENSOR_PIN 13
int STATE_PIR = 0; // we asume that the sensor it isnt detect a movement

#define LDR_PIN 12  // LDR PIN
int VALOR_LDR = 0; // we asume that the sensor it isnt detect a movement

bool sendPhoto = false;
// pines utilizados por la camara ESP32-CAM.
#define FLASH_LED_PIN 4
bool FlashState = LOW;

// Sensor de movimiento (to implement)
bool MotionDetected = false;
 bool MotionState = false;

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
// La última vez que se realizo el escaneo de mensajes.
unsigned long lastTimeBotRan;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

//CAMERA_MODEL_AI_THINKER
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

// Indicates when motion is detected
static void IRAM_ATTR detectsMovement(void * arg){
  Serial.println("MOTION DETECTED!!!");
  MotionDetected = true;
}

void configInitCamera(){
  // Configuracion de la camara
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
  //config.grab_mode = CAMERA_GRAB_LATEST;

  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(10);
  digitalWrite(PWDN_GPIO_NUM, HIGH);
  delay(10);
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("La inicializacion fallo: error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  Serial.printf("Camera Initialized >> OK \r\n");
}

// manejo de mensajes de telegram
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);
  // Procesar mensajes
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    // Verificamos que sea un usuario valido
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String fromName = bot.messages[i].from_name;
    String text = bot.messages[i].text;
    Serial.println(numNewMessages + " From " + fromName + " > " + text + " request.");
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      long rssi = WiFi.RSSI() + 100;
      String welcome = "Bienvenido " + from_name + " al bot de Jose: Ntory ESP32-CAM bot.\n";
       welcome += "Fuerza de la señal: " + String(rssi) + "\n";
      welcome += "Puedes ocupar alguno de los siguientes comandos: \n";
      welcome += "/photo : Nos permite tomar una foto.\n\n";
      welcome += "/flash : Cambia el estado del flash.\n";
      if (FlashState == true) welcome += "---> Flash : *ON now*\n"; else welcome += "---> Flash : *OFF now*\nAunque tenemos la función especial de prender un led de acuerdo a la luz del cuarto *automaticamente*!!\n\n";
      welcome += "/motion : Activa el sensor de movimiento.\n";
      if (MotionState == true) welcome += "---> Sensor de movimiento: *ON now*\n"; else welcome += "---> Sensor de movimiento: *OFF now*\n";
      welcome += "Si el sensor de movimiento está activo, recibirás una foto cada vez que se detecte movimiento como evidencia.\n";
      // Enviamos un mensaje al bot con el id, el mensaje y el modo de parseo
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
    }
    if (text == "/flash") {
      FlashState = !FlashState;
      digitalWrite(FLASH_LED_PIN, FlashState);
      Serial.print("FlashState = ");
      if (FlashState == true) Serial.println("ON"); else Serial.println("OFF");
      Serial.println("Change flash LED state");
      String welcome = "Flash:";
      if (FlashState == true) welcome += " *ON now*\n"; else welcome += " *OFF now*\n";
      // Enviamos un mensaje al bot con el id, el mensaje y el modo de parseo
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
    }
// digitalWrite(FLASH_LED_PIN, FlashState);

    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if (text == "/motion") {
      MotionState = !MotionState;
      Serial.print("MotionState = ");
      String welcome = "Sensor de movimiento:";
      if (MotionState == true) welcome += " *ON now*\n"; else welcome += " *OFF now*\n";
      // Enviamos un mensaje al bot con el id, el mensaje y el modo de parseo
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
   }
  }
}

/*
 * La funcion sendPhotoTelegram captura una foto 
 * con la camara y la envaa a traves de Telegram.
*/
String sendPhotoTelegram(camera_fb_t *fb) {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";
  Serial.println("Connect to " + String(myDomain) + " before take a photo");

  if (clientTCP.connect(myDomain, 443)) {
     // preparación y envio de la foto
    Serial.println("Connection successful");
    
    String head = "--NtoryCamBotTutorial\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + 
    "\r\n--NtoryCamBotTutorial\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--NtoryCamBotTutorial--\r\n";

    size_t imageLen = fb->len;
    size_t extraLen = head.length() + tail.length();
    size_t totalLen = imageLen + extraLen;
    
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=NtoryCamBotTutorial");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    esp_camera_fb_return(fb);
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println("Photo Sent and package:");
    //bot.sendMessage(CHAT_ID, "Foto *adjuntada*:", "Markdown");
    Serial.println(head);
    Serial.println(tail);
    Serial.println(getBody);
    esp_camera_fb_return(fb);
  }
  else {
    // coneccion no exitosa
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  Serial.println("body: "+getBody);
  return getBody;
}

bool isFirstCapture = true;
int umbral = 500;  // Podemos cambiar el umbral

/*
 * Se realiza la configuración inicial del dispositivo, incluyendo la configuración de la cámara, la conexión Wi-Fi y la inicialización del monitor serial.
*/
void setup(){
    
  pinMode(LDR_PIN, INPUT);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  Serial.begin(115200);
  delay(500);
  // Set PIR as input 
  pinMode(PIR_SENSOR_PIN, INPUT);
  //delay(500); // To dont generate movement detected fake

  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW); // Turn SENSOR LED Off

  //digitalWrite(FLASH_LED_PIN, FlashState);
  Serial.println("\nESP Cam using Telegram Bot");
  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  // Agrega certificado raíz para api.telegram.org
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
   Serial.println(" >> CONNECTED");
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  // Print the Signal Strength:
  long rssi = WiFi.RSSI() + 100;
  Serial.print("Signal Strength = " + String(rssi));
  if (rssi > 50) {
  Serial.println(F(" (>50 – Good)"));
  } else {
  Serial.println(F(" (Could be Better)"));
  }
  delay(10000); // To dont generate movement detected fake
  // Captura inicial solo en la primera ejecución
  Serial.printf("Type /start in Telegram to start bot\r\n");
  Serial.print("umbral del led para prender automaticamente: "+String(umbral)+ ".\n");
  bot.sendMessage(CHAT_ID, "Envia /start para empezar a usar el *bot*!!!", ",Markdown");
}

// El bucle principal maneja la lógica del programa, verificando si se debe enviar una foto y procesando mensajes de Telegram.
void loop() {
  if (isFirstCapture){
    //Serial.println("Preparing photo");
    //Dispose first and second picture because of bad quality, the sensor has not adjusted the white balance yet
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb); // dispose the buffered image
    isFirstCapture = false;
    Serial.println("Primera y segunda fotos ignoradas por el ajuste de blancos");
    }
  if (sendPhoto) {
    Serial.println("Preparing photo by sendPhoto");// Logica para encender/apagar el flash basado en el valor de la fotoresistencia
    int ldrValue = analogRead(LDR_PIN);
    if (ldrValue < umbral) {  // podemos ajustar el umbral
      // Enciende el flash solo si no hay movimiento
      if (FlashState == LOW) {
        digitalWrite(FLASH_LED_PIN, HIGH);
        FlashState = HIGH;
        }
    }
    // Capturar una nueva foto
    camera_fb_t * fbP = NULL;
    fbP = esp_camera_fb_get();
    
    // Apaga el flash
    digitalWrite(FLASH_LED_PIN, LOW);
    FlashState = LOW;
    if(!fbP) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
      }
    bot.sendMessage(CHAT_ID, "Foto *adjuntada* como evidencia:", "Markdown");
    sendPhotoTelegram(fbP);
    sendPhoto = false;
  }

  // Verificar-leer el estado del sensor PIR
  MotionDetected  = digitalRead(PIR_SENSOR_PIN);
  
  if (MotionDetected  && !sendPhoto) {
    // Movimiento detectado
    Serial.println("Motion detected! Pero no lo mandamos by telegram");
      
      if (MotionState) {
        bot.sendMessage(CHAT_ID, "Movimiento *Detectado*!!!", ",Markdown");
        sendPhoto = true;
        } 
      MotionDetected = false;
      // Add a delay to avoid triggering multiple times in quick succession
      delay(5000);
    }
  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
