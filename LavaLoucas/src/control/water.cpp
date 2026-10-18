// water.cpp  -  estimativa de água na cuba pelo tempo de solenoide e de drenagem.

#include "water.h"
#include "../hardware/relays.h"
#include "../storage/settings.h"
#include "../storage/event_log.h"

bool     waterPresent   = false;
bool     waterFromValve = false;
uint32_t fillAccumMs    = 0;
static uint32_t drainAccumMs = 0;

void updateWaterEstimate(uint32_t dt) {
  if (relayOn[RELAY_FILL_VALVE]) {
    fillAccumMs += dt;
    drainAccumMs = 0;
    if (!waterFromValve && fillAccumMs >= settings.fillSec * 800UL) {
      waterPresent = true;
      waterFromValve = true;
      addLog("Agua: considerada cheia");
    }
  }
  if (relayOn[RELAY_DRAIN_PUMP]) {
    drainAccumMs += dt;
    if (drainAccumMs >= settings.drainSec * 1000UL) {
      if (waterPresent) addLog("Agua: considerada vazia");
      markDrained();
    }
  }
}

void markFilledByValve() {
  waterPresent = true;
  waterFromValve = true;
}

void markDrained() {
  waterPresent = false;
  waterFromValve = false;
  fillAccumMs = 0;
}

// "Com água" marcado à mão só informa: a resistência continua exigindo água do solenoide.
// Assim um pedido vindo da rede não consegue montar "água + resistência" sem a cuba encher.
void setWaterManually(bool present) {
  if (!present) markDrained();
  else waterPresent = true;
  addLog(present ? "Agua: marcada manualmente como cheia" : "Agua: marcada manualmente como vazia");
}
