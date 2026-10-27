// config.h  -  tudo o que você pode precisar ajustar no hardware fica aqui.
// Os tempos do ciclo (enchimento, lavagem...) são ajustados pela página web.
#pragma once

#include <Arduino.h>

const char* const HOSTNAME = "lavaloucas";  // http://lavaloucas.local na rede de casa

// ---------- Relés ----------

// Placa: ESP32 DevKit V1, 30 pinos (ESP-WROOM-32). Na placa, "D32" = GPIO32.
//
// Nível que LIGA o relé. O módulo azul de 4 relés com optoacoplador (IN1..IN4, JD-VCC)
// liga com LOW. Teste: se ao ligar a placa todos os relés clicarem ligados, troque para HIGH.
#define RELAY_ACTIVE_LEVEL LOW

// Qual relé faz o quê. A ordem precisa bater com RELAY_PIN e RELAY_NAME.
enum Relay {
  RELAY_HEATER     = 0,  // resistência
  RELAY_WASH_PUMP  = 1,  // bomba de lavagem
  RELAY_DRAIN_PUMP = 2,  // bomba de drenagem
  RELAY_FILL_VALVE = 3,  // solenoide de entrada de água
  RELAY_COUNT      = 4
};

// D32, D33, D25 e D26: lado a lado na fileira de cima da placa e sem papel no boot.
// No boot eles ficam soltos e o módulo (ativo em LOW) entende como desligado.
// Evite para relé: 0, 2, 5, 12, 15 (decidem o boot), 14 (pisca no boot), 34 a 39 (só entrada).
const uint8_t RELAY_PIN[RELAY_COUNT]      = {32, 33, 25, 26};
const char* const RELAY_NAME[RELAY_COUNT] = {"Resistência", "Bomba de lavagem", "Bomba de drenagem", "Solenoide de entrada"};

// Tempo máximo que cada relé fica ligado no teste manual pela página (segurança).
const uint32_t TEST_MAX_MS[RELAY_COUNT] = {
  10UL * 60000,  // resistência: 10 min
  10UL * 60000,  // bomba de lavagem: 10 min
  3UL * 60000,   // bomba de drenagem: 3 min
  3UL * 60000    // solenoide: 3 min
};

// ---------- Sensores ----------
// Contato seco entre o pino e o GND, com pull-up interno (sem resistor).
// D18, D19 e D21: na fileira de baixo, com GND do mesmo lado, aceitam pull-up e não afetam o boot.
// Livres para o futuro (bóia): D22, D23, D27, D13, D4. Os pinos 34, 35, VP (36) e VN (39)
// servem para leitura analógica, mas não têm pull-up interno.

const uint8_t PIN_DOOR            = 18;  // porta fechada = contato fechado = LOW
const uint8_t PIN_TANK_THERMOSTAT = 19;  // termostato da cuba NA: aberto (HIGH) = frio, fecha (LOW) = atingiu a temperatura
const uint8_t PIN_KLIXON          = 21;  // Klixon da resistência NF: fechado (LOW) = ok, abre (HIGH) = resistência quente demais
// O Klixon fica em série com a resistência, no 110 V (corta sozinho, sem depender do software),
// e o D21 fica livre. true = Klixon ligado no D21 em vez disso (aí ele só desliga a resistência).
const bool    KLIXON_ON_ESP       = false;

// Debounce: o sinal precisa ficar estável por este tempo para valer.
const uint16_t DOOR_DEBOUNCE_MS       = 80;
const uint16_t THERMOSTAT_DEBOUNCE_MS = 300;
const uint16_t KLIXON_DEBOUNCE_MS     = 80;

// Depois de fechar a porta, espera este tempo antes de retomar o ciclo.
const uint32_t DOOR_RESUME_DELAY_MS = 2000;
