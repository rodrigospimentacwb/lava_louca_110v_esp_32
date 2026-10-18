// settings.cpp  -  configuração do ciclo salva na memória flash (Preferences).
// Continua valendo depois de desligar a placa ou gravar um firmware novo.

#include "settings.h"
#include <Preferences.h>
#include "event_log.h"

CycleSettings settings;
static Preferences prefs;

void loadSettings() {
  prefs.begin("lava", true);  // true = só leitura
  settings.fillSec    = prefs.getUShort("fill",     settings.fillSec);
  settings.drainSec   = prefs.getUShort("drain",    settings.drainSec);
  settings.washMin    = prefs.getUShort("wash",     settings.washMin);
  settings.heatMaxMin = prefs.getUShort("heatMax",  settings.heatMaxMin);
  settings.rinseMin   = prefs.getUShort("rinse",    settings.rinseMin);
  settings.nRinses    = prefs.getUChar ("nRinse",   settings.nRinses);
  settings.rinseHeat  = prefs.getBool  ("rHeat",    settings.rinseHeat);
  settings.preDrain   = prefs.getBool  ("preDrain", settings.preDrain);
  prefs.end();
}

void saveSettings() {
  prefs.begin("lava", false);
  prefs.putUShort("fill",     settings.fillSec);
  prefs.putUShort("drain",    settings.drainSec);
  prefs.putUShort("wash",     settings.washMin);
  prefs.putUShort("heatMax",  settings.heatMaxMin);
  prefs.putUShort("rinse",    settings.rinseMin);
  prefs.putUChar ("nRinse",   settings.nRinses);
  prefs.putBool  ("rHeat",    settings.rinseHeat);
  prefs.putBool  ("preDrain", settings.preDrain);
  prefs.end();
  addLog("Configuracao salva");
}

void restoreDefaultSettings() {
  settings = CycleSettings();
  saveSettings();
  addLog("Configuracao voltou aos padroes");
}
