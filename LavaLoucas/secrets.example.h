// Copie este arquivo para secrets.h (na mesma pasta) e preencha.
// O secrets.h está no .gitignore e não deve ser commitado.
#pragma once

// Sua rede Wi-Fi (deixe "" para usar só a rede própria da placa)
const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

// Rede própria da placa (sempre ligada). Senha com no mínimo 8 caracteres.
const char* AP_SSID   = "LavaLoucas";
const char* AP_PASS   = "troque-esta-senha";

// Senha para atualizar o programa pela página (aba Debug e admin), mínimo 8 caracteres e
// diferente das outras. Comentada = atualização pela página desligada (só pela USB).
// #define OTA_PASS "outra-senha-forte"
