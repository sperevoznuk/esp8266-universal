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
        button.edit { background: #007bff; width: auto; padding: 5px 10px; margin: 0; font-size: 14px;}
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
        .fw-status { display: none; margin-top: 12px; padding: 12px; border-radius: 6px; font-size: 14px; text-align: center; }
        .fw-status.uploading { display: block; background: #e7f0ff; color: #0056b3; border: 1px solid #a8c8ff; }
        .fw-status.ok { display: block; background: #e6f9ec; color: #1a7f37; border: 1px solid #9ce0b0; }
        .fw-status.error { display: block; background: #ffe9e9; color: #c0392b; border: 1px solid #f0b4b4; }
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
        /* Toast-уведомления */
        .toast-container { position: fixed; top: 20px; left: 50%; transform: translateX(-50%); z-index: 9999; display: flex; flex-direction: column; gap: 10px; width: 90%; max-width: 450px; pointer-events: none; }
        .toast { background: #333; color: #fff; padding: 14px 18px; border-radius: 8px; box-shadow: 0 6px 20px rgba(0,0,0,0.25); font-size: 14px; line-height: 1.4; display: flex; align-items: center; gap: 12px; pointer-events: auto; animation: toastIn 0.3s ease; }
        .toast.success { background: #28a745; }
        .toast.info { background: #007bff; }
        .toast.warning { background: #ffc107; color: #333; }
        .toast .toast-msg { flex: 1; }
        .toast .toast-btn { background: rgba(255,255,255,0.25); border: 1px solid rgba(255,255,255,0.4); color: inherit; padding: 6px 14px; border-radius: 5px; cursor: pointer; font-size: 13px; white-space: nowrap; transition: 0.2s; }
        .toast .toast-btn:hover { background: rgba(255,255,255,0.4); }
        .toast .toast-close { background: none; border: none; color: inherit; font-size: 18px; cursor: pointer; opacity: 0.6; padding: 0 4px; }
        .toast .toast-close:hover { opacity: 1; }
        @keyframes toastIn { from { opacity: 0; transform: translateY(-20px); } to { opacity: 1; transform: translateY(0); } }
        @keyframes toastOut { from { opacity: 1; transform: translateY(0); } to { opacity: 0; transform: translateY(-20px); } }
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
            <button onclick="switchTab(this, 'firmware')">Прошивка</button>
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
                <thead><tr><th>ROM-код датчика</th><th>Имя</th><th>Смещение (°C)</th><th style="width:150px">Действие</th></tr></thead>
                <tbody></tbody>
            </table>
            </div>
            <div class="row" style="margin-top:15px;">
                <button id="addSensorBtn" onclick="openSensorForm()" style="background:#28a745;width:auto;padding:10px 15px;">➕ Добавить датчик</button>
            </div>
            <div id="sensorForm" class="form-box" style="display:none;margin-top:10px;">
                <h3 id="sensorFormTitle" style="margin:0 0 10px 0;font-size:15px;color:#333;">Добавление датчика</h3>
                <div class="row">
                    <div class="col"><label>ROM датчика</label><select id="newSensorRom"></select></div>
                    <div class="col"><label>Имя</label><input type="text" id="newSensorName" placeholder="Имя датчика" maxlength="30"></div>
                    <div class="col"><label>Смещение (°C)</label><input type="number" id="newSensorOffset" placeholder="Смещение (напр. 4.0)" step="0.1" value="0.0" style="width:100%;"></div>
                </div>
                <div class="row" style="margin-top:10px;justify-content:flex-end;">
                    <button onclick="closeSensorForm()" style="background:#6c757d;width:auto;padding:10px 15px;">Отмена</button>
                    <button onclick="saveSensorName()" style="background:#007bff;width:auto;padding:10px 15px;">💾 Сохранить</button>
                </div>
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
                <button onclick="saveWifiSettings()" style="background:#007bff;width:100%;">💾 Сохранить настройки сети</button>
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
                        <option value="0">Вручную</option>
                        <option value="15">15 сек</option>
                        <option value="30">30 сек</option>
                        <option value="60">1 мин</option>
                        <option value="300">5 мин</option>
                    </select></div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Пример данных, которые будут отправлены:</label>
                        <textarea id="examplePayload" rows="8" readonly></textarea>
                    </div>
                </div>
                <div class="row">
                    <div class="col" style="display:flex;gap:10px;">
                        <button onclick="saveTelemetrySettings()" style="background:#17a2b8;flex:1;">Сохранить телеметрию</button>
                        <button onclick="testTelemetry()" style="background:#007bff;flex:1;">Проверить</button>
                    </div>
                </div>
                <div id="telemetryLog" style="background:#f8f9fa; padding:10px; border-radius:5px; font-size:12px; font-family:monospace; max-height:200px; overflow-y:auto; margin-top:10px; border:1px solid #ddd; color:#333;"></div>
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
                <div class="row">
                    <div class="col">
                        <label>Перезагрузка устройства</label>
                        <button onclick="if(confirm('Перезагрузить устройство?')) rebootDevice()" style="background:#dc3545;">🔄 Перезагрузить устройство</button>
                    </div>
                </div>
                <p style="font-size:12px;color:#999;margin-top:10px;">Функционал загрузки конфига будет доработан после тестирования.</p>
            </div>
        </div>
        <div id="tabFirmware" class="tab-content">
            <div class="tab-desc">Обновление прошивки устройства загрузкой файла (.bin). Во время обновления не выключайте питание и не закрывайте страницу.</div>
            <div class="form-box">
                <h3>Обновление прошивки</h3>
                <div class="row">
                    <div class="col"><label>Текущая версия прошивки</label><input type="text" id="fwVersion" value="—" readonly style="background:#f0f0f0;"></div>
                </div>
                <div class="row">
                    <div class="col">
                        <label>Файл прошивки</label>
                        <input type="file" id="firmwareFileInput" accept=".bin">
                        <p style="font-size:12px;color:#888;margin:5px 0 0 0;">Файл собирается PlatformIO: <code>.pio/build/esp8266/firmware.bin</code></p>
                    </div>
                </div>
                <div class="row">
                    <div class="col">
                        <button id="fwUploadBtn" onclick="uploadFirmware()" style="background:#28a745;width:100%;">📤 Обновить прошивку</button>
                    </div>
                </div>
                <div id="fwStatus" class="fw-status"></div>
            </div>
        </div>
        </div>
    <div class="toast-container" id="toastContainer"></div>

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

        // --- Уведомления (Toast) ---
        function showNotification(msg, showRebootBtn) {
            const container = document.getElementById('toastContainer');
            
            // Если уведомление требует перезагрузки — удаляем все предыдущие
            if (showRebootBtn) {
                container.innerHTML = '';
            }
            
            const toast = document.createElement('div');
            toast.className = 'toast info';
            
            let html = `<span class="toast-msg">${msg}</span>`;
            if (showRebootBtn) {
                html += `<button class="toast-btn" onclick="rebootDevice()">🔄 Перезагрузить</button>`;
            }
            html += `<button class="toast-close" onclick="dismissToast(this)">✕</button>`;
            toast.innerHTML = html;
            container.appendChild(toast);
            
            if (!showRebootBtn) {
                setTimeout(() => {
                    if (toast.parentNode) {
                        toast.style.animation = 'toastOut 0.3s ease';
                        setTimeout(() => { if (toast.parentNode) toast.remove(); }, 300);
                    }
                }, 5000);
            }
        }

        function dismissToast(btn) {
            const toast = btn.closest('.toast');
            if (toast) { toast.style.animation = 'toastOut 0.3s ease'; setTimeout(() => { if (toast.parentNode) toast.remove(); }, 300); }
        }

        // --- Фильтрация списка пинов (скрытие занятых) ---
        function populatePinDropdown() {
            const select = document.getElementById('newPin');
            const usedPins = channels.map(c => c.pin);
            // Если в режиме редактирования — исключаем текущий пин канала
            if (editChannelIndex >= 0) {
                const curPin = channels[editChannelIndex].pin;
                const idx = usedPins.indexOf(curPin);
                if (idx >= 0) usedPins.splice(idx, 1);
            }
            
            // Удаляем статичные плейсхолдеры из предыдущих вызовов
            const oldPh = document.getElementById('newPinPlaceholder');
            if (oldPh) oldPh.remove();
            
            let anyVisible = false;
            for (let opt of select.options) {
                const val = parseInt(opt.value);
                if (isNaN(val)) continue; // пропускаем нечисловые option
                const isUsed = usedPins.includes(val);
                opt.disabled = isUsed;
                opt.style.display = isUsed ? 'none' : '';
                // Убираем selected с занятых пинов
                if (isUsed && opt.selected) opt.selected = false;
                if (!isUsed) anyVisible = true;
            }
            
            // Если ни один пин не доступен — показываем плейсхолдер
            if (!anyVisible && editChannelIndex < 0) {
                const ph = document.createElement('option');
                ph.value = '';
                ph.disabled = true;
                ph.selected = true;
                ph.textContent = '— Все пины заняты —';
                ph.id = 'newPinPlaceholder';
                select.insertBefore(ph, select.firstChild);
            }
        }

        // --- Переключение вкладок ---
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
            if (sel && sel.disabled) return; // форма в режиме редактирования ROM
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
                const offsetStr = sn.offset > 0 ? '+' + Number(sn.offset).toFixed(1) : Number(sn.offset).toFixed(1);
                tbody.innerHTML += `<tr>
                    <td style="font-family:monospace;font-size:12px;">${sn.rom}</td>
                    <td>${sn.name}</td>
                    <td><b>${offsetStr}</b></td>
                    <td>
                        <button class="edit" onclick="openSensorForm('${sn.rom}')">Изменить</button>
                        <button class="del" onclick="removeSensorName(${i})">Удалить</button>
                    </td>
                </tr>`;
            });
            if (sensorNames.length === 0) {
                tbody.innerHTML = '<tr><td colspan="4" style="text-align:center;color:#888;">Имена не заданы</td></tr>';
            }
        }

        // --- Имена датчиков: скрытая форма добавления/редактирования ---

        // Открыть форму (без аргумента — добавление, с rom — редактирование)
        function openSensorForm(rom) {
            document.getElementById('sensorForm').style.display = 'block';
            document.getElementById('addSensorBtn').style.display = 'none';
            document.getElementById('sensorFormTitle').textContent = rom ? 'Изменение датчика' : 'Добавление датчика';

            // Наполняем список ROM: в режиме редактирования rома должна быть в списке,
            // даже если датчик временно не отвечает
            const sel = document.getElementById('newSensorRom');
            const currentRoms = sensorNames.map(sn => sn.rom);
            let available = availableSensors.filter(s => !currentRoms.includes(s.rom) || s.rom === rom);
            if (rom && !available.some(s => s.rom === rom)) available.push({ rom });

            sel.innerHTML = '';
            if (available.length === 0) {
                sel.innerHTML = '<option value="">— Нет датчиков —</option>';
            } else {
                available.forEach(s => {
                    const label = getSensorName(s.rom) ? getSensorName(s.rom) + ' (' + s.rom + ')' : s.rom;
                    sel.innerHTML += `<option value="${s.rom}">${label}</option>`;
                });
            }

            if (rom) {
                const sn = sensorNames.find(x => x.rom === rom);
                sel.value = rom;
                sel.disabled = true; // ROM при редактировании менять нельзя
                document.getElementById('newSensorName').value = sn ? sn.name : '';
                document.getElementById('newSensorOffset').value = sn ? sn.offset : 0.0;
            } else {
                sel.disabled = false;
                document.getElementById('newSensorName').value = '';
                document.getElementById('newSensorOffset').value = '0.0';
            }
        }

        // Скрыть форму и вернуть кнопку «Добавить датчик»
        function closeSensorForm() {
            const form = document.getElementById('sensorForm');
            if (form) form.style.display = 'none';
            const btn = document.getElementById('addSensorBtn');
            if (btn) btn.style.display = 'block';
            const sel = document.getElementById('newSensorRom');
            if (sel) sel.disabled = false;
            populateSensorRomDropdown();
        }

        // Сохранить (добавить или обновить) имя датчика
        function saveSensorName() {
            const rom = document.getElementById('newSensorRom').value;
            const name = document.getElementById('newSensorName').value.trim();
            const offset = parseFloat(document.getElementById('newSensorOffset').value) || 0.0;

            if (!rom) { alert('Нет доступных датчиков!'); return; }
            if (!name) { alert('Введите имя датчика!'); return; }

            const existing = sensorNames.findIndex(sn => sn.rom === rom);
            let action = 'добавлено';
            if (existing >= 0) {
                sensorNames[existing].name = name;
                sensorNames[existing].offset = offset;
                action = 'обновлено';
            } else {
                sensorNames.push({ rom, name, offset });
            }

            closeSensorForm();
            renderTables();
            updateExamplePayload();
            silentSave();
            showNotification('Имя датчика ' + action);
        }

        function removeSensorName(i) {
            const rom = sensorNames[i].rom;
            sensorNames.splice(i, 1);
            // Если удалённый датчик редактируется в открытой форме — закрываем её
            const form = document.getElementById('sensorForm');
            if (form && form.style.display === 'block' && document.getElementById('newSensorRom').value === rom) {
                closeSensorForm();
            } else {
                populateSensorRomDropdown();
            }
            renderTables();
            updateExamplePayload();
            silentSave();
            showNotification('Имя датчика удалено');
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
            if (data.period !== undefined) document.getElementById('periodSelect').value = data.period;
            if (data.ssid) { wifiSsid = data.ssid; document.getElementById('wifiSsid').value = data.ssid; }
            if (data.pass) { wifiPass = data.pass; document.getElementById('wifiPass').value = data.pass; }
            renderTables();
            populatePinDropdown();
            updateExamplePayload();
        }

        // Генерация примера отправляемых данных (телеметрия) — новый формат
        function updateExamplePayload() {
            const sensorList = [];
            
            // Каналы: relay/button → id = имя или "Dx", value = 1/0
            channels.filter(c => c.type !== 'sensor').forEach(c => {
                const id = pinNames[c.pin] || 'D' + c.pin;
                sensorList.push({ id, value: 0 });
            });
            
            // Температурные датчики (по ROM из availableSensors)
            // Берем из sensorNames (если имя задано) или просто ROM
            availableSensors.forEach(s => {
                const sn = sensorNames.find(n => n.rom === s.rom);
                sensorList.push({ id: sn ? sn.rom : s.rom, value: (parseFloat(s.t) || 0).toFixed(2) });
            });

            const payload = {
                sensors: sensorList,
                rules: rules.map(r => ({
                    id: r.id,
                    enabled: r.enabled ? 1 : 0,
                    triggered: (r.active || false) ? 1 : 0
                }))
            };
            document.getElementById('examplePayload').value = JSON.stringify(payload, null, 2);
        }

        // Отправка тест
        async function testTelemetry() {
            const log = document.getElementById('telemetryLog');
            try {
                const res = await fetch('/api/telemetry/send', { method: 'POST' });
                const data = await res.json();
                const time = new Date().toLocaleTimeString();
                const respText = data.response || '';
                const short = respText.length <= 140;
                const escaped = respText.replace(/</g,'&lt;').replace(/>/g,'&gt;');
                const respHtml = short
                    ? '<span style="font-size:11px;color:#666;display:block;">' + escaped + '</span>'
                    : '<details style="margin-top:3px;"><summary style="cursor:pointer;font-size:11px;">Подробнее</summary><pre style="background:#e9ecef;padding:6px;border-radius:4px;font-size:11px;margin:4px 0 0 0;white-space:pre-wrap;word-break:break-all;">' + escaped + '</pre></details>';
                if (data.status === 'ok') {
                    log.innerHTML = '<div style="color:#28a745;margin-bottom:6px;">[' + time + '] HTTP ' + data.httpCode + ' — OK' + respHtml + '</div>' + log.innerHTML;
                    showNotification('Телеметрия отправлена успешно');
                } else {
                    const errMsg = data.error || 'HTTP ' + (data.httpCode || '') + (data.response ? ': ' + data.response : '');
                    log.innerHTML = '<div style="color:#dc3545;margin-bottom:6px;">[' + time + '] HTTP ' + (data.httpCode || '—') + ' — ' + errMsg + respHtml + '</div>' + log.innerHTML;
                    showNotification('Ошибка: ' + errMsg);
                }
            } catch(e) {
                const time = new Date().toLocaleTimeString();
                log.innerHTML = '<div style="color:#dc3545;">[' + time + '] Ошибка соединения: ' + e.message + '</div>' + log.innerHTML;
                showNotification('Ошибка соединения: ' + e.message);
            }
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
            populatePinDropdown();
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
            populatePinDropdown();
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
                silentSave();
                showNotification('Канал обновлён. Для применения изменений нужна перезагрузка.', true);
                return;
            }
            
            if (channels.some(c => c.pin === pin)) { alert('Пин занят!'); return; }
            channels.push({ type, pin, name });
            renderTables();
            silentSave();
            showNotification('Канал «' + name + '» добавлен. Для инициализации пина нужна перезагрузка.', true);
        }

        function removeChannel(i) { if (confirm('Удалить канал?')) { channels.splice(i, 1); renderTables(); silentSave(); showNotification('Канал удалён. Для применения нужна перезагрузка.', true); } }
        
        function toggleRule(i) {
            rules[i].enabled = !rules[i].enabled;
            renderTables();
            silentSave();
        }
        
        function removeRule(i) { if (confirm('Удалить правило?')) { rules.splice(i, 1); renderTables(); silentSave(); showNotification('Правило удалено'); } }

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
                    showNotification('Правило обновлено');
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
                    showNotification('Правило обновлено');
                    return;
                }
                
                const id = Math.random().toString(16).substr(2, 8);
                rules.push({ id, name, enabled, condType: type, combine, sensors: [], sourcePins, actuators: acts });
            }
            
            renderTables();
            silentSave();
            resetRuleForm();
            showNotification('Правило «' + name + '» сохранено');
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
                period: parseInt(document.getElementById('periodSelect').value) || 0,
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

        // --- Сохранение настроек Wi-Fi (с подтверждением) ---
        async function saveWifiSettings() {
            if (!confirm('Сохранение настроек сети вызовет перезагрузку устройства. Продолжить?')) return;
            const config = {
                channels, rules, sensorNames,
                deviceName: document.getElementById('deviceName').value.trim(),
                serverUrl: document.getElementById('serverUrl').value.trim(),
                period: parseInt(document.getElementById('periodSelect').value) || 0,
                ssid: document.getElementById('wifiSsid').value.trim(),
                pass: document.getElementById('wifiPass').value
            };
            try {
                const res = await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(config)
                });
                if (res.ok) showNotification('Настройки сети сохранены. Перезагрузка...', true);
                else alert('Ошибка: ' + await res.text());
            } catch (e) { console.error(e); }
        }

        // --- Сохранение настроек телеметрии (без подтверждения) ---
        async function saveTelemetrySettings() {
            const config = {
                channels, rules, sensorNames,
                deviceName: document.getElementById('deviceName').value.trim(),
                serverUrl: document.getElementById('serverUrl').value.trim(),
                period: parseInt(document.getElementById('periodSelect').value) || 0,
                ssid: document.getElementById('wifiSsid').value.trim(),
                pass: document.getElementById('wifiPass').value
            };
            try {
                const res = await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(config)
                });
                if (res.ok) showNotification('Настройки телеметрии сохранены');
                else alert('Ошибка: ' + await res.text());
            } catch (e) { console.error(e); }
        }

        // --- Обратный отсчёт до перезагрузки (общий таймер) ---
        function showRebootCountdown(msg) {
            const container = document.getElementById('toastContainer');
            container.innerHTML = '';

            const toast = document.createElement('div');
            toast.className = 'toast warning';
            toast.innerHTML = '<span class="toast-msg">' + msg + ' <b id="rebootCountdown">15</b> сек</span>';
            container.appendChild(toast);

            let countdown = 15;
            const interval = setInterval(() => {
                countdown--;
                const el = document.getElementById('rebootCountdown');
                if (el) el.textContent = countdown;
                if (countdown <= 0) {
                    clearInterval(interval);
                    location.reload();
                }
            }, 1000);
        }

        // --- Перезагрузка устройства с таймером ---
        async function rebootDevice() {
            try {
                await fetch('/api/restart', { method: 'POST' });
            } catch (e) { /* Игнорируем — сервер уже перезагружается */ }
            showRebootCountdown('⏳ Перезагрузка...');
        }

        // --- Обновление прошивки файлом ---
        async function uploadFirmware() {
            const input = document.getElementById('firmwareFileInput');
            const file = input.files && input.files[0];
            if (!file) { alert('Выберите файл прошивки (.bin)!'); return; }

            const sizeMB = (file.size / 1048576).toFixed(2);
            if (!confirm('Обновить прошивку файлом «' + file.name + '» (' + sizeMB + ' МБ)?\nУстройство будет перезагружено. Продолжить?')) return;

            const btn = document.getElementById('fwUploadBtn');
            const status = document.getElementById('fwStatus');
            btn.disabled = true;
            status.className = 'fw-status uploading';
            status.textContent = '⏳ Загрузка... не закрывайте страницу и не отключайте питание';

            const fd = new FormData();
            fd.append('firmware', file, file.name);

            try {
                const res = await fetch('/update', { method: 'POST', body: fd });
                const bodyText = await res.text();

                // ESP8266HTTPUpdateServer отвечает 200 даже при ошибке — инспектируем текст ответа
                if (bodyText.toLowerCase().indexOf('error') !== -1) {
                    status.className = 'fw-status error';
                    status.textContent = '❌ Ошибка обновления: ' + bodyText;
                    btn.disabled = false;
                    return;
                }

                // Успех: показан "Update Success! Rebooting..."
                status.className = 'fw-status ok';
                status.textContent = '✅ Прошивка установлена!';
                showRebootCountdown('✅ Прошивка установлена! Перезагрузка...');
            } catch (e) {
                // Соединение оборвано — ESP закрыл коннект и начал перезагрузку (успех)
                // или сетевая ошибка при загрузке
                if (e instanceof TypeError) {
                    status.className = 'fw-status ok';
                    status.textContent = '✅ Прошивка загружена, устройство перезагружается...';
                    showRebootCountdown('✅ Прошивка загружена! Перезагрузка...');
                } else {
                    status.className = 'fw-status error';
                    status.textContent = '❌ Ошибка сети: ' + e.message;
                    btn.disabled = false;
                }
            }
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
                        period: config.period || 0,
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

                // Версия прошивки (вкладка «Прошивка»)
                if (data.fw) document.getElementById('fwVersion').value = data.fw;

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
            setInterval(updateStatus, 4000);
        };
    </script>
</body>
</html>
)rawliteral";
