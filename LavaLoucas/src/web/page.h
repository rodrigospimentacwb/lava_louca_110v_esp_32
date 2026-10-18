// page.h  -  página web servida em "/". Duas abas:
//   Principal      dial do ciclo, iniciar/parar, sensores e cargas ligadas
//   Debug e admin  teste de componentes, configuração, leituras cruas, sistema e registro
//
// A página roda na placa sem internet (o celular fica na rede LavaLoucas), então
// nada de fontes ou bibliotecas externas: tudo está aqui dentro.
// O HTML fica num raw string: tudo entre R"HTML( e )HTML" é enviado como está.
#pragma once

const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-br">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#E8ECEF">
<link rel="icon" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16'%3E%3Ccircle cx='8' cy='8' r='6' fill='none' stroke='%231E5C9E' stroke-width='3'/%3E%3C/svg%3E">
<title>Lava-louças</title>
<style>
  :root {
    --panel: #E8ECEF;      /* painel do eletrodoméstico */
    --surface: #F7F9FA;
    --line: #CDD5DC;
    --ink: #14324A;        /* texto principal */
    --ink-soft: #5B6F80;
    --fill: #3D9BE0;       /* encher */
    --wash: #1E5C9E;       /* lavar / enxaguar */
    --drain: #8C99A6;      /* drenar */
    --heat: #E0482D;       /* resistência e falha */
    --on: #2F9A5C;         /* ligado / ok */
    --warn: #B7791F;
    --focus: #1E5C9E;
    --radius-s: 8px;
    --radius-l: 18px;
  }
  @media (prefers-color-scheme: dark) {
    :root {
      --panel: #142029; --surface: #1B2A35; --line: #2C3E4C;
      --ink: #E4EDF3; --ink-soft: #93A6B5;
      --fill: #5BB0EE; --wash: #4E8FD6; --drain: #6E7E8C;
      --heat: #F06A4F; --on: #4CC07D; --warn: #E0A84A; --focus: #5BB0EE;
    }
  }

  * { box-sizing: border-box }
  body {
    margin: 0; background: var(--panel); color: var(--ink);
    font: 16px/1.45 ui-rounded, "SF Pro Rounded", system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
    -webkit-tap-highlight-color: transparent;
  }
  main { max-width: 560px; margin: 0 auto; padding: 12px 16px 48px }
  h1 { font-size: 1.15rem; margin: 0; font-weight: 700 }
  h2 { font-size: 1.05rem; margin: 0 0 10px; font-weight: 700 }
  p { margin: 0 }
  .soft { color: var(--ink-soft) }
  .num { font-variant-numeric: tabular-nums }
  :focus-visible { outline: 3px solid var(--focus); outline-offset: 2px }

  /* ---------- Topo e abas ---------- */
  header { display: flex; align-items: center; justify-content: space-between; gap: 12px; padding: 4px 0 12px }
  .conn { font-size: .85rem; color: var(--ink-soft); display: flex; align-items: center; gap: 6px }
  .conn i { width: 8px; height: 8px; border-radius: 50%; background: var(--on) }
  .conn.lost i { background: var(--heat) }
  .tabs { display: grid; grid-template-columns: 1fr 1fr; background: var(--line); border-radius: 12px; padding: 3px; margin-bottom: 16px }
  .tabs button {
    border: 0; background: transparent; color: var(--ink-soft); font: inherit; font-weight: 600;
    padding: 9px 8px; border-radius: 10px; cursor: pointer;
  }
  .tabs button[aria-selected=true] { background: var(--surface); color: var(--ink); box-shadow: 0 1px 2px #0002 }

  /* ---------- Botões ---------- */
  .btn {
    border: 0; border-radius: var(--radius-s); padding: 10px 16px; font: inherit; font-weight: 600;
    cursor: pointer; background: var(--wash); color: #fff; min-height: 44px;
  }
  .btn.ghost { background: transparent; color: var(--ink); box-shadow: inset 0 0 0 1.5px var(--line) }
  .btn.stop { background: var(--heat) }
  .btn.on { background: var(--on) }
  .btn:disabled { opacity: .45; cursor: not-allowed }
  .btn.big { width: 100%; font-size: 1.1rem; min-height: 56px; border-radius: 14px }

  /* ---------- Falha ---------- */
  .fault {
    border-left: 5px solid var(--heat); background: var(--surface); border-radius: var(--radius-s);
    padding: 12px 14px; margin-bottom: 16px; display: grid; gap: 10px;
  }
  .fault strong { color: var(--heat) }

  /* ---------- Dial (peça principal) ---------- */
  .dial-wrap { position: relative; width: min(100%, 340px); margin: 0 auto }
  .dial-wrap svg { display: block; width: 100%; height: auto }
  .dial-center {
    position: absolute; inset: 24%; display: flex; flex-direction: column;
    align-items: center; justify-content: center; text-align: center; gap: 2px;
  }
  .dial-step { font-size: 1.35rem; font-weight: 700; line-height: 1.15 }
  .dial-time { font-size: 1.05rem }
  .dial-note { font-size: .85rem; font-weight: 600; min-height: 1.2em }
  .dial-note.heat { color: var(--heat) }
  .dial-note.warn { color: var(--warn) }
  .ticks line { stroke: var(--line); stroke-width: 1.5 }
  .ticks line.major { stroke: var(--ink-soft); stroke-width: 2 }
  .needle { fill: var(--ink); transition: transform .6s ease }
  @media (prefers-reduced-motion: reduce) { .needle { transition: none } }

  .legend { display: flex; justify-content: center; gap: 16px; font-size: .85rem; color: var(--ink-soft); margin: 6px 0 16px }
  .legend span::before { content: ""; display: inline-block; width: 10px; height: 10px; border-radius: 3px; margin-right: 6px; vertical-align: -1px; background: var(--c) }

  .actions { display: grid; gap: 8px; margin-bottom: 8px }
  .hint { text-align: center; font-size: .9rem; color: var(--ink-soft); min-height: 1.3em; margin-bottom: 20px }

  /* ---------- Sensores e cargas (aba principal) ---------- */
  .facts { display: grid; grid-template-columns: 1fr 1fr; gap: 1px; background: var(--line); border: 1px solid var(--line); border-radius: var(--radius-l); overflow: hidden; margin-bottom: 20px }
  .fact { background: var(--surface); padding: 12px 14px }
  .fact p:first-child { font-size: .85rem; color: var(--ink-soft) }
  .fact p:last-child { font-weight: 700 }
  .fact .ok { color: var(--on) } .fact .bad { color: var(--heat) } .fact .warn { color: var(--warn) }

  .loads { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; text-align: center; font-size: .8rem; color: var(--ink-soft) }
  .load i { display: block; width: 14px; height: 14px; border-radius: 50%; margin: 0 auto 6px; background: var(--line); box-shadow: inset 0 0 0 2px var(--line) }
  .load.lit { color: var(--ink); font-weight: 600 }
  .load.lit i { background: var(--on); box-shadow: 0 0 0 4px color-mix(in srgb, var(--on) 25%, transparent) }
  .load.lit.hot i { background: var(--heat); box-shadow: 0 0 0 4px color-mix(in srgb, var(--heat) 25%, transparent) }

  /* ---------- Aba debug e admin ---------- */
  section.admin { background: var(--surface); border: 1px solid var(--line); border-radius: var(--radius-l); padding: 16px; margin-bottom: 14px }
  .admin > p.soft { font-size: .9rem; margin: -4px 0 12px }
  .row { display: flex; align-items: center; justify-content: space-between; gap: 12px; padding: 10px 0; border-top: 1px solid var(--line) }
  .row:first-of-type { border-top: 0 }
  .row small { display: block; font-size: .8rem; color: var(--ink-soft) }
  .row small.blocked { color: var(--heat) }
  .row .btn { min-width: 104px }
  .state { font-size: .85rem; font-weight: 700; color: var(--ink-soft) }
  .state.lit { color: var(--on) }

  form.cfg { display: grid; gap: 2px }
  form.cfg label { display: flex; align-items: center; justify-content: space-between; gap: 12px; padding: 8px 0; border-top: 1px solid var(--line) }
  form.cfg label:first-child { border-top: 0 }
  form.cfg .unit { display: flex; align-items: center; gap: 6px; color: var(--ink-soft); font-size: .9rem }
  form.cfg .u { width: 28px; font-style: normal }
  input[type=number] {
    width: 84px; padding: 8px; font: inherit; text-align: right; color: var(--ink);
    background: var(--panel); border: 1.5px solid var(--line); border-radius: var(--radius-s);
  }
  input[type=checkbox] { width: 22px; height: 22px; accent-color: var(--wash) }
  .form-actions { display: flex; gap: 8px; margin-top: 12px; flex-wrap: wrap }

  table { width: 100%; border-collapse: collapse; font-size: .9rem }
  th, td { text-align: left; padding: 7px 4px; border-top: 1px solid var(--line) }
  th { color: var(--ink-soft); font-weight: 600; border-top: 0 }
  td.r, th.r { text-align: right }

  dl { display: grid; grid-template-columns: auto 1fr; gap: 6px 16px; margin: 0; font-size: .9rem }
  dt { color: var(--ink-soft) }
  dd { margin: 0; text-align: right; font-weight: 600 }

  .log { font: .8rem/1.5 ui-monospace, Menlo, Consolas, monospace; white-space: pre-wrap; max-height: 260px; overflow: auto; background: var(--panel); border-radius: var(--radius-s); padding: 10px; margin-bottom: 10px }

  .toast {
    position: fixed; left: 50%; bottom: 20px; transform: translate(-50%, 20px); opacity: 0;
    background: var(--ink); color: var(--panel); padding: 10px 16px; border-radius: 10px;
    font-weight: 600; transition: .25s; pointer-events: none; max-width: calc(100% - 32px);
  }
  .toast.show { opacity: 1; transform: translate(-50%, 0) }
  [hidden] { display: none !important }
</style>
</head>
<body>
<main>
  <header>
    <h1>Lava-louças</h1>
    <span class="conn" id="conn"><i></i><span id="connText">conectando…</span></span>
  </header>

  <div class="tabs" role="tablist">
    <button role="tab" id="tab-main" aria-controls="panel-main" aria-selected="true">Principal</button>
    <button role="tab" id="tab-admin" aria-controls="panel-admin" aria-selected="false">Debug e admin</button>
  </div>

  <div class="fault" id="fault" role="alert" hidden>
    <p><strong>Sobretemperatura na resistência.</strong> O Klixon disparou e tudo foi desligado.
       Espere a resistência esfriar e limpe a falha.</p>
    <button class="btn stop" data-post="/api/reset">Limpar falha</button>
  </div>

  <!-- ================= Principal ================= -->
  <div role="tabpanel" id="panel-main" aria-labelledby="tab-main">
    <div class="dial-wrap">
      <svg viewBox="0 0 300 300" id="dial" role="img" aria-label="Ciclo de lavagem">
        <g class="ticks" id="ticks"></g>
        <g id="segments"></g>
        <g id="needleWrap"><path class="needle" id="needle" d="M150 27 L142 12 L158 12 Z"/></g>
      </svg>
      <div class="dial-center">
        <p class="dial-step" id="stepName">—</p>
        <p class="dial-time num soft" id="stepTime"></p>
        <p class="dial-note" id="stepNote"></p>
      </div>
    </div>
    <div class="legend">
      <span style="--c:var(--fill)">Encher</span>
      <span style="--c:var(--wash)">Lavar</span>
      <span style="--c:var(--drain)">Drenar</span>
    </div>

    <div class="actions">
      <button class="btn big" id="startBtn" data-post="/api/cycle?cmd=start">Iniciar lavagem</button>
      <button class="btn big stop" id="stopBtn" data-post="/api/cycle?cmd=stop" hidden>Parar tudo</button>
    </div>
    <p class="hint" id="hint"></p>

    <div class="facts">
      <div class="fact"><p>Porta</p><p id="fDoor">—</p></div>
      <div class="fact"><p>Água na cuba</p><p id="fWater">—</p></div>
      <div class="fact"><p>Temperatura da água</p><p id="fTank">—</p></div>
      <div class="fact"><p>Proteção da resistência</p><p id="fKlixon">—</p></div>
    </div>

    <div class="loads" id="loads" aria-label="Cargas ligadas agora"></div>
  </div>

  <!-- ================= Debug e admin ================= -->
  <div role="tabpanel" id="panel-admin" aria-labelledby="tab-admin" hidden>

    <section class="admin">
      <h2>Testar componentes</h2>
      <p class="soft" id="testNote">Cada teste desliga sozinho no tempo máximo. A resistência só liga com água que entrou pelo solenoide.</p>
      <div id="tests"></div>
      <div class="row">
        <span>Água na cuba<small id="waterInfo"></small></span>
        <button class="btn ghost" id="waterBtn">Marcar</button>
      </div>
    </section>

    <section class="admin">
      <h2>Configuração do ciclo</h2>
      <p class="soft">Fica salva na placa, mesmo desligando da tomada.</p>
      <form class="cfg" id="cfgForm">
        <label>Enchimento <span class="unit"><input type="number" name="fillSec" min="5" max="600"><i class="u">s</i></span></label>
        <label>Drenagem <span class="unit"><input type="number" name="drainSec" min="5" max="600"><i class="u">s</i></span></label>
        <label>Lavagem mínima <span class="unit"><input type="number" name="washMin" min="1" max="120"><i class="u">min</i></span></label>
        <label>Máximo esperando aquecer <span class="unit"><input type="number" name="heatMaxMin" min="1" max="120"><i class="u">min</i></span></label>
        <label>Cada enxágue <span class="unit"><input type="number" name="rinseMin" min="1" max="60"><i class="u">min</i></span></label>
        <label>Quantos enxágues <span class="unit"><input type="number" name="nRinses" min="0" max="5"><i class="u"></i></span></label>
        <label>Aquecer o último enxágue <input type="checkbox" name="rinseHeat"></label>
        <label>Drenar antes de começar <input type="checkbox" name="preDrain"></label>
        <div class="form-actions">
          <button class="btn" type="submit">Salvar configuração</button>
          <button class="btn ghost" type="button" id="defaultsBtn">Voltar aos padrões</button>
        </div>
      </form>
    </section>

    <section class="admin">
      <h2>Sensores (leitura crua)</h2>
      <p class="soft">HIGH com o contato aberto, LOW com o contato fechado no GND.</p>
      <table>
        <thead><tr><th>Sensor</th><th class="r">Pino</th><th class="r">Nível</th><th class="r">Leitura</th></tr></thead>
        <tbody id="rawInputs"></tbody>
      </table>
    </section>

    <section class="admin">
      <h2>Relés</h2>
      <table>
        <thead><tr><th>Carga</th><th class="r">Pino</th><th class="r">Teste máx.</th><th class="r">Estado</th></tr></thead>
        <tbody id="rawRelays"></tbody>
      </table>
      <p class="soft" style="margin-top:10px" id="activeLevel"></p>
    </section>

    <section class="admin">
      <h2>Sistema</h2>
      <dl id="sys"></dl>
      <div class="form-actions">
        <button class="btn ghost" id="restartBtn">Reiniciar a placa</button>
      </div>
    </section>

    <section class="admin">
      <h2>Registro</h2>
      <div class="log" id="log" aria-live="off"></div>
      <button class="btn ghost" data-post="/api/log/clear">Limpar registro</button>
    </section>
  </div>
</main>

<div class="toast" id="toast" role="status" aria-live="polite"></div>

<script>
  const $ = id => document.getElementById(id);
  let S = null;        // último /api/status
  let CFG = null;      // última /api/config
  let adminOpen = false;

  // ---------- Conversa com a placa ----------

  const ERRORS = {
    'ciclo em andamento': 'Pare o ciclo antes de fazer isso.',
    'rele invalido': 'Relé inválido.',
    'pedido recusado': 'A placa recusou o pedido. Abra a página pelo endereço da placa.'
  };

  async function post(url, okMsg) {
    try {
      const r = await fetch(url, { method: 'POST', headers: { 'X-Lava': '1' } });  // sem ele a placa recusa (403)
      const j = await r.json().catch(() => ({}));
      if (!r.ok) toast(ERRORS[j.error] || 'A placa recusou o pedido.');
      else if (okMsg) toast(okMsg);
    } catch (e) {
      toast('Sem conexão com a lava-louças.');
    }
    loadStatus();
  }

  function toast(msg) {
    const t = $('toast');
    t.textContent = msg;
    t.classList.add('show');
    clearTimeout(toast.timer);
    toast.timer = setTimeout(() => t.classList.remove('show'), 2600);
  }

  // ---------- Utilidades ----------

  function mmss(s) {
    s = Math.max(0, Math.round(s));
    const h = Math.floor(s / 3600), m = Math.floor(s / 60) % 60, ss = String(s % 60).padStart(2, '0');
    return h ? h + ':' + String(m).padStart(2, '0') + ':' + ss : m + ':' + ss;
  }
  function minutes(s) { return Math.max(1, Math.round(s / 60)) + ' min'; }
  function setText(el, txt, cls) { el.textContent = txt; el.className = cls || ''; }

  // ---------- Dial: o antigo timer redondo ----------
  // Monta a lista de etapas a partir da configuração. Ids iguais ao enum Step do firmware (cycle.h).

  function plannedSteps() {
    if (!CFG) return [];
    const st = [];
    if (CFG.preDrain) st.push({ id: 1, kind: 'drain', dur: CFG.drainSec });
    st.push({ id: 2, kind: 'fill', dur: CFG.fillSec });
    st.push({ id: 3, kind: 'wash', dur: CFG.washMin * 60 });
    st.push({ id: 4, kind: 'drain', dur: CFG.drainSec });
    for (let r = 0; r < CFG.nRinses; r++) {
      st.push({ id: 5, kind: 'fill', dur: CFG.fillSec, rinse: r });
      st.push({ id: 6, kind: 'wash', dur: CFG.rinseMin * 60, rinse: r });
      st.push({ id: 7, kind: 'drain', dur: CFG.drainSec, rinse: r });
    }
    return st;
  }

  // Cada etapa ganha um ângulo proporcional ao tempo, com um mínimo para as curtas aparecerem.
  function layoutSteps(st) {
    const GAP = 2, MIN = 9;
    const avail = 360 - GAP * st.length;
    const total = st.reduce((a, s) => a + s.dur, 0) || 1;
    const extra = Math.max(0, avail - MIN * st.length);
    let a = GAP / 2;
    st.forEach(s => { s.a0 = a; s.a1 = a + MIN + extra * s.dur / total; a = s.a1 + GAP; });
    return st;
  }

  function polar(angle, r) {
    const t = (angle - 90) * Math.PI / 180;
    return [150 + r * Math.cos(t), 150 + r * Math.sin(t)];
  }
  function arc(a0, a1, r) {
    const [x0, y0] = polar(a0, r), [x1, y1] = polar(a1, r);
    return `M${x0.toFixed(2)} ${y0.toFixed(2)}A${r} ${r} 0 ${a1 - a0 > 180 ? 1 : 0} 1 ${x1.toFixed(2)} ${y1.toFixed(2)}`;
  }

  function drawTicks() {
    let h = '';
    for (let i = 0; i < 60; i++) {
      const major = i % 5 === 0;
      const [x0, y0] = polar(i * 6, 148), [x1, y1] = polar(i * 6, major ? 140 : 144);
      h += `<line x1="${x0}" y1="${y0}" x2="${x1}" y2="${y1}"${major ? ' class="major"' : ''}/>`;
    }
    $('ticks').innerHTML = h;
  }

  function currentIndex(st) {
    if (!S || !S.active) return -1;
    return st.findIndex(s => s.id === S.stepId && (s.rinse === undefined || s.rinse === S.rinse));
  }

  function drawDial() {
    const st = layoutSteps(plannedSteps());
    const cur = currentIndex(st);
    const frac = S && S.stepTargetS ? Math.min(1, S.stepS / S.stepTargetS) : 0;
    const R = 112, W = 24;
    let h = '';
    st.forEach((s, i) => {
      const color = `var(--${s.kind})`;
      const done = cur >= 0 && i < cur;
      h += `<path d="${arc(s.a0, s.a1, R)}" stroke="${color}" stroke-width="${W}" fill="none" opacity="${done ? 1 : .28}"/>`;
      if (i === cur && frac > 0) {
        h += `<path d="${arc(s.a0, s.a0 + Math.max(.5, (s.a1 - s.a0) * frac), R)}" stroke="${color}" stroke-width="${W}" fill="none"/>`;
      }
    });
    $('segments').innerHTML = h;

    // Ponteiro na posição atual (no começo quando parado)
    let angle = 0;
    if (cur >= 0) angle = st[cur].a0 + (st[cur].a1 - st[cur].a0) * frac;
    $('needle').style.transform = `rotate(${angle}deg)`;
    $('needle').style.transformOrigin = '150px 150px';

    // Tempo restante estimado (a lavagem pode passar disso esperando aquecer)
    const totalS = st.reduce((a, s) => a + s.dur, 0);
    let leftS = totalS;
    if (cur >= 0) leftS = Math.max(0, st[cur].dur - S.stepS) + st.slice(cur + 1).reduce((a, s) => a + s.dur, 0);
    return { totalS, leftS };
  }

  // ---------- Aba principal ----------

  function renderMain() {
    const { totalS, leftS } = drawDial();
    const note = $('stepNote');

    if (!S.active) {
      $('stepName').textContent = S.fault ? 'Em falha' : 'Pronta';
      $('stepTime').textContent = CFG ? 'cerca de ' + minutes(totalS) : '';
      setText(note, '', 'dial-note');
    } else {
      $('stepName').textContent = S.step;
      $('stepTime').textContent = mmss(S.stepS) + ' / ' + mmss(S.stepTargetS);
      if (S.paused) setText(note, 'Pausado: porta aberta', 'dial-note warn');
      else if (S.relays[0].on) setText(note, 'Aquecendo a água', 'dial-note heat');
      else setText(note, '', 'dial-note');
    }
    $('dial').setAttribute('aria-label', S.active ? `Etapa ${S.step}, ${mmss(S.stepS)} de ${mmss(S.stepTargetS)}` : 'Ciclo parado');

    $('startBtn').hidden = S.active;
    $('stopBtn').hidden = !S.active;
    $('startBtn').disabled = !S.door || S.fault;

    let hint = '';
    if (S.fault) hint = 'Limpe a falha para lavar de novo.';
    else if (!S.active && !S.door) hint = 'Feche a porta para iniciar.';
    else if (S.active) {
      const end = new Date(Date.now() + leftS * 1000);
      hint = 'Faltam cerca de ' + minutes(leftS) + ', termina por volta das ' +
             end.toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit' }) + '.';
    }
    $('hint').textContent = hint;

    setText($('fDoor'), S.door ? 'Fechada' : 'Aberta', S.door ? 'ok' : 'bad');
    setText($('fWater'), S.water ? 'Com água' : 'Vazia', '');
    setText($('fTank'), S.tankHot ? 'Quente' : 'Fria', S.tankHot ? 'warn' : '');
    setText($('fKlixon'), S.klixon ? 'Disparada' : 'Normal', S.klixon ? 'bad' : 'ok');

    const short = ['Resistência', 'Lavagem', 'Drenagem', 'Entrada'];
    $('loads').innerHTML = S.relays.map((r, i) =>
      `<div class="load${r.on ? ' lit' : ''}${r.on && i === 0 ? ' hot' : ''}"><i></i>${short[i]}</div>`).join('');
  }

  // ---------- Aba debug e admin ----------

  function renderAdmin() {
    $('testNote').textContent = S.active
      ? 'Pare o ciclo para testar os componentes.'
      : 'Cada teste desliga sozinho no tempo máximo. A resistência só liga com água que entrou pelo solenoide.';

    $('tests').innerHTML = S.relays.map((r, i) => {
      const state = r.on ? 'Ligado' + (r.test ? ', desliga sozinho em ' + mmss(r.testLeftS) : '') : 'Desligado';
      const block = r.block ? `<small class="blocked">Bloqueado: ${r.block}</small>` : '';
      return `<div class="row">
        <span>${r.name}<small class="state${r.on ? ' lit' : ''}">${state}</small>${block}</span>
        <button class="btn${r.test ? ' on' : ''}" ${S.active ? 'disabled' : ''}
          data-post="/api/relay?i=${i}&on=${r.test ? 0 : 1}">${r.test ? 'Desligar' : 'Ligar'}</button>
      </div>`;
    }).join('');

    $('waterInfo').textContent = (S.water ? 'Com água' : 'Vazia') + ' (estimado). Solenoide aberto: ' + S.fillS.toFixed(1) + ' s';
    $('waterBtn').textContent = S.water ? 'Marcar vazia' : 'Marcar com água';
    $('waterBtn').dataset.post = '/api/water?v=' + (S.water ? 0 : 1);

    const log = $('log');
    const atBottom = log.scrollTop + log.clientHeight >= log.scrollHeight - 4;
    log.textContent = S.log.length ? S.log.join('\n') : 'Nada registrado ainda.';
    if (atBottom) log.scrollTop = log.scrollHeight;
  }

  function renderDebug(d) {
    $('rawInputs').innerHTML = d.inputs.map(x =>
      `<tr><td>${x.name}</td><td class="r num">${x.label}</td><td class="r">${x.raw ? 'HIGH' : 'LOW'}</td><td class="r">${x.value ? 'sim' : 'não'}</td></tr>`).join('');
    $('rawRelays').innerHTML = d.relays.map(x =>
      `<tr><td>${x.name}</td><td class="r num">${x.label}</td><td class="r num">${mmss(x.maxTestS)}</td><td class="r">${x.on ? 'ligado' : 'desligado'}</td></tr>`).join('');
    $('activeLevel').textContent = 'Relés ligam com nível ' + (d.relayActiveHigh ? 'HIGH' : 'LOW') + ' (RELAY_ACTIVE_LEVEL no config.h).';

    const kb = b => (b / 1024).toFixed(0) + ' KB';
    const rows = [
      ['Firmware compilado em', d.firmware],
      ['Chip', d.chip + ', ' + d.cpuMHz + ' MHz'],
      ['Versão do ESP-IDF', d.sdk],
      ['Ligado há', mmss(d.uptime)],
      ['Último reinício', d.resetReason],
      ['Memória livre', kb(d.heapFree) + ' (mínimo ' + kb(d.heapMin) + ')'],
      ['Wi-Fi de casa', d.wifiHome ? 'conectado, sinal ' + d.rssi + ' dBm' : 'não conectado'],
      ['Aparelhos na rede LavaLoucas', d.apClients],
      ['Endereços', d.ip]
    ];
    $('sys').innerHTML = rows.map(([k, v]) => `<dt>${k}</dt><dd class="num">${v}</dd>`).join('');
  }

  // ---------- Carregar dados ----------

  async function loadStatus() {
    try {
      S = await (await fetch('/api/status')).json();
      $('conn').className = 'conn';
      $('connText').textContent = 'ligada há ' + mmss(S.uptime);
      $('fault').hidden = !S.fault;
      renderMain();
      if (adminOpen) renderAdmin();
    } catch (e) {
      $('conn').className = 'conn lost';
      $('connText').textContent = 'sem conexão';
    }
  }

  async function loadConfig() {
    try {
      CFG = await (await fetch('/api/config')).json();
      const f = $('cfgForm');
      for (const k in CFG) {
        const el = f.elements[k];
        if (!el) continue;
        if (el.type === 'checkbox') el.checked = CFG[k]; else el.value = CFG[k];
      }
      if (S) renderMain();
    } catch (e) {}
  }

  async function loadDebug() {
    if (!adminOpen) return;
    try { renderDebug(await (await fetch('/api/debug')).json()); } catch (e) {}
  }

  // ---------- Abas ----------

  function openTab(admin) {
    adminOpen = admin;
    $('tab-main').setAttribute('aria-selected', !admin);
    $('tab-admin').setAttribute('aria-selected', admin);
    $('panel-main').hidden = admin;
    $('panel-admin').hidden = !admin;
    history.replaceState(null, '', admin ? '#admin' : '#');
    if (admin) { if (S) renderAdmin(); loadDebug(); }
  }
  $('tab-main').onclick = () => openTab(false);
  $('tab-admin').onclick = () => openTab(true);

  // ---------- Ações ----------

  // Qualquer botão com data-post faz um POST para aquela rota.
  document.addEventListener('click', e => {
    const b = e.target.closest('[data-post]');
    if (b && !b.disabled) post(b.dataset.post);
  });

  $('cfgForm').onsubmit = async e => {
    e.preventDefault();
    const f = e.target, q = new URLSearchParams();
    ['fillSec', 'drainSec', 'washMin', 'heatMaxMin', 'rinseMin', 'nRinses'].forEach(k => q.set(k, f.elements[k].value));
    q.set('rinseHeat', f.elements.rinseHeat.checked ? 1 : 0);
    q.set('preDrain', f.elements.preDrain.checked ? 1 : 0);
    await post('/api/config?' + q, 'Configuração salva.');
    loadConfig();
  };

  $('defaultsBtn').onclick = async () => {
    if (!confirm('Voltar todos os tempos do ciclo para os valores padrão?')) return;
    await post('/api/config/defaults', 'Configuração voltou aos padrões.');
    loadConfig();
  };

  $('restartBtn').onclick = () => {
    if (confirm('Reiniciar a placa? Todos os relés desligam e a página volta em alguns segundos.'))
      post('/api/restart', 'Reiniciando…');
  };

  // ---------- Início ----------

  drawTicks();
  openTab(location.hash === '#admin');
  loadConfig();
  loadStatus();
  setInterval(loadStatus, 1000);
  setInterval(loadDebug, 2000);
</script>
</body>
</html>)HTML";
