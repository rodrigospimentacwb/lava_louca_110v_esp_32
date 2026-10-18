# Ciclo de lavagem

Código em `src/control/cycle.cpp`. Cada etapa é uma função `run…Step()` que pede os relés da etapa e chama `goToStep()` com a próxima quando termina. O tempo da etapa (`stepElapsedMs`) só conta com a porta fechada há 2 s e sem falha.

| # | Etapa (`Step`) | Relés pedidos | Termina quando | Próxima |
|---|---|---|---|---|
| 1 | `STEP_PRE_DRAIN` (opcional, `preDrain`) | drenagem | `drainSec` | enchimento |
| 2 | `STEP_FILL` | solenoide | `fillSec` → marca "com água" | lavagem |
| 3 | `STEP_WASH` | bomba de lavagem + resistência enquanto a cuba estiver fria | passou `washMin` **e** (termostato abriu **ou** passou `heatMaxMin`) | drenagem |
| 4 | `STEP_DRAIN` | drenagem | `drainSec` → marca "vazia" | enxágue 1 ou fim |
| 5 | `STEP_RINSE_FILL` | solenoide | `fillSec` | enxágue |
| 6 | `STEP_RINSE` | bomba de lavagem (+ resistência no último, se `rinseHeat`) | `rinseMin` | drenagem do enxágue |
| 7 | `STEP_RINSE_DRAIN` | drenagem | `drainSec` | próximo enxágue (5) ou fim |

Etapas 5 a 7 repetem `nRinses` vezes (0 a 5). Os ids do enum são usados pela página (`stepId`) para desenhar o dial: **não reordenar o enum sem atualizar `plannedSteps()` em `page.h`.**

## Decisões

- **Nível por tempo**: ainda não há bóia; a válvula de nível original foi removida. O enchimento é calibrado na aba Debug e admin ("Solenoide aberto"). Quando a bóia entrar, `runFillStep`/`runRinseFillStep` passam a terminar pela leitura dela, e `water.cpp` deixa de estimar.
- **Termostato da cuba = termômetro liga/desliga**: durante a lavagem a resistência liga sempre que ele fecha (água esfriou) e desliga quando abre, quantas vezes for preciso. O Klixon na resistência é só a proteção de sobretemperatura.
- **Lavagem espera aquecer**: a água quente é o que limpa; por isso o tempo mínimo só termina a etapa depois que o termostato abrir. O `heatMaxMin` evita ficar preso se a resistência ou o termostato falharem; ao estourar, a resistência fica desligada até o fim da etapa e um aviso vai para o registro.
- **Sem secagem com resistência**: sem água e sem bóia, aquecer a seco é o pior cenário (incêndio). Não adicionar.
- **Resistência só com a bomba de lavagem no ciclo**: a água circulando distribui o calor e mantém o termostato da cuba lendo a temperatura real.
- **Parar = cancelar**: não existe "retomar de onde parou" depois de Parar tudo ou de falha. Ao religar a placa, a máquina começa parada.

## Padrões (`CycleSettings`, ajustáveis na página e salvos na flash)

`fillSec` 60 s, `drainSec` 60 s, `washMin` 20 min, `heatMaxMin` 40 min, `rinseMin` 5 min, `nRinses` 2, `rinseHeat` sim, `preDrain` sim. Com os padrões o ciclo leva ~37 min (até ~57 min se a água não esquentar).
