#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>
#include <vector>

#define ESPNOW_WIFI_CHANNEL 1

class ESP_NOW_Peer_Class : public ESP_NOW_Peer {
public:
  ESP_NOW_Peer_Class(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk)
    : ESP_NOW_Peer(mac_addr, channel, iface, lmk) {}

  ~ESP_NOW_Peer_Class() {}

  bool add_peer() {
    if (!add()) {
      log_e("Falha ao registrar o peer de broadcast");
      return false;
    }
    return true;
  }

  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    Serial.printf("Recebeu uma mensagem do master " MACSTR " (%s)\n", MAC2STR(addr()), broadcast ? "broadcast" : "unicast");
    Serial.printf("  Mensagem: %s\n", (char *)data);
  }
};

std::vector<ESP_NOW_Peer_Class> masters;

void register_new_master(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) == 0) {
    Serial.printf("Peer desconhecido " MACSTR " enviou uma mensagem de broadcast\n", MAC2STR(info->src_addr));
    Serial.printf("  Mensagem: %s\n", (char *)data);
    Serial.println("Registrando o peer como master");

    ESP_NOW_Peer_Class new_master(info->src_addr, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

    masters.push_back(new_master);
    if (!masters.back().add_peer()) {
      Serial.println("Falha ao registrar o novo master");
      return;
    }
  } else {
    log_v("Recebeu uma mensagem unicast de " MACSTR, MAC2STR(info->src_addr));
    log_v("Ignorando a mensagem");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW - Slave Broadcast - ESP32");
  Serial.println("Parâmetros Wi-Fi:");
  Serial.println("  Modo: STA");
  Serial.println("  Endereço MAC: " + WiFi.macAddress());
  Serial.printf("  Canal: %d\n", ESPNOW_WIFI_CHANNEL);

  if (!ESP_NOW.begin()) {
    Serial.println("Falha ao inicializar o ESP-NOW");
    Serial.println("Reiniciando em 5 segundos...");
    delay(5000);
    ESP.restart();
  }

  ESP_NOW.onNewPeer(register_new_master, NULL);

  Serial.println("Configuração completa. Aguardando o master transmitir uma mensagem...");
}

void loop() {
  delay(1000);
}
