// safety.cpp  -  travas de segurança, porta e falha do Klixon.

#include "safety.h"
#include "../../config.h"
#include "../hardware/inputs.h"
#include "../hardware/relays.h"
#include "../storage/event_log.h"
#include "cycle.h"
#include "water.h"

bool   fault = false;
String faultMsg;

static bool     lastDoorClosed  = false;
static uint32_t doorClosedSince = 0;

void setupSafety() {
  lastDoorClosed = doorClosed();
  doorClosedSince = millis();
}

// ---------- Travas ----------

// Barra o pedido de um relé e guarda o motivo (aparece na página).
static void blockRelay(int relay, const char* reason) {
  relayRequested[relay] = false;
  blockReason[relay] = reason;
}

static void blockAllRelays(const char* reason) {
  for (int i = 0; i < RELAY_COUNT; i++) blockRelay(i, reason);
}

void applySafetyInterlocks() {
  for (int i = 0; i < RELAY_COUNT; i++) blockReason[i] = "";

  // Encher e drenar ao mesmo tempo não faz sentido: a drenagem ganha.
  if (relayRequested[RELAY_FILL_VALVE] && relayRequested[RELAY_DRAIN_PUMP])
    blockRelay(RELAY_FILL_VALVE, "drenagem ligada");

  // Resistência a seco queima (e é o maior risco de incêndio).
  if (relayRequested[RELAY_HEATER] && !waterPresent)
    blockRelay(RELAY_HEATER, "sem agua");
  // "Com água" marcado à mão não basta: só vale a água que entrou pelo solenoide.
  else if (relayRequested[RELAY_HEATER] && !waterFromValve)
    blockRelay(RELAY_HEATER, "agua so marcada a mao");

  // Termostato da cuba aberto = já chegou na temperatura.
  if (relayRequested[RELAY_HEATER] && tankHot())
    blockRelay(RELAY_HEATER, "cuba quente (termostato aberto)");

  if (!doorClosed()) blockAllRelays("porta aberta");
  if (fault)         blockAllRelays("falha");
}

// ---------- Porta ----------
// Porta aberta: tudo desliga (trava acima) e o ciclo pausa (cyclePaused()).

void handleDoorChange(uint32_t now) {
  bool closed = doorClosed();
  if (closed == lastDoorClosed) return;

  lastDoorClosed = closed;
  if (closed) {
    doorClosedSince = now;
    addLog("Porta fechada");
  } else {
    addLog("Porta ABERTA: tudo desligado");
    stopAllTests();
  }
}

bool doorClosedLongEnough() {
  return doorClosed() && millis() - doorClosedSince >= DOOR_RESUME_DELAY_MS;
}

// ---------- Klixon (sobretemperatura da resistência) ----------

void checkOverTemperature() {
  if (!klixonTripped() || fault) return;

  fault = true;
  faultMsg = "Klixon da resistencia fechou: SOBRETEMPERATURA. Tudo desligado.";
  addLog("FALHA: " + faultMsg);
  stopCycle("falha Klixon");
  forceAllRelaysOff();
}

void resetFault() {
  if (klixonTripped()) {
    addLog("Reset negado: Klixon ainda fechado");
  } else if (fault) {
    fault = false;
    faultMsg = "";
    addLog("Falha resetada");
  }
}
