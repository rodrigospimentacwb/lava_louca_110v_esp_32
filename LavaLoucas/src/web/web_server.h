// web_server.h  -  servidor HTTP: a página (page.h) e a API JSON que ela usa.
//
//   Aba Principal                    Aba Debug/Admin
//   GET  /api/status                 GET  /api/debug
//   POST /api/cycle?cmd=start|stop   POST /api/relay?i=0..3&on=0|1
//   POST /api/reset                  POST /api/water?v=0|1
//                                    GET/POST /api/config
//                                    POST /api/config/defaults
//                                    POST /api/log/clear
//                                    POST /api/restart
#pragma once

void setupWebServer();     // no setup(), depois do Wi-Fi
void handleWebClients();   // a cada volta do loop()
