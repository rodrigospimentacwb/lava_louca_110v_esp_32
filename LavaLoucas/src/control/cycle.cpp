// cycle.cpp  -  etapas do ciclo de lavagem.
//
// Cada etapa tem uma função run...Step(): ela pede os relés que a etapa usa
// e, quando a etapa termina, chama goToStep() com a próxima.
// Não existe secagem com resistência, de propósito: sem água e sem bóia,
// aquecer a seco é o cenário mais perigoso.

#include "cycle.h"
#include "../hardware/inputs.h"
#include "../hardware/relays.h"
#include "../storage/settings.h"
#include "../storage/event_log.h"
#include "safety.h"
#include "water.h"

const char* const STEP_NAME[] = {"Parado", "Drenagem inicial", "Enchendo", "Lavando", "Drenando",
                                 "Enchendo (enxague)", "Enxaguando", "Drenando (enxague)"};

Step     currentStep     = STEP_IDLE;
uint32_t stepElapsedMs   = 0;
uint8_t  rinseIndex      = 0;
bool     washTempReached = false;
static bool heatTimedOut = false;  // passou do tempo máximo esperando aquecer

static void goToStep(Step next);
static void goToNextRinseOrFinish();

bool cycleActive() { return currentStep != STEP_IDLE; }

// Porta aberta (ou fechada há menos de 2 s): o ciclo para no lugar e o tempo não conta.
bool cyclePaused() { return cycleActive() && !doorClosedLongEnough(); }

bool isRinseStep() {
  return currentStep == STEP_RINSE_FILL || currentStep == STEP_RINSE || currentStep == STEP_RINSE_DRAIN;
}

static bool isLastRinse() { return rinseIndex + 1 == settings.nRinses; }

static void goToStep(Step next) {
  currentStep = next;
  stepElapsedMs = 0;
  washTempReached = false;
  heatTimedOut = false;
  if (isRinseStep())
    addLog(String("Etapa: ") + STEP_NAME[next] + " " + (rinseIndex + 1) + "/" + settings.nRinses);
  else
    addLog(String("Etapa: ") + STEP_NAME[next]);
}

// Duração planejada da etapa atual (a lavagem pode passar disso esperando aquecer).
uint32_t stepTargetMs() {
  switch (currentStep) {
    case STEP_PRE_DRAIN:
    case STEP_DRAIN:
    case STEP_RINSE_DRAIN: return settings.drainSec * 1000UL;
    case STEP_FILL:
    case STEP_RINSE_FILL:  return settings.fillSec * 1000UL;
    case STEP_WASH:        return settings.washMin * 60000UL;
    case STEP_RINSE:       return settings.rinseMin * 60000UL;
    default:               return 0;
  }
}

static bool stepTimeIsUp() { return stepElapsedMs >= stepTargetMs(); }

// ---------- Etapas ----------

static void runPreDrainStep() {
  turnOnDrainPump();
  if (stepTimeIsUp()) goToStep(STEP_FILL);
}

static void runFillStep() {
  turnOnFillValve();
  if (stepTimeIsUp()) {
    markFilledByValve();
    goToStep(STEP_WASH);
  }
}

// Lavagem: bomba sempre ligada; resistência enquanto o termostato da cuba estiver fechado.
// Termina quando passou o tempo mínimo E a água esquentou. Se não esquentar dentro do
// "máx. esperando aquecer", desliga a resistência, avisa e segue o ciclo.
static void runWashStep() {
  turnOnWashPump();
  if (!tankHot() && !heatTimedOut) turnOnHeater();

  if (tankHot() && !washTempReached) {
    washTempReached = true;
    addLog("Termostato da cuba abriu (temperatura atingida)");
  }

  bool waitedTooLong = stepElapsedMs >= settings.heatMaxMin * 60000UL;
  if (waitedTooLong && !washTempReached && !heatTimedOut) {
    heatTimedOut = true;
    addLog("AVISO: termostato da cuba nao abriu no tempo maximo, seguindo sem temperatura");
  }

  if (stepTimeIsUp() && (washTempReached || waitedTooLong)) goToStep(STEP_DRAIN);
}

static void runDrainStep() {
  turnOnDrainPump();
  if (stepTimeIsUp()) {
    markDrained();
    rinseIndex = 0;
    goToNextRinseOrFinish();
  }
}

static void runRinseFillStep() {
  turnOnFillValve();
  if (stepTimeIsUp()) {
    markFilledByValve();
    goToStep(STEP_RINSE);
  }
}

// Enxágue: circula a água; o último pode ser aquecido (configuração "Aquecer último enxágue").
static void runRinseStep() {
  turnOnWashPump();
  if (settings.rinseHeat && isLastRinse() && !tankHot()) turnOnHeater();
  if (stepTimeIsUp()) goToStep(STEP_RINSE_DRAIN);
}

static void runRinseDrainStep() {
  turnOnDrainPump();
  if (stepTimeIsUp()) {
    markDrained();
    rinseIndex++;
    goToNextRinseOrFinish();
  }
}

static void goToNextRinseOrFinish() {
  if (rinseIndex < settings.nRinses) {
    goToStep(STEP_RINSE_FILL);
  } else {
    currentStep = STEP_IDLE;
    addLog("Ciclo concluido");
  }
}

// ---------- Controle ----------

// Chamada a cada volta do loop() enquanto o ciclo está ativo.
void runCycle(uint32_t dt) {
  if (cyclePaused() || fault) return;  // sem pedidos = tudo desligado

  stepElapsedMs += dt;
  switch (currentStep) {
    case STEP_PRE_DRAIN:   runPreDrainStep();   break;
    case STEP_FILL:        runFillStep();       break;
    case STEP_WASH:        runWashStep();       break;
    case STEP_DRAIN:       runDrainStep();      break;
    case STEP_RINSE_FILL:  runRinseFillStep();  break;
    case STEP_RINSE:       runRinseStep();      break;
    case STEP_RINSE_DRAIN: runRinseDrainStep(); break;
    default: break;
  }
}

void startCycle() {
  if (fault)        { addLog("Nao iniciou: falha ativa, use Reset"); return; }
  if (!doorClosed()) { addLog("Nao iniciou: porta aberta"); return; }

  stopAllTests();
  rinseIndex = 0;
  addLog("Ciclo iniciado");
  goToStep(settings.preDrain ? STEP_PRE_DRAIN : STEP_FILL);
}

void stopCycle(const char* why) {
  if (cycleActive()) addLog(String("Ciclo parado: ") + why);
  currentStep = STEP_IDLE;
}
