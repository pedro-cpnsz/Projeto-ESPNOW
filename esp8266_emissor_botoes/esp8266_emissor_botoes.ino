#include <ESP8266WiFi.h>
#include <espnow.h>

#define ESPNOW_WIFI_CHANNEL 1

uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  //Genérico
// uint8_t broadcastAddress[] = {0x24, 0x0A, 0xC4, 0x0B, 0x52, 0x64}; //ESP32 Riscado
// uint8_t broadcastAddress[] = {0x24, 0x0A, 0xC4, 0x08, 0xD3, 0xA0};

typedef struct struct_message {
  String m;
} struct_message;

struct_message data;

void OnDataSent(uint8_t *macAddr, uint8_t sendStatus) {
  Serial.print("Envio de dados para ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X ", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.println(sendStatus == 0 ? "com sucesso" : "falhou");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("ESP-NOW Broadcast - ESP8266");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  if (esp_now_init() != 0) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, ESPNOW_WIFI_CHANNEL, NULL, 0) != 0) {
    Serial.println("Erro ao adicionar o peer de broadcast");
    return;
  }

}

void loop() {

  if (digitalRead(0) == LOW) {
    Serial.println("Botao 1 pressionado!");
    data.m = "1";
    esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
    delay(500);
  }

  if (digitalRead(2) == LOW) {
    Serial.println("Botao 2 pressionado!");
    data.m = "2";
    esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
    delay(500);
  }

}
