# Lava-louças 110 V com ESP32

Firmware Arduino para **ESP32 DevKit V1 (30 pinos, ESP-WROOM-32)** que substitui o timer eletromecânico de uma lava-louças Enxuta antiga, 110 V. A placa aciona um módulo separado de 4 relés 5 V, ativo em LOW (resistência, bomba de lavagem, bomba de drenagem, solenoide de entrada), lê 3 sensores (porta, termostato da cuba, Klixon) e serve uma página web pelo Wi-Fi para operar, testar e configurar.

**Isto controla 110 V, água e uma resistência de ~1000 W.** Toda mudança que mexe em relé, sensor ou ciclo precisa respeitar `.claude/rules/seguranca.md`.

## Comandos

```bash
# compilar (ESP32 Dev Module)
arduino-cli compile --fqbn esp32:esp32:esp32 --build-path build --warnings default LavaLoucas

# gravar e ler a serial (115200)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --input-dir build
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200

# atualizar o autocomplete do VS Code (rodar ao criar/renomear arquivos)
./scripts/intellisense.sh
```

Não há testes automatizados: "funcionou" = compila sem erro **e sem aviso**. Mudanças na página devem ser conferidas no navegador (dá para servir `src/web/page.h` com um servidor falso que imita a API).

## Regras do projeto

- [`.claude/rules/fluxo-de-trabalho.md`](.claude/rules/fluxo-de-trabalho.md): PR, senhas, idioma, o que não fazer
- [`.claude/rules/seguranca.md`](.claude/rules/seguranca.md): travas que nunca podem ser removidas
- [`.claude/rules/arquitetura.md`](.claude/rules/arquitetura.md): módulos, fluxo do `loop()` e pegadinhas do build Arduino
- [`.claude/rules/ciclo.md`](.claude/rules/ciclo.md): etapas do ciclo e por que são assim
- [`.claude/rules/web.md`](.claude/rules/web.md): página, abas, API e design

Contexto de hardware, esquema elétrico e ordem de testes: `LIGACAO.md`.
