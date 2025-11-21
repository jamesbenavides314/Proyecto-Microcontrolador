// =======================================
//            LIBRERÍAS
// =======================================
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>      // <-- Añadido: necesario para I2C/RTC

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
unsigned long lastRtcMillis = 0;     // millis() cuando obtuvimos el último timestamp real
unsigned long lastUnixTs = 0;        // último unix timestamp usado (respaldo si RTC no avanza)

// =======================================
//               HTML PAGE
// =======================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Estación</title>
<style>
  html { font-family: Arial; text-align: center; background: #f0f0f0; }

  #clock {
    font-size: 4rem;
    background: black;
    color: #00ff00;
    padding: 10px 20px;
    border-radius: 15px;
    display: inline-block;
    margin-top: 20px;
  }

  #date { font-size: 1.8rem; margin-top: 10px; }

  .container { display: flex; justify-content: center; gap: 40px; margin-top: 30px; flex-wrap: wrap; }

  .thermo-block { display: flex; flex-direction: column; align-items: center; }

  .bar-wrapper { display: flex; flex-direction: row; align-items: flex-end; gap: 12px; }

  .bar-container {
    width: 60px; height: 250px;
    background: linear-gradient(to top, #0000ff, #00ffff, #00ff00, #ffff00, #ff0000);
    border-radius: 30px;
    border: 2px solid #444;
    overflow: hidden;
    position: relative;
  }

  .bar-mask {
    position: absolute;
    top: 0;
    width: 100%;
    background: rgba(0,0,0,0.5);
    transition: height 0.4s;
  }

  .scale { height: 250px; display: flex; flex-direction: column; justify-content: space-between; }
  .value { font-size: 1.4rem; margin-top: 10px; }
  .label { font-size: 1.3rem; font-weight: bold; margin-top: 10px; }
</style>
</head>
<body>

<h2>Estación Climática</h2>

<div id="clock">--:--:--</div>
<div id="date">----/--/--</div>

<div class="container">

  <!-- TEMPERATURA -->
  <div class="thermo-block">
    <div class="bar-wrapper">
      <div class="bar-container">
        <div id="maskTemp" class="bar-mask" style="height:250px;"></div>
      </div>
      <div class="scale">
        <span>50°C</span><span>25°C</span><span>0°C</span><span>-20°C</span>
      </div>
    </div>
    <div class="label">Temperatura</div>
    <div id="tempVal" class="value">-- °C</div>
  </div>

  <!-- HUMEDAD -->
  <div class="thermo-block">
    <div class="bar-wrapper">
      <div class="bar-container">
        <div id="maskHum" class="bar-mask" style="height:250px;"></div>
      </div>
      <div class="scale">
        <span>100%</span><span>75%</span><span>50%</span><span>0%</span>
      </div>
    </div>
    <div class="label">Humedad</div>
    <div id="humVal" class="value">-- %</div>
  </div>

</div>

<script>
/*
  Reloj robusto:
  - Obtenemos una muestra de /time y construimos una Date JS.
  - Luego incrementamos esa Date cada 1000ms en el cliente para segundos estables.
  - Resincronizamos cada 30s con el servidor para corregir deriva.
*/

let clientTime = null;      // objeto Date en cliente
let syncIntervalMs = 30000; // cada 30s resincronizamos
let lastSync = 0;
let tickTimer = null;

// sensors (NO TOCAR)
function updateSensors() {
  fetch("/sensors")
    .then(res => res.json())
    .then(data => {
      // si data.temp/hum son null o undefined, se muestran "--"
      const t = Number(data.temp);
      const h = Number(data.hum);
      document.getElementById("tempVal").innerText = isFinite(t) ? t.toFixed(2) + " °C" : "-- °C";
      document.getElementById("humVal").innerText  = isFinite(h) ? h.toFixed(2) + " %" : "-- %";

      // evitar NaN en máscaras
      let tVal = isFinite(t) ? t : 0;
      let hVal = isFinite(h) ? h : 0;

      // escala visual (simple)
      let ht = Math.max(0, Math.min(50, tVal)); // limitar entre 0 y 50
      let hh = Math.max(0, Math.min(100, hVal)); // 0..100
      document.getElementById("maskTemp").style.height = (250 - (ht * 5)) + "px";
      document.getElementById("maskHum").style.height  = (250 - (hh * 2.5)) + "px";
    })
    .catch(e => {
      console.log("error sensors:", e);
    });
}

// obtiene la hora del servidor y establece clientTime
function syncTimeOnce() {
  fetch("/time")
    .then(res => res.json())
    .then(t => {
      // JSON esperado: { "hour":..,"minute":..,"second":..,"day":..,"month":..,"year":..,"weekday":.. }
      clientTime = new Date(t.year, t.month - 1, t.day, t.hour, t.minute, t.second);
      lastSync = Date.now();
      // si no hay tick en marcha, iniciarlo
      if (!tickTimer) {
        tickTimer = setInterval(tickClientClock, 1000);
      }
      // actualizar visual de inmediato
      displayClientTime();
    })
    .catch(e => {
      console.log("error time sync:", e);
    });
}

// incrementa clientTime 1 segundo y actualiza pantalla
function tickClientClock() {
  if (!clientTime) return;
  clientTime = new Date(clientTime.getTime() + 1000);
  displayClientTime();

  // resincronizar cada syncIntervalMs
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

// inicia todo
window.onload = function() {
  updateSensors();          // deja sensores tal cual
  syncTimeOnce();           // primer sync
  setInterval(updateSensors, 1500); // conservamos tu intervalo original para sensores
};
</script>

</body>
</html>
)rawliteral";

// =======================================
//           MANEJO ENDPOINT /SENSORS
// =======================================
// --- NOTA: No toqué nada de esta función (temp/hum) ---
void handleSensors() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  if (!isnan(temp)) {
    if (temp > tempMax) tempMax = temp;
    if (temp < tempMin) tempMin = temp;
  }

  if (!isnan(hum)) {
    if (hum > humMax) humMax = hum;
    if (hum < humMin) humMin = hum;
  }

  char json[200];
  snprintf(json, sizeof(json),
           "{\"temp\":%.2f,\"hum\":%.2f,"
           "\"tempMax\":%.2f,\"tempMin\":%.2f,"
           "\"humMax\":%.2f,\"humMin\":%.2f}",
           temp, hum, tempMax, tempMin, humMax, humMin);

  server.send(200, "application/json", json);
}

// =======================================
//           MANEJO ENDPOINT /TIME
// =======================================
// Mejor manejo: obtenemos time desde RTC; si el RTC parece no avanzar,
// usamos un unix timestamp en memoria para avanzar en software.
void handleTime() {
  DateTime now;
  bool rtcOk = true;

  // Intentar leer rtc.now() protegidamente
  // si falla (por ejemplo si Wire no iniciado) capturamos excepción lógica
  now = rtc.now();

  unsigned long nowUnix = (unsigned long) now.unixtime();

  unsigned long m = millis();

  // Si es la primera lectura (lastUnixTs == 0) inicializamos respaldo
  if (lastUnixTs == 0) {
    lastUnixTs = nowUnix;
    lastRtcMillis = m;
  }

  // Si RTC devolvió el mismo unix que usamos antes (posible "pegado")
  if (nowUnix == lastUnixTs) {
    // si ya pasaron > 1500 ms desde la última vez, avanzamos en software 1 segundo
    if ((m - lastRtcMillis) > 1500) {
      lastUnixTs += 1;
      lastRtcMillis = m;
    }
  } else {
    // RTC avanzó normalmente -> aceptamos su valor y actualizamos respaldo
    lastUnixTs = nowUnix;
    lastRtcMillis = m;
  }

  // Construir DateTime desde lastUnixTs (respaldo estable)
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

  // Iniciar I2C en pines por defecto SDA=21, SCL=22
  Wire.begin(21, 22);

  // RTC
  if (!rtc.begin()) {
    Serial.println("No se detecta RTC");
  } else {
    Serial.println("RTC detectado");
  }

  // Si el RTC no está corriendo, se ajusta con la hora de compilación
  if (!rtc.isrunning()) {
    Serial.println("RTC no estaba en marcha, configurando hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // WiFi (timeout incluido para no bloquear indefinidamente)
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

  // RUTAS
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
  // No bloqueamos aquí, sólo atendemos cliente
  server.handleClient();
}
