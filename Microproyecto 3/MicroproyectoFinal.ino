// =======================================
//            LIBRERÍAS
// =======================================
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>

// =======================================
//            CONFIG WiFi
// =======================================
const char* ssid = "A33";
const char* password = "27092312";

// =======================================
//            CONFIG DHT22
// =======================================
#define DHTPIN 27
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// =======================================
//            CONFIG RTC DS1307
// =======================================
RTC_DS1307 rtc;

// =======================================
//         CONFIG SENSOR LUZ HW-486
// =======================================
#define LIGHT_SENSOR_PIN 34  // Pin analógico (ADC) para el sensor de luz
int lightValue = 0;
bool isDayMode = true;
#define LIGHT_THRESHOLD 2000 // Umbral para detectar día/noche (ajustar según necesidad)
// NOTA: El HW-486 da valores BAJOS con luz y ALTOS sin luz (lógica invertida)

// =======================================
//            SERVIDOR WEB
// =======================================
WebServer server(80);

// =======================================
//        VARIABLES GLOBAL TEMP/HUM
// =======================================
float temp = 0, hum = 0;
float tempMax = -1000, tempMin = 1000;
float humMax  = -1000, humMin  = 1000;

// =======================================
//       VARIABLES PARA RTC/RESGUARDO
// =======================================
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
    max-width: 1200px;
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

  /* RESPONSIVE */
  @media (max-width: 768px) {
    h2 { font-size: 2rem; }
    #clock { font-size: 3rem; }
    #date { font-size: 1.4rem; }
    .container { gap: 30px; }
    .sensor-card { min-width: 240px; padding: 20px; }
    .value { font-size: 2rem; }
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
  </div>
</div>

<script>
let clientTime = null;
let syncIntervalMs = 30000;
let lastSync = 0;
let tickTimer = null;
let currentMode = 'day';

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

// Actualizar sensores
function updateSensors() {
  fetch("/sensors")
    .then(res => res.json())
    .then(data => {
      const t = Number(data.temp);
      const h = Number(data.hum);
      const l = Number(data.light);
      const mode = data.mode || 'day';
      
      document.getElementById("tempVal").innerText = isFinite(t) ? t.toFixed(2) + " °C" : "-- °C";
      document.getElementById("humVal").innerText = isFinite(h) ? h.toFixed(2) + " %" : "-- %";
      document.getElementById("light-value").innerText = isFinite(l) ? l + " lux" : "-- lux";

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
  fetch("/time")
    .then(res => res.json())
    .then(t => {
      clientTime = new Date(t.year, t.month - 1, t.day, t.hour, t.minute, t.second);
      lastSync = Date.now();
      if (!tickTimer) {
        tickTimer = setInterval(tickClientClock, 1000);
      }
      displayClientTime();
    })
    .catch(e => console.log("error time sync:", e));
}

function tickClientClock() {
  if (!clientTime) return;
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
  setInterval(updateSensors, 1500);
};
</script>

</body>
</html>
)rawliteral";

// =======================================
//           MANEJO ENDPOINT /SENSORS
// =======================================
void handleSensors() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  
  // Leer sensor de luz
  lightValue = analogRead(LIGHT_SENSOR_PIN);
  
  // Determinar modo día/noche (INVERTIDO: valores BAJOS = luz presente = día)
  isDayMode = (lightValue < LIGHT_THRESHOLD);

  if (!isnan(temp)) {
    if (temp > tempMax) tempMax = temp;
    if (temp < tempMin) tempMin = temp;
  }

  if (!isnan(hum)) {
    if (hum > humMax) humMax = hum;
    if (hum < humMin) humMin = hum;
  }

  char json[300];
  snprintf(json, sizeof(json),
           "{\"temp\":%.2f,\"hum\":%.2f,"
           "\"tempMax\":%.2f,\"tempMin\":%.2f,"
           "\"humMax\":%.2f,\"humMin\":%.2f,"
           "\"light\":%d,\"mode\":\"%s\"}",
           temp, hum, tempMax, tempMin, humMax, humMin,
           lightValue, isDayMode ? "day" : "night");

  server.send(200, "application/json", json);
}

// =======================================
//           MANEJO ENDPOINT /TIME
// =======================================
void handleTime() {
  DateTime now;
  now = rtc.now();

  unsigned long nowUnix = (unsigned long) now.unixtime();
  unsigned long m = millis();

  if (lastUnixTs == 0) {
    lastUnixTs = nowUnix;
    lastRtcMillis = m;
  }

  if (nowUnix == lastUnixTs) {
    if ((m - lastRtcMillis) > 1500) {
      lastUnixTs += 1;
      lastRtcMillis = m;
    }
  } else {
    lastUnixTs = nowUnix;
    lastRtcMillis = m;
  }

  DateTime effective = DateTime((time_t) lastUnixTs);

  char json[200];
  snprintf(json, sizeof(json),
           "{\"hour\":%02d,\"minute\":%02d,\"second\":%02d,"
           "\"day\":%02d,\"month\":%02d,\"year\":%04d,"
           "\"weekday\":%d}",
           effective.hour(), effective.minute(), effective.second(),
           effective.day(), effective.month(), effective.year(),
           effective.dayOfTheWeek());

  server.send(200, "application/json", json);
}

// =======================================
//                SETUP
// =======================================
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configurar pin del sensor de luz como entrada
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  Wire.begin(21, 22);

  if (!rtc.begin()) {
    Serial.println("No se detecta RTC");
  } else {
    Serial.println("RTC detectado");
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC no estaba en marcha, configurando hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - start > 10000) {
      Serial.println("\nTimeout conexión WiFi (10s). Continua sin conexión.");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No conectado a WiFi.");
  }

  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.on("/sensors", handleSensors);
  server.on("/time", handleTime);

  server.begin();
  Serial.println("Servidor iniciado.");
}

// =======================================
//                 LOOP
// =======================================
void loop() {
  server.handleClient();
}
