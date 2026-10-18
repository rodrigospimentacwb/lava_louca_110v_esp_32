# Página web e API

## Restrições

- **Roda sem internet**: o celular fica na rede `LavaLoucas` da placa. Nada de Google Fonts, CDN ou bibliotecas externas; tudo dentro de `src/web/page.h` (HTML, CSS, JS e o favicon em data URI).
- A página fica num raw string `R"HTML( … )HTML"`: o texto `)HTML"` não pode aparecer dentro do HTML.
- Celular primeiro (coluna de até 560 px), modo claro e escuro (`prefers-color-scheme`), foco de teclado visível, respeita `prefers-reduced-motion`.
- Textos para quem usa a máquina, em português, sem jargão do firmware ("Limpar falha", não "reset fault"). Falar "a placa", não o nome do chip.
- Pinos aparecem com o nome escrito na placa (`D32`), que vem de `pinLabel()` no `web_server.cpp`.

## Abas

- **Principal**: o dia a dia. Dial circular (o "timer antigo"): etapas em volta proporcionais ao tempo, ponteiro na posição atual, etapa e tempo no centro. Botão grande Iniciar/Parar, tempo restante e horário estimado de término (usa o relógio do celular; a placa não tem relógio), sensores e cargas ligadas.
- **Debug e admin**: teste de componentes (com contagem regressiva e motivo de bloqueio), marcar água, configuração do ciclo, voltar aos padrões, leitura crua dos sensores (HIGH/LOW), relés, informações do sistema, registro e reiniciar a placa. O `/api/debug` só é consultado com esta aba aberta.

A aba aberta fica na URL (`#admin`).

## Design

- Paleta de "painel de eletrodoméstico": cinza claro, tinta azul-petróleo; cores por tipo de etapa: encher `--fill` (azul água), lavar `--wash` (azul fundo), drenar `--drain` (ardósia), resistência/falha `--heat`, ligado `--on`.
- Fonte arredondada do sistema (`ui-rounded`), números tabulares nos tempos.
- O dial é a única peça "chamativa"; o resto é discreto (superfícies com borda fina, sem sombras em tudo).
- Prints em `docs/` (claro e escuro) aparecem no README: atualizar quando a página mudar visivelmente.

## API (`src/web/web_server.cpp`)

| Rota | Uso |
|---|---|
| `GET /api/status` | tudo da aba principal + relés + registro (a cada 1 s) |
| `POST /api/cycle?cmd=start\|stop` | iniciar / parar tudo |
| `POST /api/reset` | limpar falha do Klixon |
| `GET /api/debug` | leituras cruas, pinos, sistema (a cada 2 s, só na aba admin) |
| `POST /api/relay?i=0..3&on=0\|1` | teste manual (409 com ciclo rodando) |
| `POST /api/water?v=0\|1` | marcar água manualmente |
| `GET/POST /api/config` | configuração do ciclo (POST: 409 com ciclo rodando; valores limitados com `constrain`) |
| `POST /api/config/defaults` | voltar aos padrões |
| `POST /api/log/clear` | limpar registro |
| `POST /api/restart` | reiniciar a placa (409 com ciclo rodando) |

Erros respondem `{"error": "…"}`; a página traduz para uma mensagem amigável no aviso (toast).

Não há autenticação: qualquer um na rede `LavaLoucas` (protegida por senha) ou na rede de casa acessa tudo.
