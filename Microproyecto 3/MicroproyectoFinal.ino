#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ---------------------------
// CONFIGURACIÓN DHT22
// ---------------------------
#define DHTPIN 27
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------------------------
// CONFIG RTC DS1307
// ---------------------------
RTC_DS1307 rtc;

// ---------------------------
// SENSOR DE LUZ (HW-486)
// ---------------------------
#define LIGHT_SENSOR_PIN 34
int lightValue = 0;
bool isDayMode = true;
#define LIGHT_THRESHOLD 2000

// ---------------------------
// SENSOR MQ135 (CALIDAD DE AIRE)
// ---------------------------
#define MQ135_PIN 35  // Pin analógico para MQ135
int airQualityValue = 0;
String airQualityStatus = "Buena";

// ---------------------------
// Umbrales para calidad del aire
// ---------------------------
#define AQ_EXCELLENT 50
#define AQ_GOOD 100
#define AQ_MODERATE 200
#define AQ_POOR 300
// Más de 300 = Muy Mala

// ---------------------------
// CONFIG WI-FI
// ---------------------------
const char* ssid = "A33";
const char* password = "27092312";
WebServer server(80);

// ---------------------------
// OLED SH1106
// ---------------------------
#define OLED_RESET -1
Adafruit_SH1106G display(128, 64, &Wire, OLED_RESET);

// ---------------------------
// VARIABLES GLOBALES TEMP/HUM
// ---------------------------
float temp = 0, hum = 0;
float tempMax = -1000, tempMin = 1000;
float humMax  = -1000, humMin  = 1000;

// ---------------------------
// VARIABLE DE PAUSA DEL SISTEMA
// ---------------------------
bool systemPaused = false;

// ---------------------------
// VARIABLES PARA RTC CON RESGUARDO
// ---------------------------
unsigned long lastRtcMillis = 0;
unsigned long lastUnixTs = 0;

// =======================================
//               HTML PAGE
// =======================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Estación Climática</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  
  body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    text-align: center;
    min-height: 100vh;
    transition: background 1s ease, color 0.5s ease;
    position: relative;
    overflow-x: hidden;
  }

  /* MODO DÍA */
  body.day-mode {
    background: linear-gradient(to bottom, #87CEEB 0%, #E0F6FF 50%, #FFE5B4 100%);
    color: #333;
  }

  /* MODO NOCHE */
  body.night-mode {
    background: linear-gradient(to bottom, #0a1128 0%, #1a1f3a 50%, #2d1b4e 100%);
    color: #e0e0e0;
  }

  /* ELEMENTOS DECORATIVOS */
  #decorative-elements {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    pointer-events: none;
    z-index: 1;
  }

  /* SOL */
  .sun {
    position: absolute;
    top: 50px;
    right: 80px;
    width: 100px;
    height: 100px;
    background: radial-gradient(circle, #FFD700 0%, #FFA500 100%);
    border-radius: 50%;
    box-shadow: 0 0 60px #FFD700;
    animation: pulse 3s ease-in-out infinite;
    opacity: 0;
    transition: opacity 1s ease;
  }

  body.day-mode .sun { opacity: 1; }

  @keyframes pulse {
    0%, 100% { transform: scale(1); }
    50% { transform: scale(1.1); }
  }

  /* NUBES */
  .cloud {
    position: absolute;
    background: rgba(255, 255, 255, 0.8);
    border-radius: 100px;
    opacity: 0;
    transition: opacity 1s ease;
  }

  body.day-mode .cloud { opacity: 1; }

  .cloud::before, .cloud::after {
    content: '';
    position: absolute;
    background: rgba(255, 255, 255, 0.8);
    border-radius: 100px;
  }

  .cloud1 {
    width: 100px;
    height: 40px;
    top: 100px;
    left: 10%;
    animation: float 20s linear infinite;
  }

  .cloud1::before {
    width: 50px;
    height: 50px;
    top: -25px;
    left: 10px;
  }

  .cloud1::after {
    width: 60px;
    height: 40px;
    top: -15px;
    right: 10px;
  }

  .cloud2 {
    width: 120px;
    height: 50px;
    top: 150px;
    right: 15%;
    animation: float 25s linear infinite reverse;
  }

  .cloud2::before {
    width: 60px;
    height: 60px;
    top: -30px;
    left: 20px;
  }

  .cloud2::after {
    width: 70px;
    height: 50px;
    top: -20px;
    right: 15px;
  }

  @keyframes float {
    0% { transform: translateX(0); }
    100% { transform: translateX(100vw); }
  }

  /* LUNA */
  .moon {
    position: absolute;
    top: 50px;
    right: 80px;
    width: 100px;
    height: 100px;
    background: #F4F4F4;
    border-radius: 50%;
    box-shadow: 0 0 40px rgba(244, 244, 244, 0.6);
    opacity: 0;
    transition: opacity 1s ease;
  }

  body.night-mode .moon { opacity: 1; }

  .moon::before {
    content: '';
    position: absolute;
    top: 10px;
    left: 15px;
    width: 20px;
    height: 20px;
    background: #d0d0d0;
    border-radius: 50%;
  }

  .moon::after {
    content: '';
    position: absolute;
    bottom: 20px;
    right: 20px;
    width: 15px;
    height: 15px;
    background: #d0d0d0;
    border-radius: 50%;
  }

  /* ESTRELLAS */
  .star {
    position: absolute;
    background: white;
    border-radius: 50%;
    opacity: 0;
    transition: opacity 1s ease;
    animation: twinkle 2s ease-in-out infinite;
  }

  body.night-mode .star { opacity: 1; }

  @keyframes twinkle {
    0%, 100% { opacity: 0.3; transform: scale(1); }
    50% { opacity: 1; transform: scale(1.2); }
  }

  /* CONTENIDO PRINCIPAL */
  #main-content {
    position: relative;
    z-index: 2;
    padding: 20px;
  }

  h2 {
    font-size: 2.5rem;
    margin: 20px 0;
    text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
  }

  /* BOTÓN DE PAUSA/REANUDACIÓN */
  #pauseButton {
    font-size: 1.5rem;
    padding: 15px 40px;
    margin: 20px 0;
    border: none;
    border-radius: 50px;
    font-weight: bold;
    cursor: pointer;
    transition: all 0.3s ease;
    box-shadow: 0 4px 15px rgba(0,0,0,0.2);
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
  }

  #pauseButton:hover {
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(0,0,0,0.3);
  }

  #pauseButton:active {
    transform: translateY(0);
  }

  #pauseButton.paused {
    background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
  }

  /* NOTIFICACIÓN DE PAUSA */
  #pauseNotification {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    background: rgba(255, 87, 87, 0.95);
    color: white;
    padding: 30px 50px;
    border-radius: 20px;
    font-size: 2rem;
    font-weight: bold;
    z-index: 1000;
    box-shadow: 0 10px 40px rgba(0,0,0,0.5);
    display: none;
    backdrop-filter: blur(10px);
    animation: pulse-notification 2s ease-in-out infinite;
  }

  @keyframes pulse-notification {
    0%, 100% { transform: translate(-50%, -50%) scale(1); }
    50% { transform: translate(-50%, -50%) scale(1.05); }
  }

  #pauseNotification.show {
    display: block;
  }

  /* INDICADOR DE LUZ */
  #light-indicator {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    margin: 15px 0;
    padding: 10px 20px;
    background: rgba(255,255,255,0.2);
    border-radius: 25px;
    backdrop-filter: blur(10px);
  }

  body.night-mode #light-indicator {
    background: rgba(0,0,0,0.3);
  }

  .light-icon {
    width: 30px;
    height: 30px;
    font-size: 24px;
  }

  #light-value {
    font-size: 1.2rem;
    font-weight: bold;
  }

  /* RELOJ */
  #clock {
    font-size: 4rem;
    padding: 15px 30px;
    border-radius: 20px;
    display: inline-block;
    margin: 20px 0;
    font-weight: bold;
    background: rgba(255,255,255,0.15);
    backdrop-filter: blur(10px);
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
    transition: all 0.5s ease;
  }

  body.day-mode #clock {
    color: #2c3e50;
    border: 3px solid rgba(255,255,255,0.5);
  }

  body.night-mode #clock {
    color: #00ff00;
    background: rgba(0,0,0,0.5);
    border: 3px solid rgba(0,255,0,0.3);
    text-shadow: 0 0 20px rgba(0,255,0,0.5);
  }

  #date {
    font-size: 1.8rem;
    margin: 10px 0 30px 0;
    font-weight: 500;
  }

  /* CONTENEDOR DE SENSORES */
  .container {
    display: flex;
    justify-content: center;
    gap: 50px;
    margin: 40px auto;
    flex-wrap: wrap;
    max-width: 1400px;
  }

  .sensor-card {
    background: rgba(255,255,255,0.25);
    backdrop-filter: blur(10px);
    padding: 30px;
    border-radius: 25px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
    transition: all 0.5s ease;
    min-width: 280px;
  }

  body.night-mode .sensor-card {
    background: rgba(255,255,255,0.08);
    border: 1px solid rgba(255,255,255,0.1);
  }

  .sensor-card:hover {
    transform: translateY(-10px);
    box-shadow: 0 12px 40px rgba(0,0,0,0.2);
  }

  .thermo-block {
    display: flex;
    flex-direction: column;
    align-items: center;
  }

  .bar-wrapper {
    display: flex;
    flex-direction: row;
    align-items: flex-end;
    gap: 15px;
    margin: 20px 0;
  }

  .bar-container {
    width: 70px;
    height: 280px;
    background: linear-gradient(to top, #0000ff, #00ffff, #00ff00, #ffff00, #ff0000);
    border-radius: 35px;
    border: 3px solid rgba(255,255,255,0.3);
    overflow: hidden;
    position: relative;
    box-shadow: inset 0 0 20px rgba(0,0,0,0.2);
  }

  body.night-mode .bar-container {
    border-color: rgba(255,255,255,0.2);
    box-shadow: inset 0 0 20px rgba(0,0,0,0.5), 0 0 20px rgba(255,255,255,0.1);
  }

  .bar-mask {
    position: absolute;
    top: 0;
    width: 100%;
    background: rgba(0,0,0,0.6);
    transition: height 0.8s ease;
  }

  body.night-mode .bar-mask {
    background: rgba(0,0,0,0.8);
  }

  .scale {
    height: 280px;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    font-size: 1.1rem;
    font-weight: 600;
  }

  .value {
    font-size: 2.5rem;
    margin-top: 15px;
    font-weight: bold;
  }

  body.day-mode .value {
    color: #2c3e50;
  }

  body.night-mode .value {
    color: #00ff00;
    text-shadow: 0 0 10px rgba(0,255,0,0.5);
  }

  .label {
    font-size: 1.5rem;
    font-weight: bold;
    margin-top: 15px;
    text-transform: uppercase;
    letter-spacing: 2px;
  }

  /* CALIDAD DEL AIRE */
  .air-quality-card {
    background: rgba(255,255,255,0.25);
    backdrop-filter: blur(10px);
    padding: 30px;
    border-radius: 25px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
    transition: all 0.5s ease;
    min-width: 320px;
    max-width: 400px;
  }

  body.night-mode .air-quality-card {
    background: rgba(255,255,255,0.08);
    border: 1px solid rgba(255,255,255,0.1);
  }

  .air-quality-card:hover {
    transform: translateY(-10px);
    box-shadow: 0 12px 40px rgba(0,0,0,0.2);
  }

  .air-icon {
    font-size: 4rem;
    margin: 20px 0;
  }

  .air-status {
    font-size: 2rem;
    font-weight: bold;
    margin: 15px 0;
    padding: 10px 20px;
    border-radius: 15px;
    display: inline-block;
  }

  .status-excellent {
    background: linear-gradient(135deg, #00c853, #64dd17);
    color: white;
  }

  .status-good {
    background: linear-gradient(135deg, #76ff03, #b2ff59);
    color: #1b5e20;
  }

  .status-moderate {
    background: linear-gradient(135deg, #ffd600, #ffea00);
    color: #f57f17;
  }

  .status-poor {
    background: linear-gradient(135deg, #ff6f00, #ff9100);
    color: white;
  }

  .status-verypoor {
    background: linear-gradient(135deg, #d50000, #ff1744);
    color: white;
  }

  .air-ppm {
    font-size: 1.8rem;
    margin-top: 10px;
    opacity: 0.9;
  }

  /* RESPONSIVE */
  @media (max-width: 768px) {
    h2 { font-size: 2rem; }
    #clock { font-size: 3rem; }
    #date { font-size: 1.4rem; }
    .container { gap: 30px; }
    .sensor-card { min-width: 240px; padding: 20px; }
    .value { font-size: 2rem; }
    .air-quality-card { min-width: 280px; }
  }
</style>
</head>
<body class="day-mode">

<div id="decorative-elements">
  <!-- SOL -->
  <div class="sun"></div>
  
  <!-- NUBES -->
  <div class="cloud cloud1"></div>
  <div class="cloud cloud2"></div>
  
  <!-- LUNA -->
  <div class="moon"></div>
  
  <!-- ESTRELLAS (generadas por JS) -->
</div>

<div id="main-content">
  <h2>🌤️ Estación Climática 🌙</h2>

  <button id="pauseButton" onclick="togglePause()">⏸️ Pausar Sistema</button>

  <div id="pauseNotification">
    ⏸️ SISTEMA PAUSADO ⏸️
  </div>

  <div id="light-indicator">
    <span class="light-icon">💡</span>
    <span id="light-value">-- lux</span>
  </div>

  <div id="clock">--:--:--</div>
  <div id="date">----/--/--</div>

  <div class="container">
    <!-- TEMPERATURA -->
    <div class="sensor-card">
      <div class="thermo-block">
        <div class="bar-wrapper">
          <div class="bar-container">
            <div id="maskTemp" class="bar-mask" style="height:280px;"></div>
          </div>
          <div class="scale">
            <span>50°</span><span>25°</span><span>0°</span><span>-20°</span>
          </div>
        </div>
        <div class="label">🌡️ Temperatura</div>
        <div id="tempVal" class="value">-- °C</div>
      </div>
    </div>

    <!-- HUMEDAD -->
    <div class="sensor-card">
      <div class="thermo-block">
        <div class="bar-wrapper">
          <div class="bar-container">
            <div id="maskHum" class="bar-mask" style="height:280px;"></div>
          </div>
          <div class="scale">
            <span>100</span><span>75</span><span>50</span><span>0</span>
          </div>
        </div>
        <div class="label">💧 Humedad</div>
        <div id="humVal" class="value">-- %</div>
      </div>
    </div>

    <!-- CALIDAD DEL AIRE -->
    <div class="air-quality-card">
      <div class="label">🌬️ Calidad del Aire</div>
      <div class="air-icon" id="airIcon">💨</div>
      <div id="airStatus" class="air-status status-good">Buena</div>
      <div class="air-ppm" id="airPPM">-- PPM</div>
    </div>
  </div>
</div>

<script>
let clientTime = null;
let syncIntervalMs = 30000;
let lastSync = 0;
let tickTimer = null;
let currentMode = 'day';
let isPaused = false;
let updateInterval = null;

// Generar estrellas aleatorias
function generateStars() {
  const container = document.getElementById('decorative-elements');
  for (let i = 0; i < 50; i++) {
    const star = document.createElement('div');
    star.className = 'star';
    star.style.width = Math.random() * 3 + 1 + 'px';
    star.style.height = star.style.width;
    star.style.left = Math.random() * 100 + '%';
    star.style.top = Math.random() * 70 + '%';
    star.style.animationDelay = Math.random() * 2 + 's';
    container.appendChild(star);
  }
}

// Función para pausar/reanudar el sistema
function togglePause() {
  isPaused = !isPaused;
  
  fetch("/pause", {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify({paused: isPaused})
  })
  .then(res => res.json())
  .then(data => {
    console.log("Estado de pausa actualizado:", data);
    updatePauseUI();
  })
  .catch(e => console.error("Error al cambiar estado de pausa:", e));
}

// Actualizar interfaz según estado de pausa
function updatePauseUI() {
  const btn = document.getElementById("pauseButton");
  const notification = document.getElementById("pauseNotification");
  
  if (isPaused) {
    btn.innerText = "▶️ Reanudar Sistema";
    btn.classList.add("paused");
    notification.classList.add("show");
    
    // Detener actualizaciones
    if (updateInterval) {
      clearInterval(updateInterval);
      updateInterval = null;
    }
    if (tickTimer) {
      clearInterval(tickTimer);
      tickTimer = null;
    }
  } else {
    btn.innerText = "⏸️ Pausar Sistema";
    btn.classList.remove("paused");
    notification.classList.remove("show");
    
    // Reanudar actualizaciones
    updateInterval = setInterval(updateSensors, 1500);
    syncTimeOnce();
  }
}

// Actualizar calidad del aire
function updateAirQuality(value, status) {
  const statusEl = document.getElementById("airStatus");
  const iconEl = document.getElementById("airIcon");
  const ppmEl = document.getElementById("airPPM");
  
  statusEl.innerText = status;
  ppmEl.innerText = value + " PPM";
  
  // Remover todas las clases de estado
  statusEl.className = "air-status";
  
  // Agregar clase según el estado
  if (status === "Excelente") {
    statusEl.classList.add("status-excellent");
    iconEl.innerText = "✨";
  } else if (status === "Buena") {
    statusEl.classList.add("status-good");
    iconEl.innerText = "😊";
  } else if (status === "Moderada") {
    statusEl.classList.add("status-moderate");
    iconEl.innerText = "😐";
  } else if (status === "Mala") {
    statusEl.classList.add("status-poor");
    iconEl.innerText = "😷";
  } else if (status === "Muy Mala") {
    statusEl.classList.add("status-verypoor");
    iconEl.innerText = "☠️";
  }
}

// Actualizar sensores
function updateSensors() {
  if (isPaused) return;
  
  fetch("/sensors")
    .then(res => res.json())
    .then(data => {
      const t = Number(data.temp);
      const h = Number(data.hum);
      const l = Number(data.light);
      const aq = Number(data.airQuality);
      const aqStatus = data.airStatus || "Buena";
      const mode = data.mode || 'day';
      
      document.getElementById("tempVal").innerText = isFinite(t) ? t.toFixed(2) + " °C" : "-- °C";
      document.getElementById("humVal").innerText = isFinite(h) ? h.toFixed(2) + " %" : "-- %";
      document.getElementById("light-value").innerText = isFinite(l) ? l + " lux" : "-- lux";
      
      if (isFinite(aq)) {
        updateAirQuality(aq, aqStatus);
      }

      let tVal = isFinite(t) ? t : 0;
      let hVal = isFinite(h) ? h : 0;

      let ht = Math.max(0, Math.min(50, tVal));
      let hh = Math.max(0, Math.min(100, hVal));
      document.getElementById("maskTemp").style.height = (280 - (ht * 5.6)) + "px";
      document.getElementById("maskHum").style.height = (280 - (hh * 2.8)) + "px";

      // Cambiar modo día/noche
      if (mode !== currentMode) {
        currentMode = mode;
        document.body.className = mode + '-mode';
      }
    })
    .catch(e => console.log("error sensors:", e));
}

function syncTimeOnce() {
  if (isPaused) return;
  
  fetch("/time")
    .then(res => res.json())
    .then(t => {
      clientTime = new Date();
      clientTime.setFullYear(t.year, t.month - 1, t.day);
      clientTime.setHours(t.hour, t.minute, t.second, 0);
      
      lastSync = Date.now();
      if (!tickTimer && !isPaused) {
        tickTimer = setInterval(tickClientClock, 1000);
      }
      displayClientTime();
    })
    .catch(e => console.log("error time sync:", e));
}

function tickClientClock() {
  if (!clientTime || isPaused) return;
  clientTime = new Date(clientTime.getTime() + 1000);
  displayClientTime();
  if (Date.now() - lastSync > syncIntervalMs) {
    syncTimeOnce();
  }
}

function displayClientTime() {
  if (!clientTime) return;
  const pad = (n) => String(n).padStart(2, '0');
  const hh = pad(clientTime.getHours());
  const mm = pad(clientTime.getMinutes());
  const ss = pad(clientTime.getSeconds());
  document.getElementById("clock").innerText = hh + ":" + mm + ":" + ss;

  const days = ["Domingo","Lunes","Martes","Miércoles","Jueves","Viernes","Sábado"];
  const dow = days[clientTime.getDay()];
  const dd = pad(clientTime.getDate());
  const mo = pad(clientTime.getMonth() + 1);
  const yy = clientTime.getFullYear();
  document.getElementById("date").innerText = dow + " " + dd + "/" + mo + "/" + yy;
}

window.onload = function() {
  generateStars();
  updateSensors();
  syncTimeOnce();
  updateInterval = setInterval(updateSensors, 1500);
};
</script>

</body>
</html>
)rawliteral";
// ==============================

// =========================================================
//  FUNCIÓN PARA EVALUAR CALIDAD DEL AIRE
// =========================================================
void updateAirQuality() {
    airQualityValue = analogRead(MQ135_PIN);
    
    // Mapear valor analógico a PPM (0-4095 -> 0-500 PPM aprox)
    // Nota: Estos valores son aproximados, el MQ135 requiere calibración
    int ppm = map(airQualityValue, 0, 4095, 0, 500);
    airQualityValue = ppm;
    
    if (ppm < AQ_EXCELLENT) {
        airQualityStatus = "Excelente";
    } else if (ppm < AQ_GOOD) {
        airQualityStatus = "Buena";
    } else if (ppm < AQ_MODERATE) {
        airQualityStatus = "Moderada";
    } else if (ppm < AQ_POOR) {
        airQualityStatus = "Mala";
    } else {
        airQualityStatus = "Muy Mala";
    }
}

// =========================================================
//  PAUSA/REANUDA EL SISTEMA
// =========================================================
void handlePause() {
    if (server.method() == HTTP_POST) {
        String body = server.arg("plain");
        
        // Parsear JSON simple manualmente
        int pausedIndex = body.indexOf("\"paused\":");
        if (pausedIndex != -1) {
            String value = body.substring(pausedIndex + 9);
            value.trim();
            systemPaused = (value.indexOf("true") != -1);
            
            Serial.print("Sistema ");
            Serial.println(systemPaused ? "PAUSADO" : "REANUDADO");
            
            char json[100];
            snprintf(json, sizeof(json),
                     "{\"success\":true,\"paused\":%s}",
                     systemPaused ? "true" : "false");
            
            server.send(200, "application/json", json);
        } else {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    } else {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    }
}

// =========================================================
//  RETORNA TEMP, HUM, LUZ Y AIRE
// =========================================================
void handleSensors() {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    lightValue = analogRead(LIGHT_SENSOR_PIN);
    isDayMode = (lightValue < LIGHT_THRESHOLD);
    
    updateAirQuality();

    if (!isnan(temp)) {
        if (temp > tempMax) tempMax = temp;
        if (temp < tempMin) tempMin = temp;
    }
    if (!isnan(hum)) {
        if (hum > humMax) humMax = hum;
        if (hum < humMin) humMin = hum;
    }

    char json[400];
    snprintf(json, sizeof(json),
             "{\"temp\":%.2f,\"hum\":%.2f,"
             "\"tempMax\":%.2f,\"tempMin\":%.2f,"
             "\"humMax\":%.2f,\"humMin\":%.2f,"
             "\"light\":%d,\"mode\":\"%s\","
             "\"airQuality\":%d,\"airStatus\":\"%s\"}",
             temp, hum, tempMax, tempMin, humMax, humMin,
             lightValue, isDayMode ? "day" : "night",
             airQualityValue, airQualityStatus.c_str());

    server.send(200, "application/json", json);
}

// =========================================================
//  RETORNA HORA Y FECHA
// =========================================================
void handleTime() {
    if (!rtc.isrunning()) {
        server.send(500, "application/json", "{\"error\":\"RTC not running\"}");
        return;
    }
    
    DateTime now = rtc.now();
    
    char json[200];
    snprintf(json, sizeof(json),
             "{\"hour\":%d,\"minute\":%d,\"second\":%d,"
             "\"day\":%d,\"month\":%d,\"year\":%d,"
             "\"weekday\":%d}",
             now.hour(), now.minute(), now.second(),
             now.day(), now.month(), now.year(),
             now.dayOfTheWeek());

    server.send(200, "application/json", json);
}

// =========================================================
//    OLED — ALTERNA ENTRE Reloj / Temperatura / Humedad / Aire
// =========================================================
unsigned long lastOLEDUpdate = 0;
int oledState = 0;   // 0=hora, 1=temp, 2=hum, 3=aire

void updateOLED() {
    if (millis() - lastOLEDUpdate < 3000) return;
    lastOLEDUpdate = millis();

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    // Si el sistema está en pausa, mostrar mensaje
    if (systemPaused) {
        display.setTextSize(2);
        display.setCursor(5, 10);
        display.println("SISTEMA");
        display.setCursor(10, 35);
        display.println("PAUSADO");
        display.display();
        return;
    }

    DateTime now = rtc.now();

    switch (oledState) {
        case 0:  // RELOJ
            display.setTextSize(2);
            display.setCursor(0,0);
            display.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
            display.setTextSize(1);
            display.setCursor(0,40);
            display.printf("%02d/%02d/%04d", now.day(), now.month(), now.year());
            break;

        case 1:  // TEMPERATURA
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("TEMP");
            display.setTextSize(3);
            display.setCursor(0,25);
            display.printf("%.1fC", temp);
            break;

        case 2:  // HUMEDAD
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("HUM");
            display.setTextSize(3);
            display.setCursor(0,25);
            display.printf("%.1f%%", hum);
            break;

        case 3:  // CALIDAD DEL AIRE
            display.setTextSize(1);
            display.setCursor(0,0);
            display.println("CALIDAD AIRE");
            display.setTextSize(2);
            display.setCursor(0,20);
            display.println(airQualityStatus);
            display.setTextSize(2);
            display.setCursor(0,45);
            display.printf("%d PPM", airQualityValue);
            break;
    }

    display.display();

    oledState = (oledState + 1) % 4;  // Ahora son 4 estados
}

// =========================================================
//                        SETUP
// =========================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Iniciando...");

    dht.begin();
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    pinMode(MQ135_PIN, INPUT);  // Configurar pin del MQ135

    Wire.begin(21, 22);

    display.begin(0x3C, true);
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Estacion");
    display.println("Climatica");
    display.println("Iniciando...");
    display.display();
    delay(2000);

    if (!rtc.begin()) {
        Serial.println("RTC no encontrado!");
        Serial.println("Verifica las conexiones I2C (SDA=21, SCL=22)");
    } else {
        Serial.println("RTC inicializado correctamente");
    }

    if (!rtc.isrunning()) {
        Serial.println("RTC no está corriendo, ajustando fecha/hora...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("Fecha/hora ajustada");
    } else {
        Serial.println("RTC está corriendo");
        DateTime now = rtc.now();
        Serial.print("Hora actual del RTC: ");
        Serial.print(now.year());
        Serial.print("/");
        Serial.print(now.month());
        Serial.print("/");
        Serial.print(now.day());
        Serial.print(" ");
        Serial.print(now.hour());
        Serial.print(":");
        Serial.print(now.minute());
        Serial.print(":");
        Serial.println(now.second());
    }

    Serial.print("Conectando a WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 12000) {
        delay(300);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        display.println("WiFi OK!");
        display.print("IP:");
        display.println(WiFi.localIP());
        display.display();
        delay(3000);
    } else {
        Serial.println("\nWiFi NO conectado");
    }

    server.on("/", [](){ server.send(200, "text/html", index_html); });
    server.on("/sensors", handleSensors);
    server.on("/time", handleTime);
    server.on("/pause", HTTP_POST, handlePause);

    server.begin();
    Serial.println("Servidor iniciado.");
    Serial.println("Sensores configurados:");
    Serial.println("- DHT22 en pin 27");
    Serial.println("- Sensor de luz en pin 34");
    Serial.println("- MQ135 en pin 35");
}

// =========================================================
//                        LOOP
// =========================================================
void loop() {
    server.handleClient();
    updateOLED();
}
