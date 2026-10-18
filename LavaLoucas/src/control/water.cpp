// water.cpp  -  estimativa de água na cuba pelo tempo de solenoide e de drenagem.

#include "water.h"
#include "../hardware/relays.h"
#include "../storage/settings.h"
#include "../storage/event_log.h"

bool     waterPresent = false;
uint32_t fillAccumMs  = 0;
static uint32_t drainAccumMs = 0;

void updateWaterEstimate(uint32_t dt) {
  if (relayOn[RELAY_FILL_VALVE]) {
    fillAccumMs += dt;
    drainAccumMs = 0;
    if (!waterPresent && fillAccumMs >= settings.fillSec * 800UL) {
      waterPresent = true;
      addLog("Agua: considerada cheia");
    }
  }
  if (relayOn[RELAY_DRAIN_PUMP]) {
    drainAccumMs += dt;
    if (drainAccumMs >= settings.drainSec * 1000UL) {
      if (waterPresent) addLog("Agua: considerada vazia");
      waterPresent = false;
      fillAccumMs = 0;
    }
  }
}

void setWaterManually(bool present) {
  waterPresent = present;
  if (!present) fillAccumMs = 0;
  addLog(present ? "Agua: marcada manualmente como cheia" : "Agua: marcada manualmente como vazia");
}
