// ============================================================
//  DebugLogger.cpp  -  lib/DebugLogger/DebugLogger.cpp
// ============================================================
#include "DebugLogger.h"
#include <stdarg.h>

static const char LOGGER_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="it"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Semenzaio Debug</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#1a1a2e;color:#eee;font-family:'Courier New',monospace;font-size:13px;height:100vh;display:flex;flex-direction:column}
.hdr{background:#16213e;padding:10px 16px;display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid #0f3460;flex-shrink:0}
.hdr h1{font-size:15px;color:#4ecca3}
.st{display:flex;align-items:center;gap:8px;font-size:12px}
.dot{width:8px;height:8px;border-radius:50%;background:#e94560;transition:background .3s}
.dot.on{background:#4ecca3}
.tb{background:#16213e;padding:6px 16px;display:flex;gap:8px;align-items:center;border-bottom:1px solid #0f3460;flex-shrink:0;flex-wrap:wrap}
.btn{background:#0f3460;color:#eee;border:1px solid #4ecca3;padding:4px 12px;border-radius:4px;cursor:pointer;font-size:12px}
.btn:hover{background:#4ecca3;color:#1a1a2e}
.flt{display:flex;gap:6px;margin-left:auto}
.fb{padding:3px 10px;border-radius:12px;border:1px solid;cursor:pointer;font-size:11px;opacity:.35;background:transparent}
.fb.on{opacity:1}
.fd{border-color:#888;color:#888}.fi{border-color:#eee;color:#eee}
.fw{border-color:#f0c040;color:#f0c040}.fe{border-color:#e94560;color:#e94560}.fs{border-color:#4ecca3;color:#4ecca3}
#log{flex:1;overflow-y:auto;padding:8px 16px}
.ll{padding:2px 0;border-bottom:1px solid #1e1e3a;display:flex;gap:8px;line-height:1.6}
.ll:hover{background:#1e1e3a}
.lt{color:#555;min-width:80px}.lv{min-width:72px;font-weight:bold}.lg{color:#4ecca3;min-width:85px}.lm{flex:1;word-break:break-word}
.DEBUG{color:#888}.INFO{color:#eee}.WARNING{color:#f0c040}.ERROR{color:#e94560}.SUCCESS{color:#4ecca3}
.ft{background:#16213e;padding:4px 16px;font-size:11px;color:#555;border-top:1px solid #0f3460;display:flex;justify-content:space-between;flex-shrink:0}
#log::-webkit-scrollbar{width:6px}
#log::-webkit-scrollbar-track{background:#1a1a2e}
#log::-webkit-scrollbar-thumb{background:#0f3460;border-radius:3px}
</style></head><body>
<div class="hdr"><h1>Semenzaio - Debug Logger</h1>
<div class="st"><div class="dot" id="dot"></div><span id="stxt">Connessione...</span></div></div>
<div class="tb">
<button class="btn" onclick="clr()">Pulisci</button>
<button class="btn" onclick="togAS()">Auto-scroll: <span id="sc">ON</span></button>
<button class="btn" onclick="dl()">Scarica log</button>
<div class="flt">
<button class="fb fd on" onclick="togF('DEBUG')">DEBUG</button>
<button class="fb fi on" onclick="togF('INFO')">INFO</button>
<button class="fb fw on" onclick="togF('WARNING')">WARN</button>
<button class="fb fe on" onclick="togF('ERROR')">ERROR</button>
<button class="fb fs on" onclick="togF('SUCCESS')">OK</button>
</div></div>
<div id="log"></div>
<div class="ft"><span id="cnt">0 righe</span><span id="up"></span></div>
<script>
let ws,as=true,n=0,all=[],st=Date.now(),af=new Set(['DEBUG','INFO','WARNING','ERROR','SUCCESS']);
function conn(){
  ws=new WebSocket('ws://'+location.hostname+'/ws');
  ws.onopen=()=>{dot(1);add('INFO','LOGGER','Connesso al dispositivo',now())};
  ws.onclose=()=>{dot(0);add('WARNING','LOGGER','Disconnesso - riconnessione in 3s...',now());setTimeout(conn,3000)};
  ws.onmessage=e=>{try{const d=JSON.parse(e.data);if(d.type==='log')add(d.level,d.tag,d.message,d.time)}catch(x){}};
}
function dot(c){document.getElementById('dot').className='dot'+(c?' on':'');document.getElementById('stxt').innerText=c?'Connesso':'Disconnesso'}
function now(){return new Date().toLocaleTimeString()}
function esc(s){return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;')}
function add(lv,tag,msg,t){
  all.push({lv,tag,msg,t});n++;
  document.getElementById('cnt').innerText=n+' righe';
  if(af.has(lv))ren({lv,tag,msg,t});
}
function ren(l){
  const log=document.getElementById('log');
  const d=document.createElement('div');
  d.className='ll '+l.lv;
  d.innerHTML='<span class="lt">'+l.t+'</span>'
    +'<span class="lv '+l.lv+'">['+l.lv+']</span>'
    +'<span class="lg">'+l.tag+'</span>'
    +'<span class="lm">'+esc(l.msg)+'</span>';
  log.appendChild(d);
  if(as)log.scrollTop=log.scrollHeight;
}
function togF(lv){
  const m={DEBUG:'fd',INFO:'fi',WARNING:'fw',ERROR:'fe',SUCCESS:'fs'};
  const b=document.querySelector('.'+m[lv]);
  if(af.has(lv)){af.delete(lv);b.classList.remove('on')}
  else{af.add(lv);b.classList.add('on')}
  document.getElementById('log').innerHTML='';
  all.forEach(l=>{if(af.has(l.lv))ren(l)});
}
function clr(){document.getElementById('log').innerHTML='';all=[];n=0;document.getElementById('cnt').innerText='0 righe'}
function togAS(){as=!as;document.getElementById('sc').innerText=as?'ON':'OFF'}
function dl(){
  const t=all.map(l=>'['+l.t+'] ['+l.lv+'] ['+l.tag+'] '+l.msg).join('\n');
  const a=document.createElement('a');
  a.href=URL.createObjectURL(new Blob([t],{type:'text/plain'}));
  a.download='semenzaio_'+new Date().toISOString().slice(0,10)+'.txt';
  a.click();
}
setInterval(()=>{
  const s=Math.floor((Date.now()-st)/1000),m=Math.floor(s/60),h=Math.floor(m/60);
  document.getElementById('up').innerText='Sessione: '
    +String(h).padStart(2,'0')+':'+String(m%60).padStart(2,'0')+':'+String(s%60).padStart(2,'0');
},1000);
conn();
</script></body></html>
)HTML";

// ─── Istanza globale ───────────────────────────
DebugLogger Debug;

// ─── Costruttore ──────────────────────────────
DebugLogger::DebugLogger(uint16_t port)
  : _server(port), _ws("/ws"), _port(port) {}

// ─── Begin ────────────────────────────────────
void DebugLogger::begin() {
  _ws.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
    this->_onWebSocketEvent(server, client, type, arg, data, len);
  });
  _server.addHandler(&_ws);

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "text/html", LOGGER_HTML);
  });
  _server.begin();

}

// ─── Handle ───────────────────────────────────
void DebugLogger::handle() {
  _ws.cleanupClients();  // libera client disconnessi

  // Ping periodico per mantenere la connessione WebSocket attiva.
  // I browser chiudono le connessioni WS inattive dopo ~45s senza ping.
  static uint32_t lastPing = 0;
  if (millis() - lastPing >= 30000) {
    lastPing = millis();
    _ws.pingAll();
  }
}

// ─── WebSocket events ─────────────────────────
void DebugLogger::_onWebSocketEvent(AsyncWebSocket* server,
                                     AsyncWebSocketClient* client,
                                     AwsEventType type, void* arg,
                                     uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    for (const String& msg : _history)
      client->text(msg);
  }
}

// ─── Log principale ───────────────────────────
void DebugLogger::log(LogLevel level, const String& tag, const String& message) {
  char timeBuf[9] = "--:--:--";
  struct tm t;
  if (getLocalTime(&t))
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

  String json = _buildJson(level, tag, message, timeBuf);

  if (_history.size() >= _historySize)
    _history.erase(_history.begin());
  _history.push_back(json);

  _ws.textAll(json);

}

// ─── Shortcut ─────────────────────────────────
void DebugLogger::debug(const String& tag, const String& msg)   { log(LogLevel::DEBUG,   tag, msg); }
void DebugLogger::info(const String& tag, const String& msg)    { log(LogLevel::INFO,    tag, msg); }
void DebugLogger::warning(const String& tag, const String& msg) { log(LogLevel::WARNING, tag, msg); }
void DebugLogger::error(const String& tag, const String& msg)   { log(LogLevel::ERROR,   tag, msg); }
void DebugLogger::success(const String& tag, const String& msg) { log(LogLevel::SUCCESS, tag, msg); }

// ─── Printf style ─────────────────────────────
void DebugLogger::logf(LogLevel level, const String& tag, const char* format, ...) {
  char buf[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  log(level, tag, String(buf));
}

// ─── JSON builder ─────────────────────────────
String DebugLogger::_buildJson(LogLevel level, const String& tag,
                                const String& msg, const char* time) {
  String escaped = msg;
  escaped.replace("\\", "\\\\");
  escaped.replace("\"", "\\\"");

  String json = "{\"type\":\"log\",\"level\":\"";
  json += _levelName(level);
  json += "\",\"tag\":\""; json += tag;
  json += "\",\"message\":\""; json += escaped;
  json += "\",\"time\":\""; json += time; json += "\"}";
  return json;
}

// ─── Helpers ──────────────────────────────────
const char* DebugLogger::_levelName(LogLevel level) {
  switch (level) {
    case LogLevel::DEBUG:   return "DEBUG";
    case LogLevel::INFO:    return "INFO";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::ERROR:   return "ERROR";
    case LogLevel::SUCCESS: return "SUCCESS";
    default:                return "INFO";
  }
}

// ─── Getters ──────────────────────────────────
bool    DebugLogger::isClientConnected() const { return _ws.count() > 0; }
uint8_t DebugLogger::getClientCount()    const { return _ws.count(); }
String  DebugLogger::getURL()            const {
  return "http://" + WiFi.localIP().toString() + ":" + String(_port);
}
void DebugLogger::setMirrorToSerial(bool e) { _mirrorSerial = e; }
void DebugLogger::setHistorySize(uint8_t l) { _historySize  = l; }