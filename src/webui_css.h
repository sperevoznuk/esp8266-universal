#pragma once

const char WEBUI_CSS[] PROGMEM = R"rawliteral(
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
)rawliteral";