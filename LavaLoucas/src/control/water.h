// water.h  -  água na cuba, estimada por tempo (ainda não tem bóia).
//   Solenoide aberto por 80% do tempo de enchimento = "com água".
//   Drenagem ligada pelo tempo de drenagem          = "vazia".
#pragma once

#include <Arduino.h>

extern bool     waterPresent;
extern bool     waterFromValve; // a água veio do solenoide (e não de "marcar com água" na página)
extern uint32_t fillAccumMs;    // tempo acumulado de solenoide aberto (para calibrar o enchimento)

void updateWaterEstimate(uint32_t dt);  // chamar depois de updateRelays()
void markFilledByValve();               // ciclo terminou de encher
void markDrained();                     // ciclo terminou de drenar
void setWaterManually(bool present);    // botão "marcar com água / vazia" da página (não libera a resistência)
