// html_page.h — Web Interface HTML stored in PROGMEM (flash)
// Розмір: ~50KB у flash, 0 bytes у RAM під час зберігання
// Обслуговується чанками по 2KB (handleRoot у web_server.h)
#pragma once
#include <pgmspace.h>

static const char HTML_PAGE[] PROGMEM = R"===(
<!DOCTYPE html>
<html lang="uk">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>JBD BMS Controller</title>
<style>
:root{
  --bg:#0a0e1a;--sur:#111827;--sur2:#1a2235;--brd:#1e2d45;
  --ac:#00d4ff;--ac2:#00ff88;--warn:#ffaa00;--err:#ff4455;
  --txt:#e2e8f0;--mut:#64748b;
}
*{margin:0;padding:0;box-sizing:border-box}
body{background:var(--bg);color:var(--txt);font-family:'Courier New',monospace;padding-bottom:24px}

/* ── NAV ── */
nav{display:flex;background:var(--sur);border-bottom:1px solid var(--brd);position:sticky;top:0;z-index:99}
.tab{flex:1;padding:13px 4px;text-align:center;font-size:10px;letter-spacing:.8px;text-transform:uppercase;color:var(--mut);cursor:pointer;border-bottom:2px solid transparent;transition:all .2s;user-select:none}
.tab.on{color:var(--ac);border-bottom-color:var(--ac)}
.tab:hover:not(.on){color:var(--txt)}

/* ── PAGES ── */
.pg{display:none;padding:14px}
.pg.on{display:block}

/* ── HEADER ── */
.phdr{display:flex;align-items:center;justify-content:space-between;background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:11px 16px;margin-bottom:14px}
.logo{font-size:16px;font-weight:bold;color:var(--ac);letter-spacing:2px}
.logo b{color:var(--ac2)}
.srow{display:flex;align-items:center;gap:8px;font-size:11px;color:var(--mut);flex-wrap:wrap}
.dot{width:8px;height:8px;border-radius:50%;background:var(--err);animation:pu 2s infinite;flex-shrink:0}
.dot.ok{background:var(--ac2);animation:none}
@keyframes pu{0%,100%{opacity:1}50%{opacity:.3}}
.chip{font-size:10px;padding:2px 8px;border-radius:4px;background:var(--sur2);border:1px solid var(--brd);color:var(--mut);white-space:nowrap}
.chip.ok{background:rgba(0,255,136,.1);color:var(--ac2);border-color:var(--ac2)}
.chip.er{background:rgba(255,68,85,.1);color:var(--err);border-color:var(--err)}
.chip.wn{background:rgba(255,170,0,.1);color:var(--warn);border-color:var(--warn)}

/* ── METRIC CARDS ── */
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(145px,1fr));gap:10px;margin-bottom:14px}
.card{background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:12px 14px}
.clbl{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:5px}
.cval{font-size:24px;font-weight:bold;color:var(--ac);line-height:1}
.cu{font-size:12px;color:var(--mut);margin-left:3px}
.csub{font-size:10px;color:var(--mut);margin-top:4px}
.card.wn .cval{color:var(--warn)}
.card.er .cval{color:var(--err)}
.card.ok .cval{color:var(--ac2)}

/* ── SOC ── */
.soc-box{background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:14px;margin-bottom:14px}
.soc-hdr{display:flex;justify-content:space-between;align-items:baseline;margin-bottom:9px}
.soc-lbl{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:1px}
.soc-num{font-size:30px;font-weight:bold;color:var(--ac2)}
.bar{height:12px;background:var(--sur2);border-radius:6px;overflow:hidden;border:1px solid var(--brd)}
.bar-f{height:100%;border-radius:6px;background:var(--ac2);transition:width .8s,background .4s}
.bar-f.wn{background:var(--warn)}
.bar-f.er{background:var(--err)}
.soc-ft{display:flex;justify-content:space-between;margin-top:5px;font-size:10px;color:var(--mut)}

/* ── FET STATUS ── */
.fet-row{display:flex;gap:8px;margin-bottom:14px;flex-wrap:wrap}
.fet-card{flex:1;min-width:130px;background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:11px 13px;display:flex;align-items:center;gap:10px}
.fled{width:10px;height:10px;border-radius:50%;background:var(--brd);flex-shrink:0;transition:all .3s}
.fled.on{background:var(--ac2);box-shadow:0 0 8px var(--ac2)}
.fled.off{background:var(--err)}

/* ── SECTION BOX ── */
.box{background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:14px;margin-bottom:14px}
.stitle{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:11px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:6px}

/* ── CELL GRID ── */
.cgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(74px,1fr));gap:6px}
.cell{background:var(--sur2);border:1px solid var(--brd);border-radius:6px;padding:7px;text-align:center;transition:border-color .3s,background .3s}
.cell.hi{border-color:var(--ac2)}
.cell.lo{border-color:var(--warn)}
.cell.bal{border-color:var(--warn);background:rgba(255,170,0,.1);animation:balp 1s infinite}
@keyframes balp{0%,100%{opacity:1}50%{opacity:.6}}
.cnum{font-size:9px;color:var(--mut);margin-bottom:2px}
.cv{font-size:12px;font-weight:bold;color:var(--txt)}
.cbar{height:3px;background:var(--brd);border-radius:2px;margin-top:5px;overflow:hidden}
.cbf{height:100%;background:var(--ac2);border-radius:2px;transition:width .5s}
.cell.lo .cbf{background:var(--warn)}
.cell.bal .cbf{background:var(--warn)}
.bal-icon{font-size:9px;color:var(--warn);float:right}

/* ── TEMPS ── */
.trow{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:14px}
.tcard{flex:1;min-width:100px;background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:12px}

/* ── PROTECTION FLAGS ── */
.fgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(165px,1fr));gap:5px;margin-top:10px}
.flag{display:flex;align-items:center;gap:6px;font-size:10px;color:var(--mut);padding:5px 8px;background:var(--sur2);border-radius:5px;border:1px solid var(--brd)}
.flag.on{color:var(--err);border-color:var(--err);background:rgba(255,68,85,.1)}
.fdot{width:6px;height:6px;border-radius:50%;background:var(--brd);flex-shrink:0}
.flag.on .fdot{background:var(--err)}

/* ── BUTTONS ── */
.btns{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px}
button{background:transparent;border:1px solid var(--brd);color:var(--txt);font-family:'Courier New',monospace;font-size:11px;padding:8px 13px;border-radius:6px;cursor:pointer;letter-spacing:.6px;transition:all .2s}
button:hover{border-color:var(--ac);color:var(--ac)}
button.bac{border-color:var(--ac);color:var(--ac)}
button.bac2{border-color:var(--ac2);color:var(--ac2)}
button.bwn{border-color:var(--warn);color:var(--warn)}
button.ber{border-color:var(--err);color:var(--err)}
button:active{transform:scale(.97)}
button:disabled{opacity:.4;cursor:not-allowed;transform:none}

/* ── SETTINGS ── */
.sg{background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:14px;margin-bottom:12px}
.sg h3{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:12px;padding-bottom:8px;border-bottom:1px solid var(--brd)}
.field{margin-bottom:10px}
.field label{display:block;font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:5px}
.field input,.field select{width:100%;background:var(--sur2);border:1px solid var(--brd);color:var(--txt);font-family:'Courier New',monospace;font-size:12px;padding:8px 10px;border-radius:5px;outline:none;transition:border .2s}
.field input:focus,.field select:focus{border-color:var(--ac)}
.field input[type=range]{padding:4px 0;background:none;border:none}
.field select option{background:var(--sur)}
.frow{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.hint{font-size:10px;color:var(--mut);margin-top:4px;line-height:1.5}
.vd{font-size:12px;color:var(--ac);font-weight:bold;margin-left:6px}
.hr{border:none;border-top:1px solid var(--brd);margin:12px 0}

/* ── LISTS ── */
.wlist,.blist{border:1px solid var(--brd);border-radius:6px;margin-bottom:10px;overflow-y:auto}
.wlist{max-height:215px}
.blist{max-height:210px}
.witem,.bitem{display:flex;align-items:center;justify-content:space-between;padding:9px 12px;border-bottom:1px solid var(--brd);cursor:pointer;transition:background .15s}
.witem:last-child,.bitem:last-child{border-bottom:none}
.witem:hover{background:rgba(0,212,255,.05)}
.bitem:hover{background:rgba(0,255,136,.05)}
.witem.sel{background:rgba(0,212,255,.08);border-left:2px solid var(--ac)}
.bitem.sel{background:rgba(0,255,136,.08);border-left:2px solid var(--ac2)}
.wssid,.bname{font-size:12px;font-weight:bold}
.wmeta,.baddr{font-size:10px;color:var(--mut);margin-top:2px;font-family:monospace}
.brssi{font-size:10px;color:var(--mut);flex-shrink:0}

/* ── MISC ── */
.spin{display:inline-block;width:13px;height:13px;border:2px solid var(--brd);border-top-color:var(--ac);border-radius:50%;animation:sp .7s linear infinite;vertical-align:middle;margin-right:5px}
@keyframes sp{to{transform:rotate(360deg)}}
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);background:var(--sur);border:1px solid var(--brd);border-radius:8px;padding:9px 18px;font-size:11px;opacity:0;transition:opacity .25s;pointer-events:none;z-index:999;white-space:nowrap;max-width:90vw}
.toast.show{opacity:1}
.toast.ok{border-color:var(--ac2);color:var(--ac2)}
.toast.er{border-color:var(--err);color:var(--err)}
.toast.wn{border-color:var(--warn);color:var(--warn)}
.strow{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin-bottom:10px}
.rinfo{font-size:10px;color:var(--mut);display:flex;flex-wrap:wrap;gap:8px;margin-bottom:12px}
footer{text-align:center;font-size:10px;color:var(--mut);padding-top:12px;border-top:1px solid var(--brd);letter-spacing:1px}
@media(max-width:420px){.grid{grid-template-columns:1fr 1fr}.frow{grid-template-columns:1fr}.fet-row{flex-direction:column}}
</style>
</head>
<body>

<!-- ═══════════════════════════════════ NAV ═══ -->
<nav>
  <div class="tab on" onclick="gNav('dash',this)">📊 Монітор</div>
  <div class="tab"    onclick="gNav('net',this)">📡 Мережа</div>
  <div class="tab"    onclick="gNav('cloud',this)">☁ Replit</div>
</nav>

<!-- ═══════════════════════════════════════════ -->
<!--  PAGE 1 — DASHBOARD                         -->
<!-- ═══════════════════════════════════════════ -->
<div class="pg on" id="pg-dash">

  <!-- Header / connection status -->
  <div class="phdr">
    <div class="logo">JBD <b>BMS</b></div>
    <div class="srow">
      <!-- WiFi STA -->
      <div class="dot" id="wifiDot"></div>
      <span class="chip" id="wifiChip">WiFi --</span>
      <!-- BMS BLE -->
      <div class="dot" id="bmsDot"></div>
      <span class="chip" id="bmsChip">BMS --</span>
    </div>
  </div>

  <!-- SOC -->
  <div class="soc-box">
    <div class="soc-hdr">
      <span class="soc-lbl">Заряд (SOC)</span>
      <span class="soc-num" id="socN">--%</span>
    </div>
    <div class="bar"><div class="bar-f" id="socF" style="width:0%"></div></div>
    <div class="soc-ft">
      <span>0%</span>
      <span id="socAh">-- / -- Аг</span>
      <span>100%</span>
    </div>
  </div>

  <!-- Main metrics -->
  <div class="grid">
    <div class="card" id="vCard">
      <div class="clbl">Напруга пакету</div>
      <div class="cval" id="vPack">--<span class="cu">В</span></div>
      <div class="csub" id="vDiff">Δ клітин: --</div>
    </div>
    <div class="card" id="iCard">
      <div class="clbl">Струм</div>
      <div class="cval" id="vCurr">--<span class="cu">А</span></div>
      <div class="csub" id="iMode">--</div>
    </div>
    <div class="card">
      <div class="clbl">Потужність</div>
      <div class="cval" id="vPwr">--<span class="cu">Вт</span></div>
      <div class="csub" id="pwrDir">--</div>
    </div>
    <div class="card">
      <div class="clbl">Цикли</div>
      <div class="cval" id="vCyc">--<span class="cu">шт</span></div>
      <div class="csub" id="bmsName" style="color:var(--ac);overflow:hidden;white-space:nowrap;text-overflow:ellipsis">--</div>
    </div>
  </div>

  <!-- FET status -->
  <div class="fet-row">
    <div class="fet-card">
      <div class="fled" id="fChgLed"></div>
      <div>
        <div class="clbl" style="margin-bottom:2px">Заряд MOS (bit0)</div>
        <div style="font-size:11px;font-weight:bold" id="fChgTxt">--</div>
      </div>
    </div>
    <div class="fet-card">
      <div class="fled" id="fDsgLed"></div>
      <div>
        <div class="clbl" style="margin-bottom:2px">Розряд MOS (bit1)</div>
        <div style="font-size:11px;font-weight:bold" id="fDsgTxt">--</div>
      </div>
    </div>
    <div class="fet-card" id="dispCard">
      <div class="fled" id="fDispLed"></div>
      <div>
        <div class="clbl" style="margin-bottom:2px">Дисплей</div>
        <div style="font-size:11px;font-weight:bold" id="fDispTxt">--</div>
      </div>
    </div>
  </div>

  <!-- Temperatures -->
  <div class="trow" id="tRow"></div>

  <!-- Cell voltages -->
  <div class="box">
    <div class="stitle">
      <span>Напруги клітин</span>
      <div style="display:flex;gap:6px;align-items:center">
        <span id="balInd" style="font-size:10px;color:var(--warn);display:none">⚡ Балансування</span>
        <span class="chip" id="cellChip">-- клітин</span>
      </div>
    </div>
    <div class="cgrid" id="cellGrid">
      <div style="color:var(--mut);font-size:10px;grid-column:1/-1">BMS не підключено</div>
    </div>
  </div>

  <!-- Protection flags -->
  <div class="box">
    <div class="stitle">
      <span>Захисні прапори JBD</span>
      <span style="font-size:9px;color:var(--mut)">protectionStatus · bit0–bit12</span>
    </div>
    <div style="font-size:10px;color:var(--mut);margin-bottom:8px">
      Raw: <span id="protRaw" style="color:var(--ac);font-family:monospace">0x0000</span>
    </div>
    <div class="fgrid" id="flagGrid"></div>
  </div>

  <!-- ESP32 System Info -->
  <div class="card">
    <h3>⚙️ Системна інформація ESP32-S3</h3>
    <div style="display:grid;grid-template-columns:repeat(2,1fr);gap:8px;font-size:10px">
      <div>
        <span style="color:var(--mut)">Час роботи (Uptime):</span>
        <strong id="espUptime" style="color:var(--ac);font-family:monospace">--</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Частота CPU:</span>
        <strong id="espCpuFreq" style="color:var(--ac);font-family:monospace">-- MHz</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Температура ядра:</span>
        <strong id="espTemp" style="color:var(--ac);font-family:monospace">-- °C</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Внутрішня RAM (Heap):</span>
        <strong id="espHeap" style="color:var(--ac);font-family:monospace">-- / -- байт</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Зовнішня PSRAM:</span>
        <strong id="espPsram" style="color:var(--ac);font-family:monospace">-- / -- КБ</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Flash-пам'ять:</span>
        <strong id="flashSize" style="color:var(--ac);font-family:monospace">-- байт</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Wi-Fi RSSI (Сигнал):</span>
        <strong id="wifiRssi" style="color:var(--ac);font-family:monospace">-- дБм</strong>
      </div>
      <div>
        <span style="color:var(--mut)">Підключено клієнтів (AP):</span>
        <strong id="apClients" style="color:var(--ac);font-family:monospace">--</strong>
      </div>
    </div>
  </div>

  <!-- MOSFET control -->
  <div class="box">
    <div class="stitle"><span>Керування MOSFET</span></div>
    <div class="btns">
      <button class="ber"  onclick="sendMOS(false, false)">■ Всі ВИКЛ</button>
      <button onclick="sendMOS(true, false)">⚡ Тільки заряд</button>
      <button onclick="sendMOS(false, true)">▶ Тільки розряд</button>
      <button class="bac2" onclick="sendMOS(true, true)">✓ Обидва ВКЛ</button>
    </div>
    <div class="btns" style="margin-bottom:0">
      <button class="bwn" onclick="sendBalance(true)">⚖️ Ввімкнути баланс</button>
      <button class="bwn" onclick="sendBalance(false)">⚖️ Вимкнути баланс</button>
    </div>
    <div class="btns" style="margin-bottom:0">
      <button class="bwn" onclick="dispToggle()">🖥 Перемкнути дисплей</button>
      <button class="bwn" id="autoPollBtn" onclick="toggleAutoPoll()">🔄 Авто опитування: ВКЛ</button>
    </div>
  </div>

  <div class="rinfo">
    <span id="lastUpd">Не оновлено</span>
    <span>|</span><span>Polling: <code>/data</code> кожні 2 с</span>
    <span>|</span><span>WiFi IP: <span id="wifiIp" style="color:var(--ac)">--</span></span>
  </div>

  <footer>ESP32-S3 Super Mini · JBD BMS · LittleFS</footer>
</div>

<!-- ═══════════════════════════════════════════ -->
<!--  PAGE 2 — NETWORK                           -->
<!-- ═══════════════════════════════════════════ -->
<div class="pg" id="pg-net">

  <!-- WiFi STA -->
  <div class="sg">
    <h3>📶 Wi-Fi STA — підключення ESP32 до мережі</h3>
    <div class="hint" style="margin-bottom:10px">
      AP завжди активна: <code>JBD-BMS-ESP32</code> / пароль з прошивки.
    </div>

    <div class="strow">
      <button class="bac" id="wScanBtn" onclick="wifiScan()">⟳ Сканувати</button>
      <span class="chip" id="wChip">
        <span class="dot" id="wDot" style="width:6px;height:6px"></span>&nbsp;
        <span id="wTxt">завантаження...</span>
      </span>
      <span id="wCurIp" style="font-size:11px;color:var(--ac)"></span>
    </div>

    <div class="wlist" id="wList">
      <div style="padding:16px;text-align:center;color:var(--mut);font-size:11px">Натисніть «Сканувати»</div>
    </div>

    <div class="field">
      <label>SSID</label>
      <input id="wSsid" type="text" placeholder="Оберіть зі списку або введіть вручну">
    </div>
    <div class="field">
      <label>Пароль</label>
      <input id="wPass" type="password" placeholder="Залиште порожнім для відкритих мереж" autocomplete="new-password">
    </div>
    <hr class="hr">
    <div class="btns">
      <button class="bac2" onclick="wifiSave()">💾 Зберегти і підключити</button>
    </div>
  </div>

  <!-- AP info -->
  <div class="sg">
    <h3>📡 Точка доступу ESP32 (AP — завжди активна)</h3>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;font-size:11px;color:var(--mut)">
      <div><span style="color:var(--ac)">SSID:</span> JBD_BMS_Controller</div>
      <div><span style="color:var(--ac)">IP:</span> 192.168.4.1</div>
    </div>
  </div>

  <!-- OTA Update -->
  <div class="sg">
    <h3>🌐 Бездротове оновлення прошивки (OTA)</h3>
    <p style="font-size:11px;color:var(--mut);margin-bottom:10px">Завантажте відкомпільований файл <code>firmware.bin</code>:</p>
    <form method="POST" action="/update" enctype="multipart/form-data" id="upload_form">
      <div class="field">
        <input type="file" name="update" accept=".bin" style="margin-bottom:10px;width:100%">
      </div>
      <button type="submit" class="bac2" style="width:100%">Оновити по Wi-Fi</button>
      <div id="prg" style="display:none;margin-top:10px;">Прогрес: <span id="prg_val">0</span>%</div>
    </form>
  </div>

</div>

<!-- ═══════════════════════════════════════════ -->
<!--  PAGE 3 — REPLIT CLOUD                      -->
<!-- ═══════════════════════════════════════════ -->
<div class="pg" id="pg-cloud">
  <div class="sg">
    <h3>☁ Статус з'єднання з Replit</h3>
    <div class="strow">
      <span class="chip" id="rChip">
        <span class="dot" id="rDot" style="width:6px;height:6px"></span>&nbsp;
        <span id="rTxt">не підключено</span>
      </span>
    </div>
  </div>

  <!-- Cloud Relay -->
  <div class="sg">
    <h3>☁️ Cloud Relay — відправка даних в мобільний додаток</h3>
    <div class="hint" style="margin-bottom:10px">
      ESP32 надсилає POST на сервер для доступу з будь-якої мережі.
    </div>

    <div class="field">
      <label>Device ID</label>
      <input id="cDeviceId" type="text" placeholder="напр. esp01, garage, main-pack">
    </div>

    <div class="field">
      <label>Статус</label>
      <div class="strow">
        <label class="sw">
          <input type="checkbox" id="cEnabled">
          <span class="sl"></span>
          <span class="sr">Увімкнути</span>
        </label>
      </div>
    </div>

    <div class="field">
      <label>Server URL</label>
      <input id="cServer" type="text" placeholder="https://your-replit-url.replit.dev/api/bms/push">
    </div>

    <hr class="hr">
    <div class="btns">
      <button class="bac2" onclick="cloudSave()">💾 Зберегти налаштування</button>
    </div>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
'use strict';

const EP_DATA = '/data';
const EP_CONTROL = '/control';
const EP_SCAN = '/scan';
const EP_WIFISCAN = '/wifiscan';
const EP_WIFICONNECT = '/wificonnect';
const EP_CONNECT = '/connect';
const EP_AUTOPOLL = '/autopoll';
const EP_DISCONNECT = '/disconnect';
const EP_DISPLAY = '/display';
const EP_UPDATE = '/update';

function gNav(id, el) {
  document.querySelectorAll('.pg').forEach(p => p.classList.remove('on'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('on'));
  document.getElementById('pg-' + id).classList.add('on');
  el.classList.add('on');
}

let _tt;
function toast(msg, type = 'ok') {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.className = 'toast show ' + type;
  clearTimeout(_tt);
  _tt = setTimeout(() => el.className = 'toast', 2800);
}

const FLAGS = [
  { bit:0,  k:'cell_ovp', l:'bit0 — Клітина: перенапруга'   },
  { bit:1,  k:'cell_uvp', l:'bit1 — Клітина: недонапруга'   },
  { bit:2,  k:'pack_ovp', l:'bit2 — Пакет: перенапруга'     },
  { bit:3,  k:'pack_uvp', l:'bit3 — Пакет: недонапруга'     },
  { bit:4,  k:'chg_otp',  l:'bit4 — Перегрів (заряд)'       },
  { bit:5,  k:'chg_utp',  l:'bit5 — Переохол. (заряд)'      },
  { bit:6,  k:'dsg_otp',  l:'bit6 — Перегрів (розряд)'      },
  { bit:7,  k:'dsg_utp',  l:'bit7 — Переохол. (розряд)'     },
  { bit:8,  k:'chg_ocp',  l:'bit8 — Перевантаж. (заряд)'    },
  { bit:9,  k:'dsg_ocp',  l:'bit9 — Перевантаж. (розряд)'   },
  { bit:10, k:'scp',      l:'bit10 — Коротке замикання'      },
  { bit:11, k:'afe_err',  l:'bit11 — Помилка AFE/IC'         },
  { bit:12, k:'sw_lock',  l:'bit12 — Software lock MOS'      },
];
document.getElementById('flagGrid').innerHTML =
  FLAGS.map(f => `<div class="flag" id="fl_${f.k}"><div class="fdot"></div>${f.l}</div>`).join('');

function renderWifiStatus(d) {
  const ok = d.wifiConnected;
  document.getElementById('wifiDot').className = 'dot' + (ok?' ok':'');
  document.getElementById('wifiChip').className = 'chip' + (ok?' ok':' er');
  document.getElementById('wifiChip').textContent = ok ? ('WiFi '+d.wifiInfo) : 'WiFi відключено';
  document.getElementById('wDot').style.background = ok ? 'var(--ac2)' : 'var(--err)';
  document.getElementById('wTxt').textContent = ok ? 'підключено' : 'не підключено';
  document.getElementById('wChip').className = 'chip' + (ok?' ok':' er');
  document.getElementById('wCurIp').textContent = ok ? d.wifiInfo : '';
  document.getElementById('wifiIp').textContent = ok ? d.wifiInfo : '--';
}

function renderBmsStatus(d) {
  const ok = d.bmsConnected;
  const bmsDotEl = document.getElementById('bmsDot');
  if (bmsDotEl) bmsDotEl.className = 'dot' + (ok?' ok':'');
  const bmsChipEl = document.getElementById('bmsChip');
  if (bmsChipEl) {
    bmsChipEl.className = 'chip' + (ok?' ok':' er');
    bmsChipEl.textContent = ok ? ('BMS '+d.bmsInfo) : 'BMS відключено';
  }
  const bDotEl = document.getElementById('bDot');
  if (bDotEl) bDotEl.style.background = ok ? 'var(--ac2)' : 'var(--err)';
  const bTxtEl = document.getElementById('bTxt');
  if (bTxtEl) bTxtEl.textContent = ok ? ('підключено · '+(d.bmsInfo||'')) : 'не підключено';
  const bChipEl = document.getElementById('bChip');
  if (bChipEl) bChipEl.className = 'chip' + (ok?' ok':' er');
  const bmsNameEl = document.getElementById('bmsName');
  if (bmsNameEl) bmsNameEl.textContent = d.bmsInfo || '--';
}

function renderFET(fetStatus, displayActive) {
  const chg = (fetStatus >> 0) & 1, dsg = (fetStatus >> 1) & 1;
  const setFet = (ledId, txtId, on) => {
    document.getElementById(ledId).className = 'fled ' + (on ? 'on' : 'off');
    const el = document.getElementById(txtId);
    el.textContent = on ? 'ВКЛ' : 'ВИКЛ';
    el.style.color = on ? 'var(--ac2)' : 'var(--err)';
  };
  setFet('fChgLed','fChgTxt', chg);
  setFet('fDsgLed','fDsgTxt', dsg);
  document.getElementById('fDispLed').className = 'fled ' + (displayActive ? 'on' : 'off');
  document.getElementById('fDispTxt').textContent = displayActive ? 'ВКЛ' : 'ВИКЛ';
  document.getElementById('fDispTxt').style.color = displayActive ? 'var(--ac2)' : 'var(--mut)';
}

function renderCells(cvs, balLow, balHigh, cellCount) {
  const g = document.getElementById('cellGrid');
  const n = cellCount || cvs.length;
  const mn = Math.min(...cvs), mx = Math.max(...cvs), rng = mx-mn||.001;

  document.getElementById('cellChip').textContent = n + ' клітин · Δ ' + ((mx-mn)*1000).toFixed(0) + ' мВ';
  document.getElementById('vDiff').textContent = 'Δ клітин: ' + ((mx-mn)*1000).toFixed(0) + ' мВ';

  const balAny = (balLow > 0 || balHigh > 0);
  document.getElementById('balInd').style.display = balAny ? '' : 'none';

  if (g.children.length !== cvs.length) {
    g.innerHTML = cvs.map((_,i) => `
      <div class="cell" id="cl${i}">
        <div class="cnum">C${i+1} <span class="bal-icon" id="bi${i}" style="display:none">⚡</span></div>
        <div class="cv" id="cv${i}">--</div>
        <div class="cbar"><div class="cbf" id="cb${i}" style="width:0%"></div></div>
      </div>`).join('');
  }

  cvs.forEach((v, i) => {
    const isBal = i < 16 ? ((balLow >> i) & 1) : ((balHigh >> (i-16)) & 1);
    const el = document.getElementById('cl' + i);
    el.className = 'cell' + (isBal ? ' bal' : v===mx ? ' hi' : v===mn ? ' lo' : '');
    document.getElementById('cv'+i).textContent = v.toFixed(3);
    document.getElementById('cb'+i).style.width = (((v-mn)/rng)*100).toFixed(1) + '%';
    const biEl = document.getElementById('bi'+i);
    if (biEl) biEl.style.display = isBal ? '' : 'none';
  });
}

function renderTemps(arr) {
  const row = document.getElementById('tRow');
  if (!arr || !arr.length) { row.innerHTML=''; return; }
  if (row.children.length !== arr.length) {
    row.innerHTML = arr.map((_,i) => `
      <div class="tcard" id="tc${i}">
        <div class="clbl">Датчик ${i+1}</div>
        <div class="cval ok" id="tv${i}">--<span class="cu">°C</span></div>
      </div>`).join('');
  }
  arr.forEach((t, i) => {
    const el = document.getElementById('tv'+i);
    if (el) el.innerHTML = t.toFixed(1)+'<span class="cu">°C</span>';
    const c = document.getElementById('tc'+i);
    if (c) { const cv=c.querySelector('.cval'); if(cv) cv.className='cval'+(t>50?' er':t>40?' wn':' ok'); }
  });
}

function renderFlags(protectionStatus) {
  const v = protectionStatus || 0;
  document.getElementById('protRaw').textContent = '0x' + v.toString(16).toUpperCase().padStart(4,'0');
  FLAGS.forEach(f => {
    const el = document.getElementById('fl_' + f.k);
    if (el) el.className = 'flag' + (((v >> f.bit) & 1) ? ' on' : '');
  });
}

function renderEspSystem(d) {
  if (d.espUptime !== undefined) {
    document.getElementById('espUptime').innerText = d.espUptime;
  }
  if (d.espCpuFreq !== undefined) {
    document.getElementById('espCpuFreq').innerText = d.espCpuFreq + " MHz";
  }
  if (d.espTemp !== undefined) {
    document.getElementById('espTemp').innerText = (d.espTemp > 0 ? d.espTemp.toFixed(1) : '--') + " °C";
  }
  if (d.espHeap !== undefined && d.espTotalHeap !== undefined) {
    let usedHeap = d.espTotalHeap - d.espHeap;
    document.getElementById('espHeap').innerText = usedHeap + " / " + d.espTotalHeap + " байт";
  }
  if (d.espTotalPsram !== undefined && d.espFreePsram !== undefined) {
    let totalPsramKb = Math.round(d.espTotalPsram / 1024);
    let freePsramKb = Math.round(d.espFreePsram / 1024);
    let usedPsramKb = totalPsramKb - freePsramKb;
    if (totalPsramKb > 0) {
      document.getElementById('espPsram').innerText = usedPsramKb + " / " + totalPsramKb + " КБ";
    } else {
      document.getElementById('espPsram').innerText = "Не активована (0 КБ)";
    }
  }
  if (d.flashSize !== undefined) {
    document.getElementById('flashSize').innerText = d.flashSize + " байт";
  }
  if (d.wifiRssi !== undefined) {
    document.getElementById('wifiRssi').innerText = d.wifiRssi + " дБм";
  }
  if (d.apClients !== undefined) {
    document.getElementById('apClients').innerText = d.apClients;
  }
}

function render(d) {
  renderWifiStatus(d);
  try {
    renderBmsStatus(d);
  } catch(e) {
    console.error("Помилка рендеру статусу BMS:", e);
  }
  
  const soc = d.soc ?? 0;
  document.getElementById('socN').textContent = soc + '%';
  const bf = document.getElementById('socF');
  bf.style.width = soc + '%';
  bf.className = 'bar-f' + (soc<15?' er':soc<30?' wn':'');
  const cap = d.capacity ?? '--', tot = d.totalCapacity ?? '--';
  document.getElementById('socAh').textContent = (typeof cap==='number'?cap.toFixed(1):cap) + ' / ' + (typeof tot==='number'?tot.toFixed(1):tot) + ' Аг';

  const v = d.voltage ?? 0;
  document.getElementById('vPack').innerHTML = v.toFixed(2)+'<span class="cu">В</span>';

  const curr = d.current ?? 0;
  document.getElementById('vCurr').innerHTML = Math.abs(curr).toFixed(2)+'<span class="cu">А</span>';
  document.getElementById('iMode').textContent = curr>.1?'▲ Розряд':curr<-.1?'▼ Заряд':'— Відпочинок';

  const pwr = v*curr;
  document.getElementById('vPwr').innerHTML = Math.abs(pwr).toFixed(1)+'<span class="cu">Вт</span>';
  document.getElementById('pwrDir').textContent = pwr>5?'▲ Споживання':pwr<-5?'▼ Заряд':'—';

  document.getElementById('vCyc').innerHTML = (d.cycleCount??'--')+'<span class="cu">шт</span>';

  renderFET(d.fetStatus??0, d.displayActive??false);
  renderTemps(d.temperatures);

  if (d.cellVoltages?.length) {
    renderCells(d.cellVoltages, d.balanceStatus??0, d.balanceStatusHigh??0, d.cellCount??d.cellVoltages.length);
  }

  renderFlags(d.protectionStatus ?? 0);
  renderEspSystem(d);

  document.getElementById('lastUpd').textContent = 'Оновлено: ' + new Date().toLocaleTimeString('uk-UA');
}

let pollInterval = null;
let isPolling = false;

function pollData() {
  if (isPolling) return;
  isPolling = true;

  fetch(EP_DATA)
    .then(r => {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    })
    .then(d => {
      console.log('Data received:', d);
      render(d);
      isPolling = false;
    })
    .catch(err => {
      console.error('Polling error:', err);
      toast('Помилка отримання даних: ' + err.message, 'er');
      isPolling = false;
    });
}

function sendMOS(chg, dsg) {
  fetch(EP_CONTROL + '?chg=' + (chg ? '1' : '0') + '&dsg=' + (dsg ? '1' : '0'))
    .then(r => r.json())
    .then(d => {
      if (d.status === 'ok') toast('MOSFET → Заряд:' + (chg ? 'ВКЛ' : 'ВИКЛ') + ' Розряд:' + (dsg ? 'ВКЛ' : 'ВИКЛ') + ' OK', 'ok');
      else toast('Помилка: ' + (d.message||'?'), 'er');
    })
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function sendBalance(state) {
  fetch('/balance?state=' + (state ? '1' : '0'))
    .then(r => r.text())
    .then(d => toast(d, 'ok'))
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function dispToggle() {
  fetch(EP_DISPLAY + '?cmd=toggle')
    .then(r => r.json())
    .then(d => toast('Дисплей: ' + (d.displayActive ? 'ВКЛ' : 'ВИКЛ'), 'ok'))
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function toggleAutoPoll() {
  fetch(EP_AUTOPOLL)
    .then(r => r.json())
    .then(d => {
      const newState = !d.autoPolling;
      fetch(EP_AUTOPOLL, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ enabled: newState })
      })
        .then(r => r.json())
        .then(d => {
          document.getElementById('autoPollBtn').textContent = '🔄 Авто опитування: ' + (d.autoPolling ? 'ВКЛ' : 'ВИКЛ');
          toast('Авто опитування: ' + (d.autoPolling ? 'ВКЛ' : 'ВИКЛ'), 'ok');
        })
        .catch(() => toast('ESP32 не відповідає', 'er'));
    })
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function loadCloudSettings() {
  fetch('/cloud-settings')
    .then(r => r.json())
    .then(d => {
      document.getElementById('cDeviceId').value = d.device_id || '';
      document.getElementById('cEnabled').checked = d.enabled || false;
      document.getElementById('cServer').value = d.server || '';
    })
    .catch(() => console.error('Failed to load cloud settings'));
}

function cloudSave() {
  const deviceId = document.getElementById('cDeviceId').value;
  const enabled = document.getElementById('cEnabled').checked;
  const server = document.getElementById('cServer').value;

  fetch('/cloud-save?device_id=' + encodeURIComponent(deviceId) + '&enabled=' + enabled + '&server=' + encodeURIComponent(server))
    .then(r => r.json())
    .then(d => {
      if (d.status === 'ok') toast('Cloud налаштування збережено', 'ok');
      else toast('Помилка: ' + (d.message || '?'), 'er');
    })
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function wifiScan() {
  const btn = document.getElementById('wScanBtn');
  btn.disabled = true; btn.innerHTML = '<span class="spin"></span>Сканування...';
  document.getElementById('wList').innerHTML =
    '<div style="padding:16px;text-align:center;color:var(--mut);font-size:11px"><span class="spin"></span>Сканування...</div>';

  if (pollInterval) {
    clearInterval(pollInterval);
    pollInterval = null;
  }

  fetch(EP_WIFISCAN)
    .then(r => r.json())
    .then(d => {
      renderWifiList(d.networks || []);
      btn.disabled = false; btn.textContent = '⟳ Сканувати';
      pollInterval = setInterval(pollData, 2000);
    })
    .catch(() => {
      toast('ESP32 не відповідає', 'er');
      btn.disabled = false; btn.textContent = '⟳ Сканувати';
      pollInterval = setInterval(pollData, 2000);
    });
}

function escHtml(v) {
  return String(v ?? '').replace(/[&<>"']/g, ch => ({
    '&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'
  }[ch]));
}

function renderWifiList(nets) {
  if (!nets.length) {
    document.getElementById('wList').innerHTML =
      '<div style="padding:16px;text-align:center;color:var(--mut);font-size:11px">Мереж не знайдено</div>';
    return;
  }
  document.getElementById('wList').innerHTML = nets
    .sort((a,b)=>b.rssi-a.rssi)
    .map(n => `
    <div class="witem" data-ssid="${escHtml(n.ssid)}">
      <div>
        <div class="wssid">${escHtml(n.ssid)}</div>
        <div class="wmeta">${escHtml(n.rssi)} дБм &nbsp;${n.open?'🔓 відкрита':'🔒'}</div>
      </div>
    </div>`).join('');
  document.querySelectorAll('#wList .witem').forEach(el => {
    el.addEventListener('click', () => wifiPick(el.dataset.ssid || '', el));
  });
}

function wifiPick(ssid, el) {
  document.querySelectorAll('.witem').forEach(i=>i.classList.remove('sel'));
  el.classList.add('sel');
  document.getElementById('wSsid').value = ssid;
  document.getElementById('wPass').focus();
}

function bleScan() {
  const btn = document.getElementById('bScanBtn');
  btn.disabled = true; btn.innerHTML = '<span class="spin"></span>Сканування BLE (~5 с)...';
  document.getElementById('bList').innerHTML =
    '<div style="padding:16px;text-align:center;color:var(--mut);font-size:11px"><span class="spin"></span>Сканування...</div>';

  fetch(EP_SCAN)
    .then(r => r.json())
    .then(d => renderBleList(d.devices || []))
    .finally(() => { btn.disabled=false; btn.textContent='⟳ Сканувати BLE (5 с)'; });
}

function renderBleList(devs) {
  if (!devs.length) {
    document.getElementById('bList').innerHTML =
      '<div style="padding:16px;text-align:center;color:var(--mut);font-size:11px">BLE пристроїв не знайдено</div>';
    return;
  }
  document.getElementById('bList').innerHTML = devs.map(d => `
    <div class="bitem" data-mac="${escHtml(d.mac)}" data-name="${escHtml(d.name)}">
      <div>
        <div class="bname">${escHtml(d.name)}</div>
        <div class="baddr">${escHtml(d.mac)}</div>
      </div>
    </div>`).join('');
  document.querySelectorAll('#bList .bitem').forEach(el => {
    el.addEventListener('click', () => blePick(el.dataset.mac || '', el.dataset.name || '', el));
  });
}

function blePick(mac, name, el) {
  document.querySelectorAll('.bitem').forEach(i=>i.classList.remove('sel'));
  el.classList.add('sel');
  document.getElementById('bMac').value = mac;
  document.getElementById('bName').value = name;
  toast('Обрано: '+name);
}

function bleConnect() {
  const mac = document.getElementById('bMac').value.trim();
  const name = document.getElementById('bName').value.trim();
  if (!mac) { toast('Оберіть або введіть MAC', 'er'); return; }
  toast('Підключення...', 'wn');
  fetch(EP_CONNECT, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ mac, name })
  })
    .then(r => r.json())
    .then(d => {
      if (d.status === 'connected') toast('✓ BLE підключено: '+name, 'ok');
      else toast('Не вдалося підключитись', 'er');
    })
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function bleDisc() {
  fetch(EP_DISCONNECT)
    .then(r => r.json())
    .then(d => toast('Відключено', 'wn'))
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function wifiSave() {
  const ssid = document.getElementById('wSsid').value.trim();
  const pass = document.getElementById('wPass').value;
  if (!ssid) { toast('Введіть SSID', 'er'); return; }
  
  toast('Підключення до WiFi...', 'wn');
  fetch(EP_WIFICONNECT, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ ssid, password: pass })
  })
    .then(r => r.json())
    .then(d => {
      if (d.status === 'ok') {
        toast(d.message, d.connected ? 'ok' : 'er');
        if (d.connected) setTimeout(() => location.reload(), 2000);
      } else {
        toast('Помилка: ' + d.message, 'er');
      }
    })
    .catch(() => toast('ESP32 не відповідає', 'er'));
}

function handleOTAUpload(e) {
  e.preventDefault();
  try {
    const form = document.getElementById('upload_form');
    if (!form) { toast('Form not found', 'er'); return; }
    const fileInput = form.querySelector('input[type=file]');
    if (!fileInput) { toast('File input not found', 'er'); return; }
    const file = fileInput.files[0];
    if (!file) {
      toast('Оберіть файл firmware.bin', 'er');
      return;
    }
    if (!file.name.endsWith('.bin')) {
      toast('Файл має мати розширення .bin', 'er');
      return;
    }
    let prgBox = document.getElementById('prg');
    let prgVal = document.getElementById('prg_val');
    if (prgBox) { prgBox.style.display = 'block'; }
    if (prgVal) { prgVal.textContent = '0'; }

    const xhr = new XMLHttpRequest();
    xhr.upload.onprogress = function(ev) {
      if (ev.lengthComputable && prgVal) {
        const pct = Math.round((ev.loaded / ev.total) * 100);
        prgVal.textContent = pct;
      }
    };
    xhr.onload = function() {
      if (prgBox) { prgBox.style.display = 'none'; }
      if (xhr.status === 200) {
        try {
          const resp = JSON.parse(xhr.responseText);
          if (resp.status === 'ok') {
            toast('OTA успішно! Пристрій перезавантажиться', 'ok');
          } else {
            toast('Помилка: ' + (resp.message || '?'), 'er');
          }
        } catch(ex) {
          toast('OTA завершено', 'ok');
        }
      } else {
        toast('Помилка OTA: HTTP ' + xhr.status, 'er');
      }
    };
    xhr.onerror = function() {
      if (prgBox) { prgBox.style.display = 'none'; }
      toast('Помилка мережі при OTA', 'er');
    };
    xhr.open('POST', EP_UPDATE);
    xhr.send(new FormData(form));
  } catch(err) {
    console.error('OTA upload error:', err);
    toast('Помилка OTA: ' + err.message, 'er');
  }
}

document.addEventListener('DOMContentLoaded', () => {
  try {
    loadCloudSettings();
  } catch(e) {
    console.error('Cloud settings init error:', e);
  }
  try {
    if (!pollInterval) {
      pollInterval = setInterval(pollData, 2000);
    }
  } catch(e) {
    console.error('Poll interval error:', e);
  }
  try {
    const uploadForm = document.getElementById('upload_form');
    if (uploadForm) {
      uploadForm.addEventListener('submit', handleOTAUpload);
    }
  } catch(e) {
    console.error('OTA form init error:', e);
  }
});
</script>
</body>
</html>
)===";
