// safety.h  -  travas de segurança, porta e falha do Klixon.
// Tudo o que pode barrar um relé está aqui.
#pragma once

#include <Arduino.h>

extern bool   fault;      // Klixon disparou; só sai com resetFault()
extern String faultMsg;

void setupSafety();                   // no boot, depois de setupInputs()
void applySafetyInterlocks();         // entre os pedidos e updateRelays()
void handleDoorChange(uint32_t now);  // porta abriu: para os testes; fechou: marca a hora
bool doorClosedLongEnough();          // fechada há DOOR_RESUME_DELAY_MS (ciclo pode retomar)
void checkOverTemperature();          // Klixon fechou: desliga tudo e entra em falha
void resetFault();                    // só aceita com o Klixon aberto de novo
