// event_log.h  -  registro dos últimos eventos (aparece na página e no Serial).
#pragma once

#include <Arduino.h>

const int LOG_SIZE = 20;

void addLog(const String& message);  // grava com o horário desde o boot (hh:mm:ss)
int logCount();                      // quantos eventos estão guardados
const String& logLineAt(int k);      // k = 0 é o mais antigo
void clearLog();
