# Segurança (não negociável)

A máquina tem 110 V, água e uma resistência de 800 a 1200 W. Software pode travar, então o firmware é só **uma** camada; a proteção física está no `LIGACAO.md`. Mesmo assim, estas regras do firmware nunca podem ser removidas ou contornadas:

| Regra | Onde |
|---|---|
| Porta aberta desliga **todos** os relés e pausa o ciclo; retoma 2 s depois de fechar | `safety.cpp` (`applySafetyInterlocks`, `doorClosedLongEnough`) |
| Klixon NF **em série com a resistência** (110 V): corta sozinho, sem software. Se um dia for para o `D21` (`KLIXON_ON_ESP`), disparado = só a resistência desliga, até a próxima etapa; o ciclo segue | `LIGACAO.md` item 4; `safety.cpp`, `cycle.cpp` (`heatIfCold`) |
| Resistência só liga com água que entrou pelo solenoide (`waterFromValve`) **e** cuba fria (termostato da cuba aberto). "Marcar com água" na página não libera a resistência | `safety.cpp`, `water.cpp` |
| Ações da API (POST) exigem o cabeçalho `X-Lava: 1` e o endereço da placa no `Host`; leituras (GET) exigem o `Host` | `web_server.cpp` (`guarded`) |
| Solenoide de entrada e bomba de drenagem nunca juntos (a drenagem ganha) | `safety.cpp` e `startTest()` |
| No boot, cada relé é escrito como desligado **antes** do `pinMode(OUTPUT)` | `relays.cpp` (`setupRelays`) |
| Relés ativos em LOW, em pinos que ficam soltos no boot (D32, D33, D25, D26) = desligados. Nunca usar pinos de boot (0, 2, 5, 12, 15) nem o 14 para relé | `config.h` |
| Teste manual desliga sozinho em `TEST_MAX_MS` | `relays.cpp` |
| Não existe secagem com resistência | `cycle.cpp` |

## O padrão pedido → trava → escrita

Nenhum código liga relé direto. A cada `loop()`:

1. `clearRequests()` zera os pedidos;
2. o ciclo **ou** o teste manual chamam `turnOnWashPump()`, `turnOnHeater()`... (só marcam `relayRequested[]`);
3. `applySafetyInterlocks()` barra o que for perigoso e guarda o motivo em `blockReason[]`;
4. `updateRelays()` escreve nos pinos só o que mudou.

**Nunca chamar `digitalWrite` num pino de relé fora de `relays.cpp`**, e dentro dele só em `updateRelays()`, `setupRelays()` e `forceAllRelaysOff()` (este último só desliga). Uma nova carga ou regra entra como pedido + trava, nunca como escrita direta.

## Falhas seguras dos sensores

Sensores são contato seco GPIO↔GND com pull-up interno. Fio solto = HIGH: porta "aberta", então tudo desliga. O termostato da cuba é **NA** (fecha quando esquenta): fio solto nele = cuba "fria" e **não** é detectado; quem segura é o Klixon em série e o `heatMaxMin`. Nunca tirar o Klixon do circuito da resistência sem pôr outra proteção física no lugar.

## Ao mexer em algo de segurança

- Explicar no PR qual regra muda e por quê.
- Preferir a opção mais conservadora (ex.: depois de estourar o "máximo esperando aquecer", a resistência fica desligada até o fim da lavagem, mesmo que a água esfrie).
