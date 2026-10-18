/*
  LavaLoucas.ino  -  Controle de lava-louças antiga 110 V com ESP32 + módulo de 4 relés
  ------------------------------------------------------------------------------------
  Placa: "ESP32 Dev Module" (ESP32 DevKit V1 de 30 pinos, core esp32 da Espressif 3.x).
  Bibliotecas: todas já vêm com o core (WiFi, WebServer, ESPmDNS, Preferences).

  Credenciais: copie secrets.example.h para secrets.h (mesma pasta) e preencha.
  O secrets.h fica fora do git (.gitignore).

  Onde está cada coisa:
    config.h                 pinos e ajustes de hardware
    src/hardware/inputs      sensores (porta, termostato da cuba, Klixon)
    src/hardware/relays      ligar/desligar as cargas e teste manual
    src/control/safety       travas de segurança, porta e falha do Klixon
    src/control/cycle        etapas do ciclo de lavagem
    src/control/water        água na cuba (estimada por tempo)
    src/storage/settings     configuração do ciclo salva na flash
    src/storage/event_log    registro de eventos
    src/web/network          Wi-Fi e mDNS (lavaloucas.local)
    src/web/web_server       API HTTP
    src/web/page.h           página web

  LEIA O ARQUIVO LIGACAO.md ANTES DE LIGAR QUALQUER COISA NA TOMADA.
*/

#include "config.h"
#include "src/hardware/inputs.h"
#include "src/hardware/relays.h"
#include "src/control/safety.h"
#include "src/control/cycle.h"
#include "src/control/water.h"
#include "src/storage/settings.h"
#include "src/storage/event_log.h"
#include "src/web/network.h"
#include "src/web/web_server.h"

void setup() {
  setupRelays();  // primeiro de tudo: relés desligados

  Serial.begin(115200);
  delay(200);

  setupInputs();
  setupSafety();
  loadSettings();
  addLog("Iniciado");

  setupNetwork();
  setupWebServer();
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  uint32_t dt = now - last;
  last = now;

  // 1. Ler os sensores e reagir a eventos de segurança
  updateInputs(now);
  handleDoorChange(now);
  checkOverTemperature();

  // 2. Decidir o que ligar: o ciclo OU o teste manual da página
  clearRequests();
  if (cycleActive()) runCycle(dt);
  else               requestTestOutputs(now);

  // 3. Barrar o que for perigoso e escrever nos relés
  applySafetyInterlocks();
  updateRelays();
  updateWaterEstimate(dt);

  // 4. Atender a página web
  handleWebClients();
  delay(2);
}
