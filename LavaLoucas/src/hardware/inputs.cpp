// inputs.cpp  -  leitura dos sensores com debounce.

#include "inputs.h"
#include "../../config.h"

struct DebouncedInput {
  uint8_t  pin;
  uint16_t debounceMs;
  bool     stable;        // valor que vale (já sem ruído)
  bool     lastRaw;       // última leitura crua
  uint32_t lastChangeMs;  // quando a leitura crua mudou
};

static DebouncedInput door       {PIN_DOOR,            DOOR_DEBOUNCE_MS,       HIGH, HIGH, 0};
static DebouncedInput thermostat {PIN_TANK_THERMOSTAT, THERMOSTAT_DEBOUNCE_MS, HIGH, HIGH, 0};
static DebouncedInput klixon     {PIN_KLIXON,          KLIXON_DEBOUNCE_MS,     HIGH, HIGH, 0};

bool doorClosed()    { return door.stable == LOW; }
bool tankHot()       { return thermostat.stable == LOW; }   // termostato NA fechou
bool klixonTripped() { return KLIXON_ON_ESP && klixon.stable == HIGH; }  // Klixon NF abriu (só se ligado no D21)

static void setupInput(DebouncedInput& input) {
  pinMode(input.pin, INPUT_PULLUP);
  delay(2);
  input.stable = input.lastRaw = digitalRead(input.pin);
  input.lastChangeMs = millis();
}

// A leitura só vale depois de ficar igual por debounceMs.
static void updateInput(DebouncedInput& input, uint32_t now) {
  bool raw = digitalRead(input.pin);
  if (raw != input.lastRaw) {
    input.lastRaw = raw;
    input.lastChangeMs = now;
  } else if (raw != input.stable && now - input.lastChangeMs >= input.debounceMs) {
    input.stable = raw;
  }
}

void setupInputs() {
  setupInput(door);
  setupInput(thermostat);
  setupInput(klixon);
}

void updateInputs(uint32_t now) {
  updateInput(door, now);
  updateInput(thermostat, now);
  updateInput(klixon, now);
}
