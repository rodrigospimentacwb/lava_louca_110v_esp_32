// inputs.h  -  sensores: porta, termostato da cuba e Klixon (com debounce).
#pragma once

#include <Arduino.h>

void setupInputs();                // pinos com pull-up interno (no boot)
void updateInputs(uint32_t now);   // chamar a cada volta do loop()

// O que cada sensor quer dizer (já sem ruído).
// Fio solto é lido como HIGH: porta aberta e cuba quente, então tudo desliga (falha segura).
// O Klixon é NA: fio solto nele NÃO é detectado.
bool doorClosed();      // porta fechada
bool tankHot();         // termostato da cuba abriu (água atingiu a temperatura)
bool klixonTripped();   // Klixon da resistência fechou (SOBRETEMPERATURA)
