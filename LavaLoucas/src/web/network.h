// network.h  -  Wi-Fi e mDNS.
//   - Rede própria da placa (AP), sempre ligada: http://192.168.4.1
//   - Rede de casa, se WIFI_SSID estiver preenchido no secrets.h: http://lavaloucas.local
#pragma once

#include <Arduino.h>

void setupNetwork();
String networkAddresses();  // "192.168.0.50 / 192.168.4.1" (casa / AP)
