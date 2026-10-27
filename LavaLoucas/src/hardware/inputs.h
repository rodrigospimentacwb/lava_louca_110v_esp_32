// inputs.h  -  sensores: porta, termostato da cuba e Klixon (com debounce).
#pragma once

#include <Arduino.h>

void setupInputs();                // pinos com pull-up interno (no boot)
void updateInputs(uint32_t now);   // chamar a cada volta do loop()

// O que cada sensor quer dizer (já sem ruído).
// Fio solto é lido como HIGH: porta "aberta" (tudo desliga) e Klixon "disparado" (resistência
// desliga), as duas falhas seguras. O termostato da cuba é NA: fio solto nele é lido como
// "fria" e NÃO é detectado; quem segura é o Klixon e o tempo máximo de aquecimento.
bool doorClosed();      // porta fechada
bool tankHot();         // termostato da cuba fechou (água atingiu a temperatura)
bool klixonTripped();   // Klixon da resistência abriu (sempre false com KLIXON_ON_ESP = false)
