// safety.h  -  travas de segurança, porta e Klixon.
// Tudo o que pode barrar um relé está aqui.
#pragma once

#include <Arduino.h>

void setupSafety();                   // no boot, depois de setupInputs()
void applySafetyInterlocks();         // entre os pedidos e updateRelays()
void handleDoorChange(uint32_t now);  // porta abriu: para os testes; fechou: marca a hora
bool doorClosedLongEnough();          // fechada há DOOR_RESUME_DELAY_MS (ciclo pode retomar)
void handleKlixonChange();            // Klixon disparou ou voltou: só registra (a trava está acima)
