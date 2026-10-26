// web_server.cpp  -  rotas HTTP. Cada handle...() lê os parâmetros, chama a função
// do módulo certo (ciclo, relés, água...) e responde em JSON.

#include "web_server.h"
#include <WebServer.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_system.h>
#include "../../config.h"
#include "../hardware/relays.h"
#include "../control/cycle.h"
#include "../control/safety.h"
#include "../control/water.h"
#include "../hardware/inputs.h"
#include "../storage/settings.h"
#include "../storage/event_log.h"
#include "network.h"
#include "page.h"

static WebServer server(80);
static const char* const API_HEADER = "X-Lava";  // cabeçalho que toda ação da página manda
static const char* const UPDATE_PASS_HEADER = "X-Senha";  // senha de atualização do programa
static uint32_t restartAtMs = 0;  // reinício agendado (dá tempo de responder à página)

// ---------- Ajudantes de JSON ----------

static String jsonBool(bool v) { return v ? "true" : "false"; }
static String jsonText(const String& s) { return "\"" + s + "\""; }

static void sendJson(const String& json) { server.send(200, "application/json", json); }
static void sendOk() { sendJson("{\"ok\":true}"); }
static void sendError(int code, const char* msg) { server.send(code, "application/json", "{\"error\":\"" + String(msg) + "\"}"); }

// ---------- Quem pode pedir ----------
// Sem isso, qualquer site aberto num celular da rede de casa poderia mandar, escondido,
// um POST para /api/relay e ligar uma carga de 110 V.

// O endereço digitado no navegador precisa ser o da placa. Bloqueia o "DNS rebinding"
// (um site que faz o próprio domínio apontar para o IP da placa).
static bool hostAllowed() {
  String host = server.hostHeader();
  int colon = host.indexOf(':');
  if (colon >= 0) host = host.substring(0, colon);
  host.toLowerCase();
  return host == WiFi.localIP().toString() || host == WiFi.softAPIP().toString() ||
         host == String(HOSTNAME) + ".local";
}

// Toda ação (POST) precisa do cabeçalho X-Lava, que a página manda. Outro site não consegue
// mandar esse cabeçalho sem a permissão da placa (CORS), e a placa nunca dá essa permissão.
static bool actionAllowed() {
  return hostAllowed() && server.header(API_HEADER) == "1";
}

// Envolve uma rota: recusa (403) o pedido que não passou na conferência acima.
static std::function<void()> guarded(void (*handler)(), bool isAction) {
  return [handler, isAction]() {
    if (!(isAction ? actionAllowed() : hostAllowed())) return sendError(403, "pedido recusado");
    handler();
  };
}

// ---------- Aba Principal ----------

static String stepLabel() {
  String label = STEP_NAME[currentStep];
  if (isRinseStep()) label += " " + String(rinseIndex + 1) + "/" + String(settings.nRinses);
  return label;
}

static void handleStatus() {
  uint32_t now = millis();
  String j = "{";
  j += "\"door\":"        + jsonBool(doorClosed());
  j += ",\"tankHot\":"    + jsonBool(tankHot());
  j += ",\"klixon\":"     + jsonBool(klixonTripped());
  j += ",\"klixonOnEsp\":" + jsonBool(KLIXON_ON_ESP);
  j += ",\"water\":"      + jsonBool(waterPresent);
  j += ",\"fillS\":"      + String(fillAccumMs / 1000.0, 1);
  j += ",\"active\":"     + jsonBool(cycleActive());
  j += ",\"paused\":"     + jsonBool(cyclePaused());
  j += ",\"stepId\":"     + String((int)currentStep);
  j += ",\"step\":"       + jsonText(stepLabel());
  j += ",\"rinse\":"      + String(rinseIndex);
  j += ",\"heating\":"    + jsonBool(currentStep == STEP_WASH && !washTempReached && !tankHot() && !heaterStoppedForStep());
  j += ",\"stepS\":"      + String(stepElapsedMs / 1000);
  j += ",\"stepTargetS\":" + String(stepTargetMs() / 1000);
  j += ",\"nRinses\":"    + String(settings.nRinses);
  j += ",\"preDrain\":"   + jsonBool(settings.preDrain);

  j += ",\"relays\":[";
  for (int i = 0; i < RELAY_COUNT; i++) {
    if (i) j += ",";
    j += "{\"name\":" + jsonText(RELAY_NAME[i]);
    j += ",\"on\":"    + jsonBool(relayOn[i]);
    j += ",\"test\":"  + jsonBool(testOn[i]);
    j += ",\"testLeftS\":" + String(testRemainingMs(i, now) / 1000);
    j += ",\"block\":" + jsonText(blockReason[i]) + "}";
  }
  j += "]";

  j += ",\"log\":[";
  for (int k = 0; k < logCount(); k++) j += (k ? "," : "") + jsonText(logLineAt(k));
  j += "]";

  j += ",\"ip\":"     + jsonText(networkAddresses());
  j += ",\"uptime\":" + String(now / 1000);
  j += "}";
  sendJson(j);
}

static void handleCycle() {
  if (server.arg("cmd") == "start") {
    startCycle();
  } else {
    stopCycle("pelo usuario");
    stopAllTests();
  }
  sendOk();
}

// ---------- Aba Debug/Admin ----------

static void handleRelay() {
  int i = server.arg("i").toInt();
  bool on = server.arg("on").toInt() == 1;
  if (i < 0 || i >= RELAY_COUNT) return sendError(400, "rele invalido");
  if (cycleActive())             return sendError(409, "ciclo em andamento");

  if (on) startTest(i);
  else    stopTest(i);
  sendOk();
}

static void handleWater() {
  setWaterManually(server.arg("v").toInt() == 1);
  sendOk();
}

static void handleConfigGet() {
  String j = "{";
  j += "\"fillSec\":"     + String(settings.fillSec);
  j += ",\"drainSec\":"   + String(settings.drainSec);
  j += ",\"washMin\":"    + String(settings.washMin);
  j += ",\"heatMaxMin\":" + String(settings.heatMaxMin);
  j += ",\"rinseMin\":"   + String(settings.rinseMin);
  j += ",\"nRinses\":"    + String(settings.nRinses);
  j += ",\"rinseHeat\":"  + jsonBool(settings.rinseHeat);
  j += ",\"preDrain\":"   + jsonBool(settings.preDrain);
  j += "}";
  sendJson(j);
}

static void handleConfigPost() {
  if (cycleActive()) return sendError(409, "ciclo em andamento");

  if (server.hasArg("fillSec"))    settings.fillSec    = constrain(server.arg("fillSec").toInt(), 5, 600);
  if (server.hasArg("drainSec"))   settings.drainSec   = constrain(server.arg("drainSec").toInt(), 5, 600);
  if (server.hasArg("washMin"))    settings.washMin    = constrain(server.arg("washMin").toInt(), 1, 120);
  if (server.hasArg("heatMaxMin")) settings.heatMaxMin = constrain(server.arg("heatMaxMin").toInt(), 1, 120);
  if (server.hasArg("rinseMin"))   settings.rinseMin   = constrain(server.arg("rinseMin").toInt(), 1, 60);
  if (server.hasArg("nRinses"))    settings.nRinses    = constrain(server.arg("nRinses").toInt(), 0, 5);
  if (server.hasArg("rinseHeat"))  settings.rinseHeat  = server.arg("rinseHeat").toInt() == 1;
  if (server.hasArg("preDrain"))   settings.preDrain   = server.arg("preDrain").toInt() == 1;
  saveSettings();
  sendOk();
}

static void handleConfigDefaults() {
  if (cycleActive()) return sendError(409, "ciclo em andamento");
  restoreDefaultSettings();
  sendOk();
}

static void handleLogClear() {
  clearLog();
  sendOk();
}

static void handleRestart() {
  if (cycleActive()) return sendError(409, "ciclo em andamento");
  addLog("Reiniciando pela pagina (debug e admin)");
  restartAtMs = millis() + 500;
  sendOk();
}

// Motivo do último reinício, em português.
static const char* resetReasonText() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:  return "ligou na energia";
    case ESP_RST_SW:       return "reinicio pelo software";
    case ESP_RST_PANIC:    return "travou (panic)";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:      return "watchdog";
    case ESP_RST_BROWNOUT: return "queda de tensao (brownout)";
    case ESP_RST_EXT:      return "botao reset";
    default:               return "outro";
  }
}

// Nome escrito na placa ESP32 DevKit para cada GPIO (GPIO32 = "D32").
static String pinLabel(uint8_t gpio) { return "D" + String(gpio); }

static String inputJson(const char* name, uint8_t pin, bool logical) {
  return "{\"name\":" + jsonText(name) + ",\"label\":" + jsonText(pinLabel(pin)) + ",\"gpio\":" + String(pin) +
         ",\"raw\":" + String(digitalRead(pin)) + ",\"value\":" + jsonBool(logical) + "}";
}

static void handleDebug() {
  String j = "{";
  j += "\"firmware\":"   + jsonText(String(__DATE__) + " " + __TIME__);
  j += ",\"chip\":"      + jsonText(String(ESP.getChipModel()) + " rev " + ESP.getChipRevision());
  j += ",\"cpuMHz\":"    + String(ESP.getCpuFreqMHz());
  j += ",\"sdk\":"       + jsonText(ESP.getSdkVersion());
  j += ",\"heapFree\":"  + String(ESP.getFreeHeap());
  j += ",\"heapMin\":"   + String(ESP.getMinFreeHeap());
  j += ",\"uptime\":"    + String(millis() / 1000);
  j += ",\"resetReason\":" + jsonText(resetReasonText());
  j += ",\"wifiHome\":"  + jsonBool(WiFi.status() == WL_CONNECTED);
  j += ",\"rssi\":"      + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
  j += ",\"apClients\":" + String(WiFi.softAPgetStationNum());
  j += ",\"ip\":"        + jsonText(networkAddresses());
  j += ",\"relayActiveHigh\":" + jsonBool(RELAY_ACTIVE_LEVEL == HIGH);
  j += ",\"updateEnabled\":" + jsonBool(updateEnabled());

  j += ",\"inputs\":[";
  j += inputJson("Porta (fechada)", PIN_DOOR, doorClosed()) + ",";
  j += inputJson("Termostato da cuba (quente)", PIN_TANK_THERMOSTAT, tankHot());
  if (KLIXON_ON_ESP) j += "," + inputJson("Klixon (disparado)", PIN_KLIXON, klixonTripped());
  j += "]";

  j += ",\"relays\":[";
  for (int i = 0; i < RELAY_COUNT; i++) {
    if (i) j += ",";
    j += "{\"name\":" + jsonText(RELAY_NAME[i]) + ",\"label\":" + jsonText(pinLabel(RELAY_PIN[i])) + ",\"gpio\":" + String(RELAY_PIN[i]) +
         ",\"on\":" + jsonBool(relayOn[i]) + ",\"maxTestS\":" + String(TEST_MAX_MS[i] / 1000) + "}";
  }
  j += "]}";
  sendJson(j);
}

// ---------- Atualizar o programa pela página ----------
// O .bin chega em pedaços em updateUpload(); updateDone() responde no fim. Tudo é conferido
// no começo do envio, antes de gravar qualquer coisa: endereço, X-Lava, senha e ciclo parado.
// Update.end(true) só troca o programa se o arquivo chegou inteiro e é válido; se não,
// a placa continua com o programa antigo.

static bool   updateStarted = false;
static String updateError;  // vazio = tudo certo até aqui

static void updateUpload() {
  if (!server.hasUpload()) return;
  HTTPUpload& up = server.upload();

  if (up.status == UPLOAD_FILE_START) {
    updateStarted = true;
    updateError = "";
    if (!actionAllowed())                                           updateError = "pedido recusado";
    else if (!updateEnabled())                                      updateError = "atualizacao desligada";
    else if (!updatePasswordOk(server.header(UPDATE_PASS_HEADER)))  updateError = "senha errada";
    else if (cycleActive())                                         updateError = "ciclo em andamento";
    if (updateError.length()) return;

    stopAllTests();
    forceAllRelaysOff();  // nada fica ligado durante a gravação e o reinício
    addLog("Recebendo programa novo");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) updateError = Update.errorString();
  } else if (updateError.length()) {
    return;  // recusado ou já falhou: ignora o resto do arquivo
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (Update.write(up.buf, up.currentSize) != up.currentSize) {
      updateError = Update.errorString();
      Update.abort();
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (!Update.end(true)) updateError = Update.errorString();
  } else if (up.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    updateError = "envio interrompido";
  }
}

static void updateDone() {
  if (!updateStarted) updateError = "arquivo nao enviado";
  updateStarted = false;
  server.sendHeader("Connection", "close");

  if (updateError.length()) {
    int code = updateError == "ciclo em andamento" ? 409
             : (updateError == "pedido recusado" || updateError == "senha errada" ||
                updateError == "atualizacao desligada") ? 403 : 500;
    addLog("Atualizacao recusada: " + updateError);
    return sendError(code, updateError.c_str());
  }
  addLog("Programa atualizado, reiniciando");
  sendOk();
  restartAtMs = millis() + 500;
}

// ---------- Setup / loop ----------

void setupWebServer() {
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", PAGE); });

  // Leituras (GET): só conferem o endereço. Ações (POST): endereço + cabeçalho X-Lava.
  server.on("/api/status", HTTP_GET,  guarded(handleStatus, false));
  server.on("/api/cycle",  HTTP_POST, guarded(handleCycle, true));

  server.on("/api/debug",           HTTP_GET,  guarded(handleDebug, false));
  server.on("/api/relay",           HTTP_POST, guarded(handleRelay, true));
  server.on("/api/water",           HTTP_POST, guarded(handleWater, true));
  server.on("/api/config",          HTTP_GET,  guarded(handleConfigGet, false));
  server.on("/api/config",          HTTP_POST, guarded(handleConfigPost, true));
  server.on("/api/config/defaults", HTTP_POST, guarded(handleConfigDefaults, true));
  server.on("/api/log/clear",       HTTP_POST, guarded(handleLogClear, true));
  server.on("/api/restart",         HTTP_POST, guarded(handleRestart, true));
  // A conferência desta rota fica em updateUpload(), que roda antes de updateDone().
  server.on("/api/update",          HTTP_POST, updateDone, updateUpload);

  // Por padrão o WebServer só guarda alguns cabeçalhos; X-Lava e X-Senha precisam ser pedidos.
  static const char* headerKeys[] = {API_HEADER, UPDATE_PASS_HEADER};
  server.collectHeaders(headerKeys, 2);
  server.begin();
}

void handleWebClients() {
  server.handleClient();

  if (restartAtMs && millis() >= restartAtMs) {
    forceAllRelaysOff();
    ESP.restart();
  }
}
