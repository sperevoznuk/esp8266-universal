#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP8266 Universal</title>
    <link rel="stylesheet" href="/style.css">
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
            <div class="row" style="margin-top:15px;">
                <button id="rulesAddBtn" onclick="openRuleForm()" style="background:#28a745;width:auto;padding:10px 15px;">➕ Добавить правило</button>
            </div>
            <div id="ruleForm" class="form-box" style="display:none;margin-top:10px;">
                <div class="row" style="margin:0 0 10px 0;align-items:center;justify-content:space-between;">
                    <h3 id="ruleFormTitle" style="margin:0;font-size:15px;color:#333;">Добавить правило</h3>
                    <button onclick="closeRuleForm()" style="background:#6c757d;width:auto;padding:8px 12px;margin:0;">✕ Отмена</button>
                </div>
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
                        <details>
                            <summary style="cursor:pointer;font-size:14px;">Пример данных, которые будут отправлены:</summary>
                            <pre id="examplePayload" style="background:#f4f6f8;border:1px solid #ddd;border-radius:4px;padding:8px;font-size:12px;white-space:pre-wrap;word-break:break-all;margin:8px 0 0 0;"></pre>
                        </details>
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

    <script src="/app.js"></script>
</body>
</html>
)rawliteral";