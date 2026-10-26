# Arquitetura do firmware

Arduino (core `esp32:esp32` 3.x), placa **ESP32 DevKit V1 de 30 pinos** (ESP-WROOM-32), FQBN `esp32:esp32:esp32`. Só bibliotecas do core: `WiFi`, `WebServer`, `ESPmDNS`, `Preferences`.

Histórico (2026-10-17): o projeto nasceu para ESP32 com shield de relés; uma foto mostrou uma NodeMCU ESP8266 e o firmware foi portado; depois o dono confirmou que a placa da máquina é esta ESP32 DevKit e o firmware voltou para ESP32. O que ficou das descobertas: os relés são um **módulo separado, ativo em LOW**, e os pinos foram escolhidos pelas regras de boot. O módulo ESP8266 (NodeMCU, CP2102) foi o usado nos primeiros testes de bancada (sensores, Wi-Fi, IP fixo).

## Módulos

| Arquivo | Responsabilidade |
|---|---|
| `LavaLoucas.ino` | só `setup()` e `loop()`: o roteiro, sem lógica |
| `config.h` | pinos (números de GPIO; na placa aparecem como `D32`…), nível ativo dos relés (`LOW`), nomes, tempos máximos de teste, debounce |
| `secrets.h` | senhas (fora do git); só `src/web/network.cpp` inclui |
| `src/hardware/inputs` | debounce e significado dos sensores: `doorClosed()`, `tankHot()`, `klixonTripped()` |
| `src/hardware/relays` | pedidos `turnOn…/turnOff…`, escrita nos pinos, teste manual |
| `src/control/safety` | travas, porta, registro do Klixon (se ligado no `D21`) |
| `src/control/cycle` | etapas do ciclo (`run…Step()`), iniciar/parar |
| `src/control/water` | água estimada por tempo (sem bóia ainda) |
| `src/storage/settings` | `CycleSettings` salvo na flash (`Preferences`, namespace `lava`) |
| `src/storage/event_log` | fila com os últimos 20 eventos (página + Serial) |
| `src/web/network` | AP `LavaLoucas` sempre ligado + rede de casa opcional + mDNS `lavaloucas.local` (só funciona na mesma rede: não atravessa roteador) |
| `src/web/web_server` | rotas HTTP/JSON |
| `src/web/page.h` | HTML/CSS/JS da página num raw string |

Cada `.h` é o "índice" do módulo: lista o que ele oferece, com comentário curto. Estado compartilhado é exposto com `extern` no `.h`; o resto é `static` no `.cpp`.

## Ordem do `loop()`

1. `updateInputs` → `handleDoorChange` → `checkOverTemperature`
2. `clearRequests` → `runCycle(dt)` **ou** `requestTestOutputs(now)`
3. `applySafetyInterlocks` → `updateRelays` → `updateWaterEstimate(dt)`
4. `handleWebClients` → `delay(2)`

Sem `delay` longo em lugar nenhum: tudo é por `millis()` e `dt`.

## Wi-Fi

- `WiFi.setSleep(false)` no `network.cpp`: a ESP32, por padrão, faz "modem sleep" (o rádio cochila entre pacotes para economizar energia). A placa fica na tomada, então isso não ajuda em nada, e com sinal fraco causava respostas lentas e "timed out" na página (visto em 2026-10-17 com -74 dBm). **Não remover.**
- A lava-louças fica longe do modem Huawei (sinal em torno de -74 dBm, que é "ok"). Antes de culpar o código por lentidão na página, olhar o sinal na aba Debug e admin: abaixo de -80 dBm é problema de rede, não de firmware.
- O servidor web atende um pedido por vez: evitar consultas muito frequentes (a página consulta `/api/status` a cada 1 s e `/api/debug` a cada 2 s, só com a aba admin aberta).

## Alimentação

- Módulo de relés com `VCC` → `VIN` (5 V) e o jumper `JD-VCC` no lugar. Com `VCC` em 3,3 V os relés **não ligam** (LED indicador em série com o LED do optoacoplador); testado em 2026-10-18. Detalhes no `LIGACAO.md`, item 3.
- Placa reiniciando em loop com `Brownout detector was triggered` na serial = alimentação fraca (visto em 2026-10-18: era mau contato na USB). **Não desativar o brownout** no código; resolver a fonte ou o cabo.

## Pegadinhas do build Arduino

- Subpastas só compilam dentro de `LavaLoucas/src/`, e lá **não pode ter `.ino`**: só `.cpp` e `.h`. Incluir com caminho relativo (`"../hardware/relays.h"`, `"../../config.h"`).
- O `config.h` é incluído por vários `.cpp`: constantes de texto precisam ser `const char* const` (senão dá definição duplicada no link). Arrays e números `const` já têm ligação interna.
- `#define RELAY_ACTIVE_LEVEL` depende de `HIGH/LOW`, por isso o `config.h` inclui `<Arduino.h>`.
- Se algum dia voltar a ter várias `.ino`: o Arduino gera protótipos automaticamente e pode colocá-los antes de um `enum` usado como parâmetro (foi o erro `'Step' was not declared`). Solução: protótipo manual depois do `enum`.
- O `compile_commands.json` gerado pelo `arduino-cli` aponta para cópias em `build/sketch/`; o `scripts/intellisense.sh` corrige os caminhos para o VS Code.

## Pinos da ESP32 DevKit (não mudar sem ler)

| Pino na placa | Uso | Por quê |
|---|---|---|
| D32, D33, D25, D26 | resistência, lavagem, drenagem, solenoide | lado a lado na fileira de cima, sem papel no boot; soltos no boot = relé desligado (ativo em LOW) |
| D18, D19 (D21 reservado ao Klixon, hoje em série no 110 V) | porta, termostato | fileira de baixo (com GND do lado), aceitam `INPUT_PULLUP`, não afetam o boot |
| D22, D23, D27, D13, D4 | livres (bóia, extras) | seguros |
| D34, D35, VP (36), VN (39) | livres, **só entrada** | sem pull-up interno; servem para analógico |
| D0 (BOOT), D2, D5, D12, D15 | **evitar** | decidem o modo de boot (D12 em HIGH no boot pode até impedir a placa de ligar) |
| D14 | **evitar para relé** | solta PWM no boot |
| RX0/TX0 (1, 3) | **não usar** | serial da USB |

## Uso de memória (referência)

Flash ~77% (1,0 MB de 1,3 MB), RAM global ~15%. A página é a maior parte do crescimento; manter sem dependências.
