#pragma once

// Форма настройки Wi-Fi (осталась без изменений)
const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>body{font-family:sans-serif;text-align:center;padding:20px;} input{padding:10px;margin:5px;width:80%;max-width:300px;} button{padding:10px 20px;background:#007bff;color:white;border:none;border-radius:5px;font-size:16px;}</style>
</head><body>
<h2>Настройка Wi-Fi</h2>
<form action="/save" method="POST">
    <input type="text" name="ssid" value="%SSID%" placeholder="Имя сети (SSID)" required><br>
    <input type="password" name="pass" value="" placeholder="Пароль (оставьте пустым для открытой сети)"><br>
    <button type="submit">Сохранить и подключиться</button>
</form>
</body></html>
)rawliteral";

// Главная страница устройства с конфигуратором и правилами
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP8266 Universal</title>
    <style>
        body { font-family: sans-serif; background: #f4f7f6; color: #333; margin: 0; padding: 20px; }
        .card { background: white; max-width: 800px; margin: 0 auto 20px auto; padding: 20px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1, h2 { color: #007bff; text-align: center; }
        table { width: 100%; border-collapse: collapse; margin-top: 15px; }
        th, td { padding: 10px; border-bottom: 1px solid #ddd; text-align: left; font-size: 14px;}
        th { background-color: #f8f9fa; }
        input, select { padding: 8px; width: 100%; box-sizing: border-box; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px;}
        button { background: #28a745; color: white; border: none; padding: 10px; border-radius: 5px; cursor: pointer; font-size: 16px; }
        button.del { background: #dc3545; width: auto; padding: 5px 10px; margin: 0; font-size: 14px;}
        button.toggle { background: #ffc107; color: #000; padding: 5px 10px; width: auto; font-size: 14px; margin: 0;}
        button.save { background: #007bff; width: 100%; margin-top: 15px; }
        
        .row { display: flex; gap: 10px; align-items: center; margin-bottom: 10px;}
        .col { flex: 1; }
        .status-group { display: flex; flex-wrap: wrap; gap: 15px; }
        .status-group > div { flex: 1; min-width: 200px; background: #f9f9f9; border-radius: 6px; padding: 8px 10px; border: 1px solid #e0e0e0; }
        .status-group h3 { font-size: 13px; margin: 0 0 5px 0; color: #555; text-transform: uppercase; letter-spacing: 0.5px; }
        .status-item { display: flex; justify-content: space-between; align-items: center; padding: 4px 0; border-bottom: 1px solid #eee; font-size: 13px; gap: 8px; }
        .status-item:last-child { border-bottom: none; }
        .status-item .toggle { padding: 3px 8px; font-size: 12px; white-space: nowrap; }
        .status-item .toggle.on { background: #28a745; color: white; }
        .status-item .toggle.off { background: #dc3545; color: white; }
        .status-dot { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 4px; vertical-align: middle; }
        .status-dot.on { background: #28a745; box-shadow: 0 0 4px #28a745; }
        .status-dot.off { background: #dc3545; }
        
        .rule-toggle { cursor: pointer; display: inline-block; width: 40px; height: 20px; background: #ccc; border-radius: 10px; position: relative; transition: 0.3s; vertical-align: middle;}
        .rule-toggle.on { background: #28a745; }
        .rule-toggle::after { content: ''; position: absolute; width: 16px; height: 16px; background: white; border-radius: 50%; top: 2px; left: 2px; transition: 0.3s; }
        .rule-toggle.on::after { left: 22px; }
        .active-dot { display: inline-block; width: 12px; height: 12px; border-radius: 50%; background: #ccc; }
        .active-dot.on { background: #28a745; box-shadow: 0 0 5px #28a745; }
        .form-box { background: #f9f9f9; padding: 15px; border-radius: 5px; border: 1px solid #ddd; margin-top: 15px; }
        .tabs { display: flex; gap: 0; margin-bottom: 0; border-bottom: 2px solid #007bff; }
        .tabs button { flex: 1; padding: 10px; background: #e9ecef; border: none; border-radius: 6px 6px 0 0; cursor: pointer; font-size: 14px; color: #555; transition: 0.2s; margin-bottom: -2px; border-bottom: 2px solid transparent; }
        .tabs button.active { background: white; color: #007bff; font-weight: bold; border-bottom: 2px solid #007bff; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        textarea#examplePayload { width: 100%; box-sizing: border-box; padding: 8px; border: 1px solid #ccc; border-radius: 4px; font-family: monospace; font-size: 12px; resize: vertical; }
    </style>
</head>
<body>
    <div class="card">
        <h2>Текущее состояние</h2>
        <div class="status-group">
            <div><h3>Реле</h3><div id="relayStatus"></div></div>
            <div><h3>Кнопки</h3><div id="buttonStatus"></div></div>
            <div><h3>Датчики</h3><div id="sensorStatus"></div></div>
        </div>
    </div>

    <div class="card">
        <div class="tabs">
            <button class="active" onclick="switchTab(this, 'channels')">Каналы</button>
            <button onclick="switchTab(this, 'rules')">Правила</button>
            <button onclick="switchTab(this, 'wifi')">Wi-Fi</button>
            <button onclick="switchTab(this, 'http')">HTTP</button>
        </div>
        <div id="tabChannels" class="tab-content active">
            <table id="channelsTable">
                <thead><tr><th>Тип</th><th>Пин</th><th>Имя</th><th>Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            <div class="row" style="margin-top: 15px;">
                <div class="col"><select id="newType">
                    <option value="relay">Реле</option>
                    <option value="button">Кнопка</option>
                    <option value="sensor">Датчик (DS18B20)</option>
                </select></div>
                <div class="col"><select id="newPin">
                    <option value="5">D1 (GPIO5)</option>
                    <option value="4">D2 (GPIO4)</option>
                    <option value="0">D3 (GPIO0)</option>
                    <option value="2">D4 (GPIO2)</option>
                    <option value="14">D5 (GPIO14)</option>
                    <option value="12">D6 (GPIO12)</option>
                    <option value="13">D7 (GPIO13)</option>
                </select></div>
                <div class="col"><input type="text" id="newName" placeholder="Имя (до 25 симв.)" maxlength="25"></div>
                <button onclick="addChannel()" style="width: 120px; margin-bottom: 10px;">Добавить</button>
            </div>
        </div>
        <div id="tabRules" class="tab-content">
            <table id="rulesTable">
                <thead><tr><th>Имя</th><th>Условие</th><th>Действие</th><th>Работает</th><th>Сработало</th><th>Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            <div class="form-box">
                <h3>Добавить правило</h3>
                <div class="row">
                    <div class="col"><input type="text" id="ruleName" placeholder="Имя правила (до 25 симв.)" maxlength="25"></div>
                    <div class="col"><select id="ruleCondType" onchange="updateRuleSource()">
                        <option value="sensor">Датчик</option>
                        <option value="button">Кнопка</option>
                        <option value="relay">Реле</option>
                    </select></div>
                </div>
                <div class="row">
                    <div class="col"><select id="ruleSource"></select></div>
                    <div class="col" id="sensorParams" style="display:flex; gap:5px;">
                        <input type="number" id="ruleTarget" placeholder="Target °C" step="0.1">
                        <input type="number" id="ruleHyst" placeholder="Гистерезис" step="0.1" value="0.5">
                    </div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Включать реле (Actuators):</label>
                        <div id="actuatorsList" style="display:flex; flex-wrap:wrap; gap:10px; margin-top:5px;"></div>
                    </div>
                </div>
                <button onclick="addRule()">Добавить правило</button>
            </div>
        </div>
        <div id="tabWifi" class="tab-content">
            <div class="form-box">
                <h3>Настройка беспроводной сети</h3>
                <div class="row">
                    <div class="col"><label>Имя сети (SSID)</label><input type="text" id="wifiSsid" maxlength="32" placeholder="Имя сети Wi-Fi"></div>
                </div>
                <div class="row">
                    <div class="col"><label>Пароль</label><input type="password" id="wifiPass" maxlength="64" placeholder="Пароль (пусто - открытая сеть)"></div>
                </div>
            </div>
        </div>
        <div id="tabHttp" class="tab-content">
            <div class="form-box">
                <h3>HTTP интеграция</h3>
                <div class="row">
                    <div class="col"><label>Адрес сервера</label><input type="text" id="serverUrl" placeholder="http://192.168.1.100:8080/api"></div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Пример данных, которые будут отправлены:</label>
                        <textarea id="examplePayload" rows="8" readonly></textarea>
                    </div>
                </div>
            </div>
        </div>
        <button class="save" onclick="saveConfig()">Сохранить конфигурацию</button>
    </div>

    <script>
        const pinNames = {5:"D1", 4:"D2", 0:"D3", 2:"D4", 14:"D5", 12:"D6", 13:"D7"};
        let channels = [];
        let rules = [];
        let availableSensors = [];
        let wifiSsid = '';
        let wifiPass = '';

        // Переключение вкладок
        function switchTab(btn, name) {
            document.querySelectorAll('.tabs button').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
            document.getElementById('tab' + name.charAt(0).toUpperCase() + name.slice(1)).classList.add('active');
        }

        // Загрузка конфига
        async function loadConfig() {
            const res = await fetch('/api/config');
            const data = await res.json();
            channels = data.channels || [];
            rules = data.rules || [];
            if (data.serverUrl) document.getElementById('serverUrl').value = data.serverUrl;
            if (data.ssid) { wifiSsid = data.ssid; document.getElementById('wifiSsid').value = data.ssid; }
            renderTables();
            updateExamplePayload();
        }

        // Генерация примера отправляемых данных
        function updateExamplePayload() {
            const payload = {
                device: "ESP8266",
                channels: channels.map(c => ({
                    type: c.type,
                    pin: c.pin,
                    name: c.name || ''
                })),
                rules: rules.map(r => ({
                    name: r.name || '',
                    enabled: r.enabled,
                    condition: r.cond.type === 'sensor' ? `${r.cond.rom} <= ${r.cond.target}°C` : `pin ${r.cond.pin}`,
                    actuators: r.actuators
                }))
            };
            document.getElementById('examplePayload').value = JSON.stringify(payload, null, 2);
        }

        // Загрузка датчиков для списков
        async function loadSensors() {
            const res = await fetch('/api/sensors');
            availableSensors = await res.json();
        }

        function renderTables() {
            // Таблица каналов
            const chBody = document.querySelector('#channelsTable tbody');
            chBody.innerHTML = '';
            channels.forEach((ch, i) => {
                chBody.innerHTML += `<tr>
                    <td>${ch.type}</td>
                    <td>${pinNames[ch.pin] || ch.pin}</td>
                    <td><input type="text" value="${ch.name || ''}" maxlength="25" onchange="channels[${i}].name=this.value" style="width:100%;box-sizing:border-box;padding:2px;border:1px solid #ddd;border-radius:3px;"></td>
                    <td><button class="del" onclick="removeChannel(${i})">Удалить</button></td>
                </tr>`;
            });

            // Таблица правил
            const rBody = document.querySelector('#rulesTable tbody');
            rBody.innerHTML = '';
            rules.forEach((r, i) => {
                let condStr = '';
                if (r.cond.type === 'sensor') {
                    condStr = `Датчик ${r.cond.rom} <= ${r.cond.target}°C (h:${r.cond.hysteresis})`;
                } else {
                    condStr = `${r.cond.type === 'relay' ? 'Реле' : 'Кнопка'} ${pinNames[r.cond.pin] || r.cond.pin}`;
                }
                const acts = r.actuators.map(p => pinNames[p] || p).join(', ');
                
                // Индикатор active (берем из последнего статуса)
                const isActive = r.active || false;
                
                rBody.innerHTML += `<tr>
                    <td><input type="text" value="${r.name || ''}" maxlength="25" onchange="rules[${i}].name=this.value" style="width:100%;box-sizing:border-box;padding:2px;border:1px solid #ddd;border-radius:3px;"></td>
                    <td>${condStr}</td>
                    <td>${acts}</td>
                    <td>
                        <div class="rule-toggle ${r.enabled ? 'on' : ''}" onclick="toggleRule(${i})"></div>
                    </td>
                    <td><div class="active-dot ${isActive ? 'on' : ''}"></div></td>
                    <td><button class="del" onclick="removeRule(${i})">Удалить</button></td>
                </tr>`;
            });

            updateRuleSource(); // Обновить выпадающие списки в форме
        }

        // Обновление полей формы в зависимости от типа условия
        function updateRuleSource() {
            const type = document.getElementById('ruleCondType').value;
            const srcSelect = document.getElementById('ruleSource');
            const sensorParams = document.getElementById('sensorParams');
            
            srcSelect.innerHTML = '';
            if (type === 'sensor') {
                sensorParams.style.display = 'flex';
                availableSensors.forEach(s => {
                    srcSelect.innerHTML += `<option value="${s.rom}">${s.rom} (${s.t}°C)</option>`;
                });
            } else {
                sensorParams.style.display = 'none';
                channels.filter(c => c.type === type).forEach(c => {
                    srcSelect.innerHTML += `<option value="${c.pin}">${pinNames[c.pin] || c.pin} (${c.name || 'без имени'})</option>`;
                });
            }

            // Обновление чекбоксов для actuators (только реле)
            const actsDiv = document.getElementById('actuatorsList');
            actsDiv.innerHTML = '';
            channels.filter(c => c.type === 'relay').forEach(c => {
                actsDiv.innerHTML += `<label><input type="checkbox" value="${c.pin}"> ${pinNames[c.pin] || c.pin}${c.name ? ` (${c.name})` : ''}</label>`;
            });
        }

        function addChannel() {
            const type = document.getElementById('newType').value;
            const pin = parseInt(document.getElementById('newPin').value);
            const name = document.getElementById('newName').value.trim();
            
            if (channels.some(c => c.pin === pin)) { alert("Пин занят!"); return; }
            channels.push({ type, pin, name });
            renderTables();
        }

        function removeChannel(i) { channels.splice(i, 1); renderTables(); }
        
        function toggleRule(i) {
            rules[i].enabled = !rules[i].enabled;
            renderTables();
        }
        
        function removeRule(i) { rules.splice(i, 1); renderTables(); }

        function addRule() {
            const name = document.getElementById('ruleName').value.trim();
            const type = document.getElementById('ruleCondType').value;
            const source = document.getElementById('ruleSource').value;
            
            // Сбор выбранных реле
            const acts = [];
            document.querySelectorAll('#actuatorsList input:checked').forEach(cb => {
                acts.push(parseInt(cb.value));
            });
            
            if (acts.length === 0) { alert("Выберите хотя бы одно реле!"); return; }
            
            let cond = { type };
            if (type === 'sensor') {
                cond.rom = source;
                cond.target = parseFloat(document.getElementById('ruleTarget').value) || 0;
                cond.hysteresis = parseFloat(document.getElementById('ruleHyst').value) || 0.5;
            } else {
                cond.pin = parseInt(source);
            }
            
            // Генерация простого ID
            const id = Math.random().toString(16).substr(2, 8);
            
            rules.push({ id, name, enabled: true, cond, actuators: acts });
            renderTables();
        }

        async function saveConfig() {
            const config = {
                channels,
                rules,
                serverUrl: document.getElementById('serverUrl').value.trim(),
                ssid: document.getElementById('wifiSsid').value.trim(),
                pass: document.getElementById('wifiPass').value
            };
            const res = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            
            if (res.ok) alert("Сохранено! Перезагрузка...");
            else alert("Ошибка валидации: " + await res.text());
        }

        async function updateStatus() {
            try {
                const res = await fetch('/api/status');
                const data = await res.json();

                // Обновляем флаги active в таблице правил (без полной перерисовки)
                if (data.rules) {
                    data.rules.forEach((r, i) => {
                        if (rules[i]) rules[i].active = r.active;
                    });
                    // Точечно обновляем классы
                    document.querySelectorAll('.active-dot').forEach((dot, i) => {
                        dot.className = 'active-dot ' + (rules[i]?.active ? 'on' : '');
                    });
                }

                // Реле
                let html = '';
                data.relays.forEach(r => {
                    const label = pinNames[r.pin] || r.pin;
                    const btnClass = r.state ? 'toggle on' : 'toggle off';
                    html += `<div class="status-item">
                        <span>${label}${r.name ? ` (${r.name})` : ''}</span>
                        <button class="${btnClass}" onclick="toggleRelay(${r.pin}, ${!r.state})">${r.state ? 'Выкл' : 'Вкл'}</button>
                    </div>`;
                });
                document.getElementById('relayStatus').innerHTML = html || '<i>Нет реле</i>';

                // Кнопки
                html = '';
                data.buttons.forEach(b => {
                    const label = pinNames[b.pin] || b.pin;
                    const dotClass = b.pressed ? 'status-dot on' : 'status-dot off';
                    html += `<div class="status-item"><span><span class="${dotClass}"></span>${label}${b.name ? ` (${b.name})` : ''}</span></div>`;
                });
                document.getElementById('buttonStatus').innerHTML = html || '<i>Нет кнопок</i>';

                // Датчики
                html = '';
                data.sensors.forEach(s => {
                    html += `<div class="status-item"><span><b>${s.id}</b>: ${s.t}°C</span></div>`;
                });
                document.getElementById('sensorStatus').innerHTML = html || '<i>Нет датчиков</i>';

            } catch (e) { console.error(e); }
        }

        async function toggleRelay(pin, state) {
            await fetch('/api/relay', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ pin, state })
            });
            updateStatus();
        }

        window.onload = async () => {
            await loadSensors();
            await loadConfig();
            setInterval(updateStatus, 2000);
        };
    </script>
</body>
</html>
)rawliteral";