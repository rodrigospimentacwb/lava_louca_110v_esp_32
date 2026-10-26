// network.cpp  -  Wi-Fi (AP + rede de casa) e mDNS.
//
// Atenção: com a rede de casa conectada, o AP passa a usar o mesmo canal do roteador.
// Se o celular estiver no AP na hora em que a placa conecta em casa, ele pode cair e voltar.
// O mDNS (lavaloucas.local) só funciona para quem está na mesma rede da placa: não atravessa roteador.

#include "network.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include "../../config.h"
#include "../../secrets.h"  // WIFI_SSID, WIFI_PASS, AP_SSID, AP_PASS (fora do git)
#include "../storage/event_log.h"

void setupNetwork() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);  // Wi-Fi sempre acordado: a placa fica na tomada, e o "cochilo" padrão atrasa as respostas com sinal fraco
  WiFi.setHostname(HOSTNAME);
  WiFi.softAP(AP_SSID, AP_PASS);

  if (strlen(WIFI_SSID) > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(250);
    if (WiFi.status() == WL_CONNECTED) addLog("WiFi conectado: " + WiFi.localIP().toString());
    else addLog("WiFi da casa nao conectou, use o AP");
  }
  addLog("AP: " + String(AP_SSID) + " -> " + WiFi.softAPIP().toString());

  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
}

String networkAddresses() {
  String home = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() + " / " : String("");
  return home + WiFi.softAPIP().toString();
}

// ---------- Senha de atualização do programa ----------
// OTA_PASS fica no secrets.h, que só este arquivo inclui. Sem ela, atualizar pela página fica desligado.

bool updateEnabled() {
#ifdef OTA_PASS
  return strlen(OTA_PASS) >= 8;
#else
  return false;
#endif
}

bool updatePasswordOk(const String& typed) {
#ifdef OTA_PASS
  return updateEnabled() && typed == OTA_PASS;
#else
  (void)typed;
  return false;
#endif
}
