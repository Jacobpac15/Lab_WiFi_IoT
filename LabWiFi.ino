#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences preferences;
WebServer server(80);

String ssid;
String password;

bool isConfigured = false;

// ----------- HTML -----------
const char* form_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Config WiFi</title>
</head>
<body>
  <h2>Configurar WiFi</h2>
  <form action="/save" method="POST">
    SSID:<br>
    <input type="text" name="ssid"><br>
    Password:<br>
    <input type="password" name="password"><br><br>
    <input type="submit" value="Guardar">
  </form>
</body>
</html>
)rawliteral";

// ----------- HANDLERS -----------

void handleRoot() {
  server.send(200, "text/html", form_html);
}

void handleSave() {
  ssid = server.arg("ssid");
  password = server.arg("password");

  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", password);
  preferences.end();

  server.send(200, "text/html", "<h3>Guardado. Reiniciando...</h3>");
  delay(2000);
  ESP.restart();
}

void handleReset() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();

  server.send(200, "text/plain", "Configuracion borrada. Reiniciando...");
  delay(2000);
  ESP.restart();
}

// ----------- WIFI -----------

void startAP() {
  WiFi.softAP("ESP32_Config");
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP iniciado");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reset", handleReset);

  server.begin();
}

void connectWiFi() {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  preferences.end();

  if (ssid == "") {
    Serial.println("No hay credenciales");
    startAP();
    return;
  }

  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("Conectando");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.println(WiFi.localIP());

    server.on("/reset", handleReset);
    server.begin();

  } else {
    Serial.println("\nFallo. Iniciando AP...");
    startAP();
  }
}

// ----------- SETUP -----------

void setup() {
  Serial.begin(115200);
  connectWiFi();
}

void loop() {
  server.handleClient();
}