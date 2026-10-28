#pragma once

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
        input[type="checkbox"] { width: auto; margin: 0; }
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
        .tabs { display: flex; gap: 0; margin-bottom: 0; overflow-x: auto; overflow-y: hidden; white-space: nowrap; flex-wrap: nowrap; }
        .tabs button { flex: 0 0 auto; padding: 10px 12px; background: #e9ecef; border: none; border-radius: 6px 6px 0 0; cursor: pointer; font-size: 13px; color: #555; transition: 0.2s; }
        .tabs button.active { background: white; color: #007bff; font-weight: normal; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        .tab-desc { font-size: 12px; color: #666; margin: 5px 0 15px 0; line-height: 1.4; }
        .table-wrap { overflow-x: auto; }
        .table-wrap table { min-width: 500px; }
        #channelsTable { min-width: 300px; }
        textarea#examplePayload { width: 100%; box-sizing: border-box; padding: 8px; border: 1px solid #ccc; border-radius: 4px; font-family: monospace; font-size: 12px; resize: vertical; }
        .rule-cond-cell { min-width: 230px; }
        .rule-cond-cell .rname { font-weight: bold; font-size: 14px; margin-bottom: 3px; }
        .rule-cond-cell .rcond { font-size: 12px; color: #555; margin-bottom: 2px; }
        .rule-cond-cell .racts { font-size: 12px; color: #888; }
        .r-togglers { font-size: 12px; color: #888; display: flex; align-items: center; gap: 4px; flex-wrap: wrap; }
        .ch-name-line { font-weight: bold; font-size: 14px; margin-bottom: 3px; }
        .ch-type-line { font-size: 12px; color: #555; }
        #actuatorsList label { display: block; padding: 4px 0; }
        .action-btns { display: flex; gap: 5px; flex-wrap: nowrap; }
        .action-btns button { flex: 1; min-width: 0; }
        @media (max-width: 600px) {
            .row { flex-direction: column; gap: 5px; }
            .row .col { width: 100%; }
            .row button { width: 100% !important; margin-bottom: 5px !important; }
            .action-btns { flex-direction: column; gap: 5px; }
            .action-btns button { width: 100%; }
        }
    </style>
</head>
<body>
    <div class="card">
        <h2>Текущее состояние <span id="deviceNameDisplay" style="font-size:14px;color:#888;"></span></h2>
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
            <button onclick="switchTab(this, 'sensorsTab')">Имена датчиков</button>
            <button onclick="switchTab(this, 'wifi')">Wi-Fi</button>
            <button onclick="switchTab(this, 'telemetry')">Телеметрия</button>
            <button onclick="switchTab(this, 'config')">Конфиг</button>
        </div>
        <div id="tabChannels" class="tab-content active">
            <div class="tab-desc">Управление пинами: добавьте реле для управления, кнопки для ввода и датчики DS18B20 для измерения температуры.</div>
            <div class="table-wrap">
            <table id="channelsTable">
                <thead><tr><th>Канал</th><th style="width:110px">Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            </div>
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
                <button id="addChannelBtn" onclick="addChannel()" style="width: 120px; margin-bottom: 10px;">Добавить</button>
            </div>
        </div>
        <div id="tabSensorsTab" class="tab-content">
            <div class="tab-desc">Задайте удобные имена для температурных датчиков (DS18B20). Имена будут отображаться во всех разделах, а в блоке состояния — имя и код датчика.</div>
            <div class="table-wrap">
            <table id="sensorNamesTable">
                <thead><tr><th>ROM-код датчика</th><th>Имя</th><th style="width:80px">Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            </div>
            <div class="row" style="margin-top:15px;">
                <div class="col"><select id="newSensorRom"></select></div>
                <div class="col"><input type="text" id="newSensorName" placeholder="Имя датчика" maxlength="30"></div>
                <button onclick="addSensorName()" style="width:120px;margin-bottom:10px;">Добавить</button>
            </div>
        </div>
        <div id="tabRules" class="tab-content">
            <div class="tab-desc">Правила автоматизации: условие (температура датчика, состояние кнопки или реле) &rarr; действие (включить реле).</div>
            <div class="table-wrap">
            <table id="rulesTable">
                <thead><tr><th style="width:30px"></th><th style="min-width:230px">Условие</th><th>Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            </div>
            <div class="form-box">
                <h3>Добавить правило</h3>
                <div class="row">
                    <div class="col"><input type="text" id="ruleName" placeholder="Имя правила (до 25 симв.)" maxlength="25"></div>
                    <div class="col"><select id="ruleCondType" onchange="updateRuleSource()">
                        <option value="sensor">Датчик(и)</option>
                        <option value="button">Кнопка</option>
                        <option value="relay">Реле</option>
                    </select></div>
                </div>
                <div class="row">
                    <div class="col"><select id="ruleCombine">
                        <option value="and">И (все условия)</option>
                        <option value="or">ИЛИ (любое условие)</option>
                    </select></div>
                </div>
                <div id="sourceRows"></div>
                <div class="row">
                    <div class="col">
                        <label style="display:flex;align-items:center;gap:6px;"><input type="checkbox" id="ruleEnabled" checked> Работает</label>
                    </div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Включать реле (Actuators):</label>
                        <div id="actuatorsList" style="display:block; margin-top:5px;"></div>
                    </div>
                </div>
                <button id="addRuleBtn" onclick="addRule()">Добавить правило</button>
            </div>
        </div>
        <div id="tabWifi" class="tab-content">
            <div class="tab-desc">Настройка подключения к беспроводной сети. После сохранения устройство перезагрузится с новыми параметрами.</div>
            <div class="form-box">
                <h3>Настройка беспроводной сети</h3>
                <div class="row">
                    <div class="col"><label>Название устройства</label><input type="text" id="deviceName" maxlength="32" placeholder="ESP8266-Universal"></div>
                </div>
                <div class="row">
                    <div class="col"><label>Имя сети (SSID)</label><input type="text" id="wifiSsid" maxlength="32" placeholder="Имя сети Wi-Fi"></div>
                </div>
                <div class="row">
                    <div class="col"><label>Пароль</label><input type="password" id="wifiPass" maxlength="64" placeholder="Пароль (пусто - открытая сеть)"></div>
                </div>
            </div>
        </div>
        <div id="tabTelemetry" class="tab-content">
            <div class="tab-desc">Настройка отправки телеметрии на внешний сервер. Данные содержат показания датчиков, состояние реле/кнопок и статус правил.</div>
            <div class="form-box">
                <h3>Телеметрия</h3>
                <div class="row">
                    <div class="col"><label>Адрес сервера</label><input type="text" id="serverUrl" placeholder="http://192.168.1.100:8080/api"></div>
                </div>
                <div class="row">
                    <div class="col"><label>Период отправки</label><select id="periodSelect">
                        <option value="15">15 сек</option>
                        <option value="30">30 сек</option>
                        <option value="60" selected>1 мин</option>
                        <option value="300">5 мин</option>
                    </select></div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Пример данных, которые будут отправлены:</label>
                        <textarea id="examplePayload" rows="8" readonly></textarea>
                    </div>
                </div>
            </div>
        </div>
        <div id="tabConfig" class="tab-content">
            <div class="tab-desc">Сохранение и восстановление полной конфигурации устройства: каналы, правила, настройки сети и телеметрии.</div>
            <div class="form-box">
                <h3>Управление конфигурацией</h3>
                <div class="row">
                    <div class="col">
                        <label>Скачать текущий конфиг</label>
                        <button onclick="downloadConfig()" style="background:#007bff;">📥 Скачать config.json</button>
                    </div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Загрузить конфиг с компьютера</label>
                        <input type="file" id="configFileInput" accept=".json">
                        <button onclick="uploadConfig()" style="background:#ffc107;color:#000;">📤 Загрузить и применить</button>
                    </div>
                </div>
                <p style="font-size:12px;color:#999;margin-top:10px;">Функционал загрузки конфига будет доработан после тестирования.</p>
            </div>
        </div>
        <button class="save" onclick="saveConfig()">Сохранить конфигурацию</button>
    </div>

    <script>
        const pinNames = {5:"D1", 4:"D2", 0:"D3", 2:"D4", 14:"D5", 12:"D6", 13:"D7"};
        let channels = [];
        let rules = [];
        let sensorNames = [];
        let availableSensors = [];
        let editChannelIndex = -1;
        let editRuleIndex = -1;
        let wifiSsid = '';
        let wifiPass = '';

        // Переключение вкладок
        function switchTab(btn, name) {
            document.querySelectorAll('.tabs button').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
            document.getElementById('tab' + name.charAt(0).toUpperCase() + name.slice(1)).classList.add('active');
            if (name === 'sensorsTab') populateSensorRomDropdown();
        }

        // Получить имя датчика по ROM
        function getSensorName(rom) {
            const found = sensorNames.find(sn => sn.rom === rom);
            return found ? found.name : '';
        }

        // Заполнить выпадающий список ROM-кодов для добавления имени
        function populateSensorRomDropdown() {
            const sel = document.getElementById('newSensorRom');
            const currentRoms = sensorNames.map(sn => sn.rom);
            const available = availableSensors.filter(s => !currentRoms.includes(s.rom));
            sel.innerHTML = '';
            if (available.length === 0) {
                sel.innerHTML = '<option value="">— Нет датчиков —</option>';
            } else {
                available.forEach(s => {
                    const label = getSensorName(s.rom) ? getSensorName(s.rom) + ' (' + s.rom + ')' : s.rom;
                    sel.innerHTML += `<option value="${s.rom}">${label}</option>`;
                });
            }
            renderSensorNames();
        }

        // Рендер таблицы имён датчиков
        function renderSensorNames() {
            const tbody = document.querySelector('#sensorNamesTable tbody');
            tbody.innerHTML = '';
            sensorNames.forEach((sn, i) => {
                tbody.innerHTML += `<tr>
                    <td style="font-family:monospace;font-size:12px;">${sn.rom}</td>
                    <td>${sn.name}</td>
                    <td><button class="del" onclick="removeSensorName(${i})">Удалить</button></td>
                </tr>`;
            });
            if (sensorNames.length === 0) {
                tbody.innerHTML = '<tr><td colspan="3" style="text-align:center;color:#888;">Имена не заданы</td></tr>';
            }
        }

        // Добавить/обновить имя датчика
        function addSensorName() {
            const rom = document.getElementById('newSensorRom').value;
            const name = document.getElementById('newSensorName').value.trim();
            if (!rom) { alert('Нет доступных датчиков!'); return; }
            if (!name) { alert('Введите имя датчика!'); return; }
            // Проверяем, нет ли уже такого ROM
            const existing = sensorNames.findIndex(sn => sn.rom === rom);
            if (existing >= 0) {
                sensorNames[existing].name = name;
            } else {
                sensorNames.push({ rom, name });
            }
            document.getElementById('newSensorName').value = '';
            populateSensorRomDropdown();
            renderTables();
            updateExamplePayload();
        }

        function removeSensorName(i) {
            sensorNames.splice(i, 1);
            populateSensorRomDropdown();
            renderTables();
            updateExamplePayload();
        }

        // Загрузка конфига
        async function loadConfig() {
            const res = await fetch('/api/config');
            const data = await res.json();
            channels = data.channels || [];
            rules = data.rules || [];
            sensorNames = data.sensorNames || [];
            if (data.deviceName) { document.getElementById('deviceName').value = data.deviceName; document.getElementById('deviceNameDisplay').textContent = '— ' + data.deviceName; }
            if (data.serverUrl) document.getElementById('serverUrl').value = data.serverUrl;
            if (data.period) document.getElementById('periodSelect').value = data.period;
            if (data.ssid) { wifiSsid = data.ssid; document.getElementById('wifiSsid').value = data.ssid; }
            if (data.pass) { wifiPass = data.pass; document.getElementById('wifiPass').value = data.pass; }
            renderTables();
            updateExamplePayload();
        }

        // Генерация примера отправляемых данных (телеметрия)
        function updateExamplePayload() {
            // sensorValues — все "датчики" (реле, кнопки, темп. датчики)
            const sensorValues = [];
            // Реле
            channels.filter(c => c.type === 'relay').forEach(c => {
                sensorValues.push({ name: c.name || pinNames[c.pin] || c.pin, value: false, type: 'relay' });
            });
            // Кнопки
            channels.filter(c => c.type === 'button').forEach(c => {
                sensorValues.push({ name: c.name || pinNames[c.pin] || c.pin, value: false, type: 'button' });
            });
            // Температурные датчики (каждый = отдельный элемент)
            availableSensors.forEach(s => {
                const sn = getSensorName(s.rom) || s.rom;
                sensorValues.push({ name: sn, value: s.t, type: 'temperature' });
            });

            const payload = {
                sensors: sensorValues,
                rules: rules.map(r => ({
                    id: r.id,
                    name: r.name || '',
                    enabled: r.enabled,
                    triggered: r.active || false
                }))
            };
            document.getElementById('examplePayload').value = JSON.stringify(payload, null, 2);
        }

        // Загрузка датчиков для списков
        async function loadSensors() {
            const res = await fetch('/api/sensors');
            availableSensors = await res.json();
            populateSensorRomDropdown();
        }

        function renderTables() {
            renderSensorNames();
            // Таблица каналов
            const chBody = document.querySelector('#channelsTable tbody');
            chBody.innerHTML = '';
            channels.forEach((ch, i) => {
                const typeLabel = {relay:'Реле', button:'Кнопка', sensor:'Датчик'}[ch.type] || ch.type;
                chBody.innerHTML += `<tr>
                    <td>
                        <div class="ch-name-line">${ch.name || '—'}</div>
                        <div class="ch-type-line">${typeLabel} &middot; ${pinNames[ch.pin] || ch.pin}</div>
                    </td>
                    <td><button class="toggle" onclick="editChannel(${i})">Изменить</button> <button class="del" onclick="removeChannel(${i})">Удалить</button></td>
                </tr>`;
            });

            // Таблица правил
            const rBody = document.querySelector('#rulesTable tbody');
            rBody.innerHTML = '';
            rules.forEach((r, i) => {
                let condStr = '';
                if (r.condType === 'sensor') {
                    const combineLabel = r.combine === 'or' ? ' ИЛИ ' : '; ';
                    condStr = (r.sensors || []).map((s, idx) => {
                        const sname = getSensorName(s.rom);
                        const modeOp = s.mode ? '≤' : '≥';
                        return `${sname || s.rom} ${modeOp}${Number(s.target).toFixed(1)}°C (h:${Number(s.hysteresis).toFixed(1)})`;
                    }).join(combineLabel);
                } else {
                    const typeLabel = r.condType === 'relay' ? 'Реле' : 'Кнопка';
                    const combineLabel = r.combine === 'or' ? ' ИЛИ ' : '; ';
                    condStr = (r.sourcePins || []).map((p, idx) => {
                        return `${typeLabel}${idx+1}: ${pinNames[p] || p}`;
                    }).join(combineLabel);
                }
                const acts = r.actuators.map(p => pinNames[p] || p).join(', ');
                const isActive = r.active || false;
                
                rBody.innerHTML += `<tr>
                    <td style="text-align:center"><div class="active-dot ${isActive ? 'on' : ''}"></div></td>
                    <td class="rule-cond-cell">
                        <div class="rname">${r.name || '—'}</div>
                        <div class="rcond">${condStr}</div>
                        <div class="r-togglers"><div class="rule-toggle ${r.enabled ? 'on' : ''}" onclick="toggleRule(${i})"></div> <span style="font-size:12px;color:#888;vertical-align:middle;">Работает</span> &middot; → ${acts}</div>
                    </td>
                    <td><button class="toggle" onclick="editRule(${i})">Изменить</button> <button class="del" onclick="removeRule(${i})">Удалить</button></td>
                </tr>`;
            });

            updateRuleSource(); // Обновить выпадающие списки в форме
        }

        // Обновление полей формы в зависимости от типа условия
        function updateRuleSource() {
            renderSourceRows();
            // Обновление чекбоксов для actuators (только реле)
            const actsDiv = document.getElementById('actuatorsList');
            actsDiv.innerHTML = '';
            channels.filter(c => c.type === 'relay').forEach(c => {
                actsDiv.innerHTML += `<label style="display:block;padding:4px 0;white-space:nowrap;"><input type="checkbox" value="${c.pin}" style="width:auto;margin:0;vertical-align:middle;"> <span style="vertical-align:middle;">${pinNames[c.pin] || c.pin}${c.name ? ` (${c.name})` : ''}</span></label>`;
            });
        }

        // Количество строк источников (датчиков/кнопок/реле)
        let currentSourceCount = 1;

        function renderSourceRows(savedValues) {
            const type = document.getElementById('ruleCondType').value;
            const container = document.getElementById('sourceRows');

            // Сохраняем значения текущих строк (если не переданы через savedValues)
            const saved = savedValues || collectSourceRowValues();
            const isSensor = type === 'sensor';
            const addLabel = isSensor ? 'датчик' : (type === 'button' ? 'кнопку' : 'реле');

            let html = '';
            for (let i = 0; i < saved.length; i++) {
                html += `<div class="row source-row" data-idx="${i}">
                    <div class="col"><select id="srcSel${i}" class="src-sel"></select></div>
                    ${isSensor ? `
                    <div class="col" style="flex:0.5;"><input type="number" id="srcTarget${i}" class="src-target" placeholder="Цель °C" step="0.1"></div>
                    <div class="col" style="flex:0.5;"><input type="number" id="srcHyst${i}" class="src-hyst" placeholder="Гистерезис" step="0.1" value="0.5"></div>
                    <div class="col" style="flex:0.3;"><select id="srcMode${i}" class="src-mode">
                        <option value="true">≤</option>
                        <option value="false">≥</option>
                    </select></div>` : ''}
                    <div class="col" style="flex:0.15;">
                        ${i > 0 ? `<button class="del" style="width:auto;padding:4px 8px;margin:0;font-size:12px;" onclick="removeSourceRow(${i})">✕</button>` : ''}
                    </div>
                </div>`;
            }
            if (saved.length < 3) {
                html += `<div style="text-align:right;margin-top:4px;"><button onclick="addSourceRow()" style="background:#17a2b8;font-size:13px;width:auto;padding:5px 12px;">+ Добавить ${addLabel}</button></div>`;
            }
            container.innerHTML = html;

            // Восстанавливаем значения (заполнение селектов + поля)
            for (let i = 0; i < saved.length; i++) {
                const s = saved[i] || { source: '', target: '', hyst: '0.5', mode: 'true' };

                const sel = document.getElementById(`srcSel${i}`);
                sel.innerHTML = `<option value="">— выберите ${addLabel} —</option>`;
                if (isSensor) {
                    availableSensors.forEach(sn => {
                        const sname = getSensorName(sn.rom) || sn.rom;
                        sel.innerHTML += `<option value="${sn.rom}">${sname} (${sn.t}°C)</option>`;
                    });
                } else {
                    channels.filter(c => c.type === type).forEach(c => {
                        const label = pinNames[c.pin] || c.pin;
                        sel.innerHTML += `<option value="${c.pin}">${label}${c.name ? ` (${c.name})` : ''}</option>`;
                    });
                }
                if (s.source !== undefined && s.source !== '') {
                    for (let opt of sel.options) {
                        if (opt.value === String(s.source)) { opt.selected = true; break; }
                    }
                }

                if (isSensor) {
                    const tInp = document.getElementById(`srcTarget${i}`);
                    const hInp = document.getElementById(`srcHyst${i}`);
                    const mSel = document.getElementById(`srcMode${i}`);
                    if (tInp) tInp.value = s.target;
                    if (hInp) hInp.value = s.hyst;
                    if (mSel) mSel.value = s.mode;
                }
            }
        }

        // Сбор значений строк из DOM (по currentSourceCount)
        function collectSourceRowValues() {
            const type = document.getElementById('ruleCondType').value;
            const isSensor = type === 'sensor';
            const saved = [];
            for (let i = 0; i < currentSourceCount; i++) {
                const sel = document.getElementById(`srcSel${i}`);
                if (isSensor) {
                    const tEl = document.getElementById(`srcTarget${i}`);
                    const hEl = document.getElementById(`srcHyst${i}`);
                    const mEl = document.getElementById(`srcMode${i}`);
                    saved.push({
                        source: sel ? sel.value : '',
                        target: tEl ? tEl.value : '',
                        hyst: hEl ? hEl.value : '0.5',
                        mode: mEl ? mEl.value : 'true'
                    });
                } else {
                    saved.push({ source: sel ? sel.value : '' });
                }
            }
            return saved;
        }

        function addSourceRow() {
            if (currentSourceCount < 3) {
                const saved = collectSourceRowValues();
                const obj = document.getElementById('ruleCondType').value === 'sensor'
                    ? { source: '', target: '', hyst: '0.5', mode: 'true' }
                    : { source: '' };
                saved.push(obj);
                currentSourceCount = saved.length;
                renderSourceRows(saved);
            }
        }

        function removeSourceRow(idx) {
            if (currentSourceCount > 1) {
                const saved = collectSourceRowValues();
                saved.splice(idx, 1); // Удаляем именно выбранную строку
                currentSourceCount = saved.length;
                renderSourceRows(saved);
            }
        }

        function editChannel(i) {
            editChannelIndex = i;
            const ch = channels[i];
            document.getElementById('newType').value = ch.type;
            document.getElementById('newPin').value = ch.pin;
            document.getElementById('newName').value = ch.name || '';
            document.getElementById('addChannelBtn').textContent = 'Обновить';
        }

        function addChannel() {
            const type = document.getElementById('newType').value;
            const pin = parseInt(document.getElementById('newPin').value);
            const name = document.getElementById('newName').value.trim();
            
            if (editChannelIndex >= 0) {
                // Режим редактирования
                channels[editChannelIndex].type = type;
                channels[editChannelIndex].pin = pin;
                channels[editChannelIndex].name = name;
                editChannelIndex = -1;
                document.getElementById('addChannelBtn').textContent = 'Добавить';
                renderTables();
                return;
            }
            
            if (channels.some(c => c.pin === pin)) { alert("Пин занят!"); return; }
            channels.push({ type, pin, name });
            renderTables();
        }

        function removeChannel(i) { if (confirm('Удалить канал?')) { channels.splice(i, 1); renderTables(); } }
        
        function toggleRule(i) {
            rules[i].enabled = !rules[i].enabled;
            renderTables();
            silentSave();
        }
        
        function removeRule(i) { if (confirm('Удалить правило?')) { rules.splice(i, 1); renderTables(); } }

        function editRule(i) {
            editRuleIndex = i;
            const r = rules[i];
            document.getElementById('ruleName').value = r.name || '';
            document.getElementById('ruleCondType').value = r.condType || 'sensor';
            document.getElementById('ruleCombine').value = r.combine || 'and';
            document.getElementById('ruleEnabled').checked = r.enabled;

            // Определяем количество строк по количеству источников правила
            if ((r.condType || 'sensor') === 'sensor') {
                const sensors = r.sensors || [];
                currentSourceCount = Math.min(Math.max(sensors.length, 1), 3);
            } else {
                const pins = r.sourcePins || [];
                currentSourceCount = Math.min(Math.max(pins.length, 1), 3);
            }

            updateRuleSource();

            setTimeout(() => {
                const type = r.condType || 'sensor';
                const isSensor = type === 'sensor';
                for (let i = 0; i < currentSourceCount; i++) {
                    const sel = document.getElementById(`srcSel${i}`);
                    const sourceVal = isSensor ? (r.sensors || [])[i]?.rom : (r.sourcePins || [])[i];
                    if (sel && sourceVal !== undefined) {
                        for (let opt of sel.options) {
                            if (opt.value === String(sourceVal)) { opt.selected = true; break; }
                        }
                    }
                    if (isSensor) {
                        const sc = (r.sensors || [])[i];
                        if (!sc) continue;
                        const tInp = document.getElementById(`srcTarget${i}`);
                        const hInp = document.getElementById(`srcHyst${i}`);
                        const mSel = document.getElementById(`srcMode${i}`);
                        if (tInp) tInp.value = sc.target;
                        if (hInp) hInp.value = sc.hysteresis || 0.5;
                        if (mSel) mSel.value = sc.mode ? 'true' : 'false';
                    }
                }

                // Отметить актуаторы
                document.querySelectorAll('#actuatorsList input').forEach(cb => {
                    cb.checked = r.actuators.includes(parseInt(cb.value));
                });
            }, 0);

            document.getElementById('addRuleBtn').textContent = 'Обновить правило';
        }

        function addRule() {
            const name = document.getElementById('ruleName').value.trim();
            const type = document.getElementById('ruleCondType').value;
            
            // Сбор выбранных реле
            const acts = [];
            document.querySelectorAll('#actuatorsList input:checked').forEach(cb => {
                acts.push(parseInt(cb.value));
            });
            
            if (acts.length === 0) { alert("Выберите хотя бы одно реле!"); return; }
            
            const enabled = document.getElementById('ruleEnabled').checked;
            const combine = document.getElementById('ruleCombine').value;
            const rows = collectSourceRowValues();
            
            if (type === 'sensor') {
                // Собираем данные с строк датчиков
                const sensors = [];
                for (let i = 0; i < rows.length; i++) {
                    const row = rows[i];
                    if (!row.source) { alert(`Выберите датчик в строке ${i+1}!`); return; }
                    sensors.push({
                        rom: row.source,
                        target: parseFloat(row.target) || 0,
                        hysteresis: parseFloat(row.hyst) || 0.5,
                        mode: row.mode === 'true'
                    });
                }
                if (sensors.length === 0) { alert("Добавьте хотя бы один датчик!"); return; }

                if (editRuleIndex >= 0) {
                    rules[editRuleIndex].name = name;
                    rules[editRuleIndex].condType = type;
                    rules[editRuleIndex].combine = combine;
                    rules[editRuleIndex].sensors = sensors;
                    rules[editRuleIndex].sourcePins = [];
                    rules[editRuleIndex].actuators = acts;
                    rules[editRuleIndex].enabled = enabled;
                    editRuleIndex = -1;
                    document.getElementById('addRuleBtn').textContent = 'Добавить правило';
                    renderTables(); silentSave(); resetRuleForm();
                    return;
                }
                
                const id = Math.random().toString(16).substr(2, 8);
                rules.push({ id, name, enabled, condType: type, combine, sensors, sourcePins: [], actuators: acts });
            } else {
                // Собираем пины кнопок/реле
                const sourcePins = [];
                for (let i = 0; i < rows.length; i++) {
                    const row = rows[i];
                    if (!row.source) { alert(`Выберите ${type === 'button' ? 'кнопку' : 'реле'} в строке ${i+1}!`); return; }
                    sourcePins.push(parseInt(row.source));
                }
                if (sourcePins.length === 0) { alert("Добавьте хотя бы один источник!"); return; }

                if (editRuleIndex >= 0) {
                    rules[editRuleIndex].name = name;
                    rules[editRuleIndex].condType = type;
                    rules[editRuleIndex].combine = combine;
                    rules[editRuleIndex].sourcePins = sourcePins;
                    rules[editRuleIndex].sensors = [];
                    rules[editRuleIndex].actuators = acts;
                    rules[editRuleIndex].enabled = enabled;
                    editRuleIndex = -1;
                    document.getElementById('addRuleBtn').textContent = 'Добавить правило';
                    renderTables(); silentSave(); resetRuleForm();
                    return;
                }
                
                const id = Math.random().toString(16).substr(2, 8);
                rules.push({ id, name, enabled, condType: type, combine, sensors: [], sourcePins, actuators: acts });
            }
            
            renderTables();
            silentSave();
            resetRuleForm();
        }

        function resetRuleForm() {
            document.getElementById('ruleName').value = '';
            document.getElementById('ruleEnabled').checked = true;
            document.getElementById('ruleCombine').value = 'and';
            // Сбросить селекты актуаторов
            document.querySelectorAll('#actuatorsList input').forEach(cb => cb.checked = false);
            // Сбросить строки источников
            currentSourceCount = 1;
            document.getElementById('ruleCondType').value = 'sensor';
            updateRuleSource();
            // Скролл к таблице правил
            scrollToRules();
        }

        function scrollToRules() {
            const rulesTable = document.getElementById('rulesTable');
            if (rulesTable) rulesTable.scrollIntoView({ behavior: 'smooth', block: 'start' });
        }

        async function silentSave() {
            const config = {
                channels, rules, sensorNames,
                deviceName: document.getElementById('deviceName').value.trim(),
                serverUrl: document.getElementById('serverUrl').value.trim(),
                period: parseInt(document.getElementById('periodSelect').value) || 60,
                ssid: document.getElementById('wifiSsid').value.trim(),
                pass: document.getElementById('wifiPass').value
            };
            try {
                const res = await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(config)
                });
                if (!res.ok) console.warn('silentSave error:', await res.text());
            } catch (e) { console.error(e); }
        }

        async function saveConfig() {
            const config = {
                channels,
                rules,
                sensorNames,
                deviceName: document.getElementById('deviceName').value.trim(),
                serverUrl: document.getElementById('serverUrl').value.trim(),
                period: parseInt(document.getElementById('periodSelect').value) || 60,
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

        async function downloadConfig() {
            const res = await fetch('/api/config');
            const data = await res.json();
            const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'esp_config.json';
            a.click();
            URL.revokeObjectURL(url);
        }

        async function uploadConfig() {
            const fileInput = document.getElementById('configFileInput');
            if (!fileInput.files.length) { alert("Выберите файл конфигурации!"); return; }
            const file = fileInput.files[0];
            const text = await file.text();
            try {
                const config = JSON.parse(text);
                const res = await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        channels: config.channels || [],
                        rules: config.rules || [],
                        sensorNames: config.sensorNames || [],
                        deviceName: config.deviceName || '',
                        serverUrl: config.serverUrl || '',
                        period: config.period || 60,
                        ssid: config.ssid || '',
                        pass: config.pass || ''
                    })
                });
                if (res.ok) alert("Конфиг загружен! Перезагрузка...");
                else alert("Ошибка: " + await res.text());
            } catch (e) {
                alert("Ошибка парсинга JSON: " + e.message);
            }
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
                    const dotClass = r.state ? 'status-dot on' : 'status-dot off';
                    html += `<div class="status-item">
                        <span><span class="${dotClass}"></span>${label}${r.name ? ` (${r.name})` : ''}</span>
                        <button class="toggle" onclick="toggleRelay(${r.pin}, ${!r.state})">Перекл</button>
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
                    const sname = s.name || '';
                    if (sname) {
                        html += `<div class="status-item"><span><b>${sname}</b><br><span style="font-size:11px;color:#999;font-family:monospace;">${s.id}</span>: ${s.t}°C</span></div>`;
                    } else {
                        html += `<div class="status-item"><span><b>${s.id}</b>: ${s.t}°C</span></div>`;
                    }
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
