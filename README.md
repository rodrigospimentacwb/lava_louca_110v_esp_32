# Lava-louças 110 V com ESP32

<img src="docs/lava-loucas-enxuta.jpg" width="300" align="right" alt="Lava-louças Enxuta Master Power de bancada, com tampa escura e o timer redondo no painel bege">

Esta é a nossa **Enxuta Master Power**. Ela já viu mais louça do que muito restaurante, o painel bege entrega a idade, e o timer de engrenagens já cumpriu a missão dele.

Comprar uma nova? Já viu o preço de uma lava-louças hoje? Pois é. E lavar louça na mão está fora de cogitação: em casa **ninguém** quer voltar para a pia. Então a solução foi a mais lógica possível: dar um cérebro novo para a velhinha.

O timer redondo saiu e no lugar entrou uma **ESP32 com um módulo de 4 relés**. Agora ela é controlada pelo **celular**, numa página web que a própria placa serve, sem internet e sem aplicativo. Ela continua velha, mas agora tem Wi-Fi. 😎

<br clear="right">

O timer antigo virou um dial na tela:

| Modo claro | Modo escuro |
|---|---|
| <img src="docs/principal-claro.png" width="280" alt="Aba principal no modo claro: dial do ciclo lavando, botão Parar tudo e sensores"> | <img src="docs/principal-escuro.png" width="280" alt="Aba principal no modo escuro"> |
| <img src="docs/debug-admin-claro.png" width="280" alt="Aba Debug e admin no modo claro: teste de componentes e configuração do ciclo"> | <img src="docs/debug-admin-escuro.png" width="280" alt="Aba Debug e admin no modo escuro"> |

## As placas

| ESP32 DevKit V1 (30 pinos) | Módulo de 4 relés 5 V |
|---|---|
| <img src="docs/placa-esp32.jpeg" width="320" alt="Placa ESP32 DevKit V1 de 30 pinos, com botões EN e BOOT e conector micro-USB"> | <img src="docs/modulo-reles.jpeg" width="200" alt="Módulo azul de 4 relés SRD-05VDC-SL-C com optoacopladores e jumper JD-VCC"> |

A **ESP32** roda o firmware e cria a rede Wi-Fi. O **módulo de 4 relés 5 V** (com optoacoplador, liga com LOW) aciona as cargas de 110 V. A ligação entre as duas (pinos e alimentação) está no [`LIGACAO.md`](LIGACAO.md).

## A caixa

Impressa em 3D (PETG), com a ESP32, o módulo de relés, o step-down e um fan na tampa. Desenho paramétrico em Python; arquivos, medidas e como regerar no [`caixa/README.md`](caixa/README.md).

| Base | Montada (tampa levantada) |
|---|---|
| <img src="caixa/desenhos/render_base.png" width="420" alt="Base da caixa vista de cima: divisória no meio, colunas das placas, furos dos prensa-cabos na parede direita e fendas de ar na esquerda"> | <img src="caixa/desenhos/render_montada.png" width="420" alt="Caixa com a tampa levantada: tampa com grade redonda do fan e os 4 parafusos dos cantos"> |

- **Dois lados separados por uma divisória:** à esquerda a baixa tensão (ESP32, step-down, fios dos sensores e entrada de 12 V); à direita o 110 V (relés e cabos das cargas).
- **Fan de 70 mm na tampa** soprando para dentro; o ar sai pelas fendas da parede esquerda.
- **Furos para prensa-cabos** e, ao lado de cada um, uma âncora para abraçadeira: um puxão no cabo não chega ao borne do relé.
- Para ver em 3D, abra os `.stl` em [`caixa/desenhos/`](caixa/desenhos) aqui no GitHub.

### Lista de materiais

| Item | Qtd. | Para quê |
|---|---|---|
| ESP32 DevKit V1 (30 pinos) | 1 | controle e Wi-Fi |
| Módulo de 4 relés 5 V (SRD-05VDC-SL-C, com optoacoplador) | 1 | aciona as cargas de 110 V |
| Step-down LM2596 | 1 | 12 V → 5 V para a ESP32 e os relés |
| Fonte de 12 V (tipo carregador, ≥ 1 A) | 1 | alimentação (externa por enquanto) |
| Fan 70 × 70 × 16 mm, 12 V (Delta AFB0712HB) | 1 | ventilação da caixa |
| Filamento PETG | ~160 g | base e tampa |
| Inserto de rosca M3 × 6 (Ø 4,2 mm) + parafuso M3 × 8 | 4 + 4 | tampa |
| Inserto de rosca M3 × 4 (Ø 4,2 mm) + parafuso M3 × 6 | 10 + 10 | ESP32 (4), relés (4), LM2596 (2) |
| Parafuso M3 × 25 + porca autotravante + arruela | 4 + 4 + 8 | fan (por fora, em cima da tampa) |
| Conector Wago 221-415 | 2 | distribuir fase e juntar neutros |
| Ponteira tubular 1,0 e 1,5 mm² | 1 pacote | ponta dos fios nos bornes dos relés |
| Cabo PP 3 × 1,5 mm² | ~1 m | entrada de força (fase, neutro, terra) |
| Cabo PP 2 × 1,5 mm² | conforme a distância | resistência |
| Cabo PP 2 × 1,0 mm² | 3 pedaços | bomba de lavagem, bomba de drenagem, solenoide |
| Cabo manga de 4 vias | conforme a distância | porta e termostato da cuba |
| Abraçadeira de nylon (até 3,6 mm de largura) | 1 pacote | prender os cabos nas âncoras |
| Prensa-cabo PG11 / PG9 / PG7 | 2 / 3 / 2 | vedação dos cabos (opcional por enquanto) |

## Como funciona

A placa liga e desliga 4 cargas e lê 2 sensores:

| Liga e desliga (relés) | Lê (sensores) |
|---|---|
| Resistência (esquenta a água) | Porta fechada ou aberta |
| Bomba de lavagem (joga água nas louças) | Termostato da cuba: a água chegou na temperatura? |
| Bomba de drenagem (tira a água) | |
| Solenoide de entrada (deixa a água entrar) | |

Ainda não tem bóia, então o nível de água é controlado **pelo tempo** que o solenoide fica aberto.

**O ciclo:** drena o que sobrou, enche, lava com água quente, drena e faz os enxágues. Tudo aparece num dial redondo, no lugar do timer antigo.

**Segurança vem antes de tudo:**
- porta aberta desliga tudo na hora;
- a resistência só liga com água que entrou pelo solenoide e com a água ainda fria. Marcar "com água" na página não basta;
- encher e drenar nunca acontecem juntos;
- o Klixon fica em série com a resistência: se ela superaquecer, ele corta a corrente sozinho, sem depender da placa;
- só a página da própria placa consegue mandar comandos. Um site qualquer aberto no celular não consegue ligar nada.

**A página tem duas abas:**
- **Principal:** o dia a dia. Mostra o dial, o botão iniciar/parar, os sensores e o tempo que falta.
- **Debug e admin:** testar cada componente, ajustar os tempos do ciclo, ver as leituras cruas, as informações da placa e o registro de eventos.

## Andamento

Testes de bancada feitos até agora, sem 110 V:

| Teste | Situação |
|---|---|
| Sensores: porta, termostato da cuba, Klixon (falha e liberação) | ✅ |
| Wi-Fi da casa com IP fixo reservado no roteador | ✅ |
| Módulo de relés alimentado pelo `VIN` (5 V), com o jumper | ✅ |
| Os 4 relés nos pinos definitivos | ✅ |
| Resistência bloqueada sem água e com a água quente | ✅ |
| Porta aberta desliga e bloqueia tudo | ✅ |
| Página recusa comandos vindos de outros sites | ✅ |
| Resistência bloqueada com a água só marcada à mão | ✅ |
| Encher e drenar nunca juntos | ✅ |
| Sensores de verdade na máquina | ⏳ |
| Testes com 110 V, calibrar o enchimento, ciclo completo | ⏳ |

A ordem dos próximos testes está no [`LIGACAO.md`](LIGACAO.md), item 7.

## Como gravar

1. Copie `LavaLoucas/secrets.example.h` para `LavaLoucas/secrets.h` e coloque a senha da rede da placa (`AP_PASS`, mínimo 8 caracteres). Se quiser, preencha também a sua rede de casa (`WIFI_SSID` e `WIFI_PASS`). O `secrets.h` não vai para o git.
2. Compile e grave pelo terminal com o [`arduino-cli`](https://arduino.github.io/arduino-cli/), com o core `esp32:esp32` instalado (URL da placa: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`):
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32 --build-path build LavaLoucas
   arduino-cli upload  -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --input-dir build
   ```
   Se preferir, use a Arduino IDE: abra `LavaLoucas/LavaLoucas.ino`, escolha a placa **ESP32 Dev Module** e grave. Grave com a máquina fora da tomada.
3. **Depois da primeira gravação, dá para atualizar pela página**, sem abrir a caixa: defina `OTA_PASS` no `secrets.h` (senha própria, mínimo 8 caracteres) e grave uma vez pela USB. Daí em diante, compile, abra a aba **Debug e admin**, em "Atualizar o programa" escolha `build/LavaLoucas.ino.bin`, digite a senha e envie. Só funciona com o ciclo parado; a placa desliga tudo, grava e reinicia. Se o envio cair no meio, ela continua com o programa antigo. O `.bin` tem as senhas dentro: nunca publique.
4. No celular, entre na rede Wi-Fi **LavaLoucas** e abra **http://192.168.4.1**. Se configurou a rede de casa, use o IP que a placa pegou (aparece no registro da página e na serial) ou **http://lavaloucas.local**, que só funciona para quem está na mesma rede da placa. Dica: reserve um IP fixo para a placa no roteador.

**Se a placa reiniciar sozinha** (a página ora responde, ora trava), o problema costuma ser a alimentação: cabo ou porta USB com mau contato. Veja o item 3 do [`LIGACAO.md`](LIGACAO.md).

> ⚠️ **Perigo: 110 V, água e uma resistência de ~1000 W.** Há risco real de choque elétrico e de incêndio.
> - **Antes de ligar na tomada, leia o [`LIGACAO.md`](LIGACAO.md)**: esquema elétrico, aterramento, proteções físicas e a ordem de testes.
> - Nunca mexa na fiação nem grave o firmware com a máquina na tomada.
> - O firmware desliga tudo com a porta aberta, mas software pode falhar: as proteções físicas (Klixon em série com a resistência, aterramento, DR) são obrigatórias.
> - Faça por sua conta e risco.

## Onde está cada coisa

```
LavaLoucas/
├── LavaLoucas.ino        setup() e loop(): o roteiro principal
├── config.h              pinos e ajustes de hardware
├── secrets.h             senhas (só na sua máquina)
└── src/
    ├── hardware/         sensores e relés
    ├── control/          ciclo, travas de segurança e água
    ├── storage/          configuração salva e registro de eventos
    └── web/              Wi-Fi, API e a página
docs/                     fotos da máquina e das placas, prints da página
caixa/                    caixa impressa: scripts, STL/3MF e imagens
scripts/intellisense.sh   autocomplete do VS Code
```

Para usar o VS Code: instale a extensão C/C++ e rode `./scripts/intellisense.sh` depois de instalar o `arduino-cli`.
