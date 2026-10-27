#pragma once

// Форма настройки Wi-Fi со сканированием, списком сетей и спиннером
const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:sans-serif;text-align:center;padding:20px;background:#f4f7f6;}
.card{background:white;max-width:400px;margin:0 auto;padding:20px;border-radius:10px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}
h2{color:#333;}
input,select{padding:10px;margin:5px 0;width:100%;max-width:100%;border:1px solid #ccc;border-radius:4px;font-size:16px;box-sizing:border-box;}
button{padding:10px 20px;background:#007bff;color:white;border:none;border-radius:5px;font-size:16px;cursor:pointer;margin:5px 0;width:100%;}
button:disabled{opacity:0.6;cursor:not-allowed;}
#scanBtn{background:#28a745;}
#scanResult{max-height:200px;overflow-y:auto;margin:10px 0;text-align:left;}
.net-item{cursor:pointer;padding:10px;margin:3px 0;background:#e9ecef;border-radius:4px;border:1px solid #ddd;transition:0.2s;}
.net-item:hover{background:#d0d7de;border-color:#007bff;}
.spinner{display:none;border:4px solid #f3f3f3;border-top:4px solid #007bff;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:20px auto;}
@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
#checking{display:none;}
#error{color:#dc3545;display:none;background:#ffe0e0;padding:10px;border-radius:4px;margin:10px 0;}
</style>
</head><body>
<div class="card">
<h2>Настройка Wi-Fi</h2>
<button id="scanBtn" onclick="scanNetworks()">📡 Сканировать сети</button>
<div id="scanResult"></div>
<form id="wifiForm" action="/save" method="POST" onsubmit="return onSubmit()">
    <input type="text" id="ssid" name="ssid" value="%SSID%" placeholder="Имя сети (SSID)" required autocomplete="off"><br>
    <input type="password" id="pass" name="pass" value="" placeholder="Пароль (оставьте пустым для открытой сети)"><br>
    <div id="error"></div>
    <button type="submit" id="submitBtn">💾 Сохранить и подключиться</button>
</form>
<div id="checking">
    <div class="spinner"></div>
    <p>Подключение к сети...<br><small>Пожалуйста, подождите</small></p>
</div>
</div>
<script>
async function scanNetworks() {
    const btn = document.getElementById('scanBtn');
    const result = document.getElementById('scanResult');
    btn.disabled = true;
    btn.textContent = '⏳ Сканирование...';
    result.innerHTML = '<p style="color:#888;">Поиск сетей...</p>';
    try {
        const res = await fetch('/api/scan');
        const list = await res.json();
        if (!list || list.length === 0) {
            result.innerHTML = '<p style="color:#888;">Сетей не найдено</p>';
        } else {
            result.innerHTML = list.map(function(n, idx) {
                var icon = n.rssi > -60 ? '📶' : (n.rssi > -75 ? '📡' : '📡');
                var lock = n.open ? '' : '🔒';
                var safeSsid = n.ssid.replace(/"/g, '&quot;').replace(/'/g, '&#39;');
                return '<div class="net-item" data-ssid="' + safeSsid + '" onclick="selectNetwork(this)">'
                    + icon + ' ' + lock + ' <b>' + n.ssid + '</b> <span style="color:#888;font-size:12px;">(' + n.rssi + ' dBm)</span>'
                    + '</div>';
            }).join('');
        }
    } catch(e) {
        result.innerHTML = '<p style="color:#dc3545;">Ошибка сканирования</p>';
    }
    btn.disabled = false;
    btn.textContent = '📡 Сканировать сети';
}
function selectNetwork(el) {
    document.getElementById('ssid').value = el.dataset.ssid;
    document.getElementById('pass').focus();
    document.getElementById('scanResult').innerHTML = '';
}
function onSubmit() {
    document.getElementById('submitBtn').style.display = 'none';
    document.getElementById('scanBtn').style.display = 'none';
    document.getElementById('scanResult').style.display = 'none';
    document.getElementById('checking').style.display = 'block';
    return true;
}
</script>
</body></html>
)rawliteral";