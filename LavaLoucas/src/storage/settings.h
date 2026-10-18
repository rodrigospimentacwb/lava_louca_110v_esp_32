// settings.h  -  configuração do ciclo, ajustável pela página e salva na memória flash (Preferences).
#pragma once

#include <Arduino.h>

struct CycleSettings {
  uint16_t fillSec    = 60;   // tempo de solenoide aberto por enchimento
  uint16_t drainSec   = 60;   // tempo de drenagem
  uint16_t washMin    = 20;   // tempo mínimo de lavagem
  uint16_t heatMaxMin = 40;   // tempo máximo esperando o termostato da cuba abrir
  uint16_t rinseMin   = 5;    // tempo de cada enxágue
  uint8_t  nRinses    = 2;    // quantidade de enxágues
  bool     rinseHeat  = true; // aquece o último enxágue
  bool     preDrain   = true; // drena antes de começar (tira água parada)
};

extern CycleSettings settings;  // valores em uso

void loadSettings();            // lê da flash (no boot)
void saveSettings();            // grava na flash
void restoreDefaultSettings();  // volta aos padrões acima e grava
