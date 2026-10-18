// relays.cpp  -  pedidos, escrita nos pinos e teste manual.

#include "relays.h"
#include "../storage/event_log.h"

bool     relayRequested[RELAY_COUNT] = {};
bool     relayOn[RELAY_COUNT]        = {};
String   blockReason[RELAY_COUNT];
bool     testOn[RELAY_COUNT]         = {};
uint32_t testStartedMs[RELAY_COUNT]  = {};

// ---------- Pedidos ----------

void clearRequests() {
  for (int i = 0; i < RELAY_COUNT; i++) relayRequested[i] = false;
}

void turnOnHeater()       { relayRequested[RELAY_HEATER] = true; }
void turnOffHeater()      { relayRequested[RELAY_HEATER] = false; }

void turnOnWashPump()     { relayRequested[RELAY_WASH_PUMP] = true; }
void turnOffWashPump()    { relayRequested[RELAY_WASH_PUMP] = false; }

void turnOnDrainPump()    { relayRequested[RELAY_DRAIN_PUMP] = true; }
void turnOffDrainPump()   { relayRequested[RELAY_DRAIN_PUMP] = false; }

void turnOnFillValve()    { relayRequested[RELAY_FILL_VALVE] = true; }
void turnOffFillValve()   { relayRequested[RELAY_FILL_VALVE] = false; }

void turnEverythingOff()  { clearRequests(); }

// ---------- Pinos ----------

static void writeRelayPin(int relay, bool on) {
  digitalWrite(RELAY_PIN[relay], on ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
}

// Escreve "desligado" ANTES de o pino virar saída, para nenhum relé dar um pulso no boot.
void setupRelays() {
  for (int i = 0; i < RELAY_COUNT; i++) {
    writeRelayPin(i, false);
    pinMode(RELAY_PIN[i], OUTPUT);
    writeRelayPin(i, false);
  }
}

void updateRelays() {
  for (int i = 0; i < RELAY_COUNT; i++) {
    if (relayRequested[i] != relayOn[i]) {
      relayOn[i] = relayRequested[i];
      writeRelayPin(i, relayOn[i]);
      addLog(String(RELAY_NAME[i]) + (relayOn[i] ? " LIGOU" : " desligou"));
    }
  }
}

void forceAllRelaysOff() {
  for (int i = 0; i < RELAY_COUNT; i++) {
    relayRequested[i] = false;
    relayOn[i] = false;
    testOn[i] = false;
    writeRelayPin(i, false);
  }
}

// ---------- Teste manual ----------

void startTest(int relay) {
  // Solenoide e drenagem nunca juntos: ligar um desliga o teste do outro.
  if (relay == RELAY_FILL_VALVE) testOn[RELAY_DRAIN_PUMP] = false;
  if (relay == RELAY_DRAIN_PUMP) testOn[RELAY_FILL_VALVE] = false;
  testOn[relay] = true;
  testStartedMs[relay] = millis();
  addLog(String("Teste: pedido ligar ") + RELAY_NAME[relay]);
}

void stopTest(int relay) { testOn[relay] = false; }

void stopAllTests() {
  for (int i = 0; i < RELAY_COUNT; i++) testOn[i] = false;
}

void requestTestOutputs(uint32_t now) {
  for (int i = 0; i < RELAY_COUNT; i++) {
    if (testOn[i] && now - testStartedMs[i] >= TEST_MAX_MS[i]) {
      testOn[i] = false;
      addLog(String("Teste: ") + RELAY_NAME[i] + " desligado por tempo maximo");
    }
    if (testOn[i]) relayRequested[i] = true;
  }
}

uint32_t testRemainingMs(int relay, uint32_t now) {
  if (!testOn[relay]) return 0;
  uint32_t elapsed = now - testStartedMs[relay];
  return elapsed >= TEST_MAX_MS[relay] ? 0 : TEST_MAX_MS[relay] - elapsed;
}
