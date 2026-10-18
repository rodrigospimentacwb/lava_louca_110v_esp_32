# Lava-louças com ESP32: ligação e uso

## 1. As placas

- **ESP32 DevKit V1, 30 pinos (módulo ESP-WROOM-32)**: a placa com Wi-Fi. Os pinos têm nomes escritos nela (`D32`, `D18`…, o número é o GPIO); é por eles que este guia fala. Os pinos trabalham com 3,3 V (**nunca** ligue 5 V num pino `D`).
- **Módulo de 4 relés 5 V com optoacoplador** (azul, relés SRD-05VDC-SL-C, pinos `GND IN1 IN2 IN3 IN4 VCC` e o jumper `JD-VCC`):
  - O relé **liga com LOW** no pino `IN` (no firmware: `RELAY_ACTIVE_LEVEL LOW` no `LavaLoucas/config.h`).
  - Teste rápido: grave o firmware **sem nada ligado nos contatos dos relés**. Se ao ligar a placa os 4 relés clicarem e ficarem ligados, troque para `HIGH`.

## 2. Mapa de pinos

| Função | Pino na placa | Ligar em | Observação |
|---|---|---|---|
| Relé 1: Resistência | **D32** | `IN1` | liga só com água, porta fechada e termostato da cuba fechado |
| Relé 2: Bomba de lavagem | **D33** | `IN2` | |
| Relé 3: Bomba de drenagem | **D25** | `IN3` | nunca junto com o solenoide |
| Relé 4: Solenoide de entrada | **D26** | `IN4` | nunca junto com a drenagem |
| Interruptor da porta | **D18** | contato até o GND | fechado (LOW) = porta fechada |
| Termostato da cuba (NF) | **D19** | contato até o GND | fechado (LOW) = frio; aberto (HIGH) = temperatura atingida |
| Klixon da resistência (NA) | **D21** | contato até o GND | aberto (HIGH) = normal; fechado (LOW) = **sobretemperatura, desliga tudo** |

**Por que esses pinos.** Os relés (`D32`, `D33`, `D25`, `D26`) ficam lado a lado na fileira de cima, e os sensores (`D18`, `D19`, `D21`) na fileira de baixo, com um `GND` do mesmo lado. Nenhum deles tem papel no boot da ESP32. Alguns pinos a evitar:
- `D0` (BOOT), `D2`, `D5`, `D12` e `D15` decidem como a placa liga. O `D12` em HIGH na hora de ligar pode até impedir a placa de iniciar.
- O `D14` solta pulsos no boot e faria um relé clicar.
- `D34`, `D35`, `VP` e `VN` só servem de entrada e não têm pull-up interno.

**Pinos livres para o futuro (bóia):** `D22`, `D23`, `D27`, `D13` e `D4`.

Cada sensor vai como **contato seco**: um terminal no pino, o outro no **GND** da ESP32. Não precisa resistor (o pull-up interno está ativo). Para fios longos, um capacitor de 100 nF entre o pino e o GND ajuda contra ruído.

Fio solto ou mau contato: a porta é lida como **aberta** e a cuba como **quente**, então tudo desliga (falha segura). O Klixon é NA, então fio solto nele **não** é detectado; por isso a proteção física do item 5.4 é importante.

## 3. Alimentação do módulo de relés

O módulo inteiro é alimentado com **5 V**, com o **jumper amarelo no lugar** (unindo `VCC` e `JD-VCC`):

1. **Deixe o jumper** entre `JD-VCC` e `VCC`. Não ligue nenhum fio no `JD-VCC`.
2. Barra de pinos `VCC` → **`VIN`** da ESP32 (os 5 V que chegam pela USB ou pela fonte).
3. Barra de pinos `GND` → **`GND`** da ESP32.
4. `IN1` a `IN4` → `D32`, `D33`, `D25`, `D26`.

```
   ESP32 VIN ─────────────── VCC  (barra de pinos do módulo, jumper no lugar)
   ESP32 GND ─────────────── GND  (barra de pinos do módulo)
   ESP32 D32, D33, D25, D26 ─ IN1, IN2, IN3, IN4
```

**Por que não alimentar o `VCC` com 3,3 V.** Em cada canal, o LED indicador fica em série com o LED do optoacoplador, e os dois juntos precisam de uns 3 V. Com o `VCC` em 3,3 V não sobra tensão, e o relé não liga (testado: nenhum LED acendeu).

**Por que 5 V no `VCC` é seguro para a ESP32.** O pino `IN` só recebe corrente: quando a ESP32 manda LOW (0 V), os LEDs acendem e o relé liga. Quando manda HIGH (3,3 V), sobram uns 1,7 V para os dois LEDs, abaixo do que eles precisam, e o relé fica desligado de verdade. A tensão no pino nunca passa de 3,3 V.

**Cuidados:**
- O `VIN` fica **colado no `GND`** na ESP32. Confira o pino antes de encaixar: um fio no lugar errado faz curto nos 5 V.
- **Placa reiniciando sozinha** (página que ora responde, ora trava; na serial aparece `Brownout detector was triggered`) é tensão baixa na alimentação. Na bancada, a causa foi **mau contato no cabo ou na porta USB**: replugar resolveu. Use cabo curto e uma porta traseira do PC, sem hub. **Não desligue o detector de brownout** no firmware: ele protege a placa que comanda os relés.
- Na máquina, use uma **fonte de 5 V de pelo menos 1 A** (carregador de celular bom serve) no `VIN`/`GND`. Os 4 relés juntos puxam uns 300 mA além da ESP32.

## 4. Esquema de ligação (110 V)

```
                REDE 110 V
   FASE ──[disjuntor/fusível 15 A]──┬──────────────────────────────┐
                                    │                              │
                         (opcional, recomendado:                    │
                          interruptor da porta em série             │
                          aqui, ver item 5.4)                       │
                                    │                              │
                     ┌──────────────┼──────────────┬───────────────┤
                     │              │              │               │
                  COM K1         COM K2         COM K3          COM K4
                  NO  K1         NO  K2         NO  K3          NO  K4
                     │              │              │               │
              [Termostato        Bomba          Bomba          Solenoide
               segurança NF]    lavagem        drenagem        entrada
                     │              │              │               │
               Resistência          │              │               │
                     │              │              │               │
   NEUTRO ───────────┴──────────────┴──────────────┴───────────────┘

   TERRA (fio verde) ── carcaça metálica da máquina, cuba e motores

   ESP32 (baixa tensão, isolada da rede):
     D18 ──── interruptor da porta ──── GND
     D19 ──── termostato da cuba ────── GND
     D21 ──── Klixon da resistência ─── GND
```

Cada relé tem 3 bornes: o do meio é o **COM**, e o **NO** (normalmente aberto) fica de um lado. Use sempre COM + NO: assim, com a placa desligada ou reiniciando, nada liga.

## 5. Segurança (leia antes de ligar na tomada)

1. **Sensores precisam sair do circuito de 110 V.** No timer original, porta, termostato e Klixon estavam ligados na rede. Desconecte **os dois fios** de cada um da fiação antiga e ligue só à ESP32. Se um terminal desses ainda encostar em fase, você queima a placa e coloca 110 V numa placa que você toca.
2. **Resistência no relé do módulo.** Esses relés são de 10 A (às vezes menos na prática, com trilhas finas). Resistência de lava-louças costuma ter 800 a 1200 W, ou seja 7 a 11 A em 110 V, no limite ou acima. Para o teste: meça a corrente com alicate amperímetro, deixe ligado pouco tempo e verifique se o relé ou as trilhas esquentam. Se esquentar, passe para o SSR (com dissipador) ou use o relé do módulo para acionar um contator.
3. **Aterramento.** A máquina tem água e metal: a carcaça precisa estar no terra da tomada. Ligue a máquina num circuito com **DR** (disjuntor diferencial residual) se tiver.
4. **Intertravamento físico, não só software.** O firmware desliga tudo com a porta aberta ou com o Klixon fechado, mas software pode travar. Recomendo:
   - Um **termostato de segurança NF** (ou fusível térmico) em série com a resistência, como no esquema. Muitas máquinas já têm um no próprio corpo da resistência.
   - Mais adiante, a porta cortando a fase dos relés. Como o interruptor dela vai para a ESP32 (item 1), seria preciso um segundo microswitch na porta, ou um interruptor de 2 polos.
5. **Ruído elétrico.** Bombas e solenoide são indutivos e podem reiniciar a placa quando desligam. Se acontecer: coloque um snubber RC (100 Ω + 100 nF/275 V X2) ou um varistor 150 V em paralelo com cada carga, use uma fonte 5 V de boa qualidade e mantenha os fios de sensor longe dos de 110 V.
6. **Grave o firmware com a máquina fora da tomada.** Durante a gravação e o reinício, os pinos ficam soltos por alguns instantes; com o módulo certo nada liga, mas não vale arriscar com 110 V.
7. **Nunca mexa na fiação com a máquina na tomada.** Os bornes dos relés ficam expostos; feche as placas numa caixa plástica.

## 6. Como os dois sensores de temperatura trabalham juntos

| | Termostato da cuba | Klixon da resistência |
|---|---|---|
| Onde fica | no fundo da cuba, em contato com a água | preso na própria resistência |
| Tipo | NF: fechado frio, **abre** quando a água chega na temperatura | NA: aberto normal, **fecha** quando a resistência superaquece |
| Papel | o "termômetro": liga e desliga a resistência para **manter a água quente** | a proteção: só dispara se algo deu errado (resistência sem água, bomba parada) |
| O que o firmware faz | durante a lavagem, liga a resistência sempre que ele fecha (água esfriou) e desliga quando ele abre | desliga **tudo**, cancela o ciclo e espera você limpar a falha |

Na lavagem, a água esfria ao bater nas louças, o termostato fecha de novo e a resistência volta a ligar, quantas vezes for preciso. Se o termostato **nunca** abrir dentro do "máximo esperando aquecer", alguma coisa está errada (termostato ou resistência com defeito). Nesse caso a resistência desliga até o fim da lavagem, o registro mostra um aviso e o ciclo segue.

## 7. Como usar

1. Copie `LavaLoucas/secrets.example.h` para `LavaLoucas/secrets.h` e defina a senha do AP (`AP_PASS`, mínimo 8 caracteres). Se quiser que a placa entre na sua rede, preencha também `WIFI_SSID` e `WIFI_PASS`.
2. Grave (veja "Como gravar" no `README.md`). No celular, conecte na rede **LavaLoucas** (com a senha que você pôs em `AP_PASS`) e abra **http://192.168.4.1**. Se configurou sua rede, use **http://lavaloucas.local** ou o IP que aparece no Monitor Serial (115200).
3. A página tem duas abas:
   - **Principal**: o dial do ciclo (etapa atual, tempo e quanto falta), Iniciar/Parar, porta, água, temperatura e Klixon, e quais cargas estão ligadas.
   - **Debug e admin**: **teste de componentes** (um botão por relé; cada um desliga sozinho depois de um tempo máximo: 10 min resistência/lavagem, 3 min drenagem/solenoide; se algo estiver bloqueado, aparece o motivo), marcar água, **configuração do ciclo** (fica salva na placa), leitura crua dos sensores, relés, informações do sistema, **registro** dos últimos eventos e reiniciar a placa.

### Ordem de testes sugerida

1. **Só a placa na USB**, sem o módulo de relés: confira na aba Debug e admin se porta, termostato e Klixon mudam de HIGH para LOW ao fechar cada contato.
2. **Placa + módulo de relés, sem 110 V**: confira se os relés ficam desligados ao ligar a placa (se clicarem todos, troque `RELAY_ACTIVE_LEVEL`) e se cada botão de teste liga o relé certo.
3. Sensores de verdade, ainda sem 110 V: abra e feche a porta, aqueça o termostato da cuba com um secador, e confira se a página mostra a mudança.
4. Com 110 V e máquina vazia: teste a drenagem e a bomba de lavagem por alguns segundos.
5. **Calibre o enchimento**: ligue o solenoide e desligue quando a água chegar no nível certo (normalmente logo abaixo da porta, cobrindo a resistência). Na aba **Debug e admin**, "Solenoide aberto" mostra quantos segundos foram. Coloque esse valor em **Enchimento**. Depois drene.
6. Encha de novo e só então teste a resistência, acompanhando corrente e temperatura do relé (item 5.2).
7. Rode um ciclo completo com você do lado.

### Ciclo programado

1. Drenagem inicial (opcional)
2. Enchimento por tempo
3. Lavagem: bomba ligada; resistência ligada sempre que o termostato da cuba estiver fechado (água fria). A etapa termina quando passou o tempo mínimo de lavagem **e** o termostato abriu. Se ele não abrir dentro do "máximo esperando aquecer", a resistência desliga até o fim da lavagem, aparece um aviso no registro e o ciclo segue.
4. Drenagem
5. Enxágues (enche, circula, drena), quantos estiverem configurados; o último pode ser aquecido
6. Fim

**Proteções:**
- Porta aberta: tudo desliga e o ciclo **pausa**. Retoma 2 s depois que fechar.
- Klixon fechado: tudo desliga, o ciclo é cancelado e fica em falha até você apertar **Limpar falha** (só aceita com o Klixon aberto de novo).
- Resistência só liga com água que entrou pelo solenoide ("Marcar com água" na página não basta), e no ciclo só junto com a bomba de lavagem.
- Solenoide e drenagem nunca ligam juntos.

Não há etapa de secagem com a resistência, de propósito: sem água e sem bóia, aquecer a seco é o cenário mais perigoso. Quando instalar a bóia, dá para trocar o enchimento por tempo pela leitura dela.
