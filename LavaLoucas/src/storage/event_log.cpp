// event_log.cpp  -  fila circular com os últimos LOG_SIZE eventos.

#include "event_log.h"

static String lines[LOG_SIZE];
static int head = 0;   // onde entra o próximo
static int count = 0;

void addLog(const String& message) {
  unsigned long t = millis() / 1000;
  char timestamp[16];
  snprintf(timestamp, sizeof(timestamp), "%02lu:%02lu:%02lu ", t / 3600, (t / 60) % 60, t % 60);

  String line = String(timestamp) + message;
  line.replace("\"", "'");  // não quebrar o JSON da página

  lines[head] = line;
  head = (head + 1) % LOG_SIZE;
  if (count < LOG_SIZE) count++;
  Serial.println(line);
}

int logCount() { return count; }

const String& logLineAt(int k) {
  return lines[(head - count + k + LOG_SIZE) % LOG_SIZE];
}

void clearLog() {
  head = count = 0;
  addLog("Registro limpo");
}
