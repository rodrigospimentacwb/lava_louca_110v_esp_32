// safety.cpp  -  travas de segurança, porta e Klixon.

#include "safety.h"
#include "../../config.h"
#include "../hardware/inputs.h"
#include "../hardware/relays.h"
#include "../storage/event_log.h"
#include "cycle.h"
#include "water.h"

static bool     lastDoorClosed  = false;
static uint32_t doorClosedSince = 0;
static bool     lastKlixonTripped = false;

void setupSafety() {
  lastKlixonTripped = klixonTripped();
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
    blockRelay(RELAY_HEATER, "cuba quente (termostato fechado)");

  // Klixon disparou: só a resistência desliga; bombas e o resto do ciclo seguem.
  if (relayRequested[RELAY_HEATER] && klixonTripped())
    blockRelay(RELAY_HEATER, "Klixon disparou (resistencia quente demais)");

  if (!doorClosed()) blockAllRelays("porta aberta");
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

// ---------- Klixon (resistência quente demais) ----------
// A trava acima desliga a resistência enquanto ele estiver aberto, e o ciclo deixa a
// resistência desligada até o fim da etapa (cycle.cpp). Aqui só fica o registro.

void handleKlixonChange() {
  bool tripped = klixonTripped();
  if (tripped == lastKlixonTripped) return;

  lastKlixonTripped = tripped;
  addLog(tripped ? "AVISO: Klixon disparou: resistencia desligada" : "Klixon voltou ao normal");
}
