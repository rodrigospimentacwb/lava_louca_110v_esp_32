// cycle.h  -  ciclo de lavagem programado.
//
//   [drenagem inicial] -> enchimento -> lavagem -> drenagem
//   -> enxágues (enche, circula, drena) x N -> fim
#pragma once

#include <Arduino.h>

enum Step {
  STEP_IDLE, STEP_PRE_DRAIN, STEP_FILL, STEP_WASH, STEP_DRAIN,
  STEP_RINSE_FILL, STEP_RINSE, STEP_RINSE_DRAIN
};
extern const char* const STEP_NAME[];

extern Step     currentStep;
extern uint32_t stepElapsedMs;
extern uint8_t  rinseIndex;       // enxágue atual (0 = primeiro)
extern bool     washTempReached;  // termostato da cuba fechou na lavagem

bool cycleActive();
bool cyclePaused();           // porta aberta: o ciclo para no lugar
bool isRinseStep();
uint32_t stepTargetMs();      // duração planejada da etapa atual
bool heaterStoppedForStep();  // Klixon ou tempo máximo: sem resistência até a próxima etapa

void runCycle(uint32_t dt);   // chamar a cada volta do loop() com o ciclo ativo
void startCycle();
void stopCycle(const char* why);
