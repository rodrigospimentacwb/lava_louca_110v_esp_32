// relays.h  -  ligar e desligar as cargas.
//
// Como funciona, a cada volta do loop():
//   1. clearRequests()           zera os pedidos
//   2. o ciclo ou o teste manual chamam turnOnWashPump(), turnOnHeater()...
//   3. applySafetyInterlocks()   barra o que for perigoso (control/safety)
//   4. updateRelays()            escreve nos pinos só o que mudou
//
// As funções turnOn...() NÃO mexem no pino direto. Assim nenhuma parte do código
// consegue pular as travas de segurança.
#pragma once

#include <Arduino.h>
#include "../../config.h"

extern bool     relayRequested[RELAY_COUNT];  // pedidos desta volta do loop
extern bool     relayOn[RELAY_COUNT];         // o que está ligado de fato
extern String   blockReason[RELAY_COUNT];     // por que a trava barrou o pedido
extern bool     testOn[RELAY_COUNT];          // teste manual ligado pela página
extern uint32_t testStartedMs[RELAY_COUNT];

// ---------- Pedidos ----------
void clearRequests();
void turnOnHeater();       void turnOffHeater();
void turnOnWashPump();     void turnOffWashPump();
void turnOnDrainPump();    void turnOffDrainPump();
void turnOnFillValve();    void turnOffFillValve();
void turnEverythingOff();

// ---------- Pinos ----------
void setupRelays();        // no boot: todos desligados antes de virar saída
void updateRelays();       // aplica os pedidos que passaram pelas travas
void forceAllRelaysOff();  // emergência: desliga na hora

// ---------- Teste manual (botões da página) ----------
void startTest(int relay);
void stopTest(int relay);
void stopAllTests();
void requestTestOutputs(uint32_t now);  // desliga sozinho depois de TEST_MAX_MS
uint32_t testRemainingMs(int relay, uint32_t now);
