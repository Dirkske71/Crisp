#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <Preferences.h>

// --- CONFIGURATIE ---
const char* AP_SSID = "Willy_Pro_Master";
const char* supabase_url = "https://rrphcsrfdkknupiitrxk.supabase.co/rest/v1/willy_pro_devices";
const char* supabase_key = "sb_publishable_e0UFQ9EKRAC1_k3ZnI0ztg_i5JLT3av";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  preferences.begin("willy-pro", true);
  String stored_ssid = preferences.getString("ssid", "");
  String stored_pass = preferences.getString("pass", "");
  preferences.end();

  if (stored_ssid != "") {
    WiFi.begin(stored_ssid.c_str(), stored_pass.c_str());
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 20) {
      delay(500);
      Serial.print(".");
      counter++;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[STATUS] Online!");
  } else {
    setupAP();
  }
}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    dnsServer.processNextRequest();
  }
  server.handleClient();
}

void setupAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleRoot); 
  server.begin();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="nl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Willy Pro Onboarding</title>
    <style>
        :root { --willy-blue: #007BFF; --bg: #0a0a0a; --card: #161616; --input: #242424; --text: #ffffff; --label: #999999; --border: #333333; }
        body { font-family: 'Segoe UI', sans-serif; background-color: var(--bg); color: var(--text); margin: 0; padding: 20px; display: flex; justify-content: center; }
        .container { width: 100%; max-width: 450px; background: var(--card); padding: 40px 30px; border-radius: 24px; box-shadow: 0 20px 50px rgba(0,0,0,0.6); }
        .header { text-align: center; margin-bottom: 30px; }
        h2 { margin: 0; font-size: 26px; font-weight: 700; }
        .brand-sub { color: var(--willy-blue); font-weight: 600; font-size: 14px; text-transform: uppercase; margin-top: 5px; letter-spacing: 1px; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-size: 13px; color: var(--label); font-weight: 500; }
        input, select { width: 100%; padding: 12px; background: var(--input); border: 1px solid var(--border); border-radius: 10px; color: #fff; font-size: 15px; box-sizing: border-box; }
        .btn-activeer { background: var(--willy-blue); color: white; border: none; padding: 18px; width: 100%; border-radius: 14px; font-size: 18px; font-weight: 700; cursor: pointer; margin-top: 20px; }
        .section-title { font-size: 16px; color: var(--willy-blue); margin: 20px 0 10px 0; border-bottom: 1px solid var(--border); padding-bottom: 5px; }
    </style>
</head>
<body>
<div class="container">
    <div class="header">
        <h2>Willy Pro</h2>
        <div class="brand-sub">Tap & Connect Setup</div>
    </div>
    <form action="/save" method="POST">
        <div class="section-title">Project & Locatie</div>
        <div class="form-group"><label>Projectnaam</label><input type="text" name="project_naam" required></div>
        <div class="form-group"><label>Plaats</label><input type="text" name="plaats" required></div>
        
        <div class="section-title">Contactgegevens</div>
        <div class="form-group"><label>E-mail Eigenaar (Klant)</label><input type="email" name="email_eigenaar" required></div>
        <div class="form-group"><label>E-mail Beheerder/Installateur</label><input type="email" name="email_beheerder" required></div>
        
        <div class="section-title">Technische Gegevens</div>
        <div class="form-group"><label>Merk</label><input type="text" name="merk" required></div>
        <div class="form-group"><label>Model Buitenunit</label><input type="text" name="model_buitenunit" required></div>
        <div class="form-group"><label>Serienummer Buitenunit</label><input type="text" name="serienummer_buitenunit" required></div>
        
        <div class="section-title">WiFi Verbinding</div>
        <div class="form-group"><label>WiFi SSID</label><input type="text" name="wifi_ssid" required></div>
        <div class="form-group"><label>WiFi Wachtwoord</label><input type="password" name="wifi_pw" required></div>
        
        <button type="submit" class="btn-activeer">🚀 Activeer Willy Pro</button>
    </form>
</div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleSave() {
  String qsid = server.arg("wifi_ssid");
  String qpass = server.arg("wifi_pw");

  preferences.begin("willy-pro", false);
  preferences.putString("ssid", qsid);
  preferences.putString("pass", qpass);
  preferences.end();

  server.send(200, "text/html", "<h1>Instellingen opgeslagen</h1><p>De Willy verbindt nu met de database...</p>");
  
  WiFi.begin(qsid.c_str(), qpass.c_str());
  int check = 0;
  while (WiFi.status() != WL_CONNECTED && check < 20) { delay(500); check++; }

  if (WiFi.status() == WL_CONNECTED) {
    sendDataToSupabase();
  }
  delay(2000);
  ESP.restart();
}

void sendDataToSupabase() {
  HTTPClient http;
  http.begin(supabase_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", supabase_key);
  http.addHeader("Authorization", String("Bearer ") + supabase_key);

  String json = "{";
  json += "\"project_naam\":\"" + server.arg("project_naam") + "\",";
  json += "\"plaats\":\"" + server.arg("plaats") + "\",";
  json += "\"email_eigenaar\":\"" + server.arg("email_eigenaar") + "\",";
  json += "\"email_beheerder\":\"" + server.arg("email_beheerder") + "\",";
  json += "\"merk\":\"" + server.arg("merk") + "\",";
  json += "\"model_buitenunit\":\"" + server.arg("model_buitenunit") + "\",";
  json += "\"serienummer_buitenunit\":\"" + server.arg("serienummer_buitenunit") + "\",";
  json += "\"wifi_ssid\":\"" + server.arg("wifi_ssid") + "\",";
  json += "\"wifi_pw\":\"" + server.arg("wifi_pw") + "\",";
  json += "\"mac_address\":\"" + WiFi.macAddress() + "\"";
  json += "}";

  Serial.println("Verzonden JSON:");
  Serial.println(json);

  int httpResponseCode = http.POST(json);
  Serial.print("Supabase Response: "); Serial.println(httpResponseCode);
  
  if (httpResponseCode != 201) {
    Serial.println("Foutmelding:");
    Serial.println(http.getString());
  }
  
  http.end();
}