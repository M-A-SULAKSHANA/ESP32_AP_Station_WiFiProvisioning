#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <vector>

const char* mqtt_server = "broker.hivemq.com"; // Use your broker IP if local
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/test";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer webServer(80);

AsyncMqttClient mqttClient;

struct WifiNetwork {
    String ssid;
    int rssi;
};

std::vector<WifiNetwork> scanNetworks() {
    std::vector<WifiNetwork> networks;
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0) {
        Serial.println("No networks found");
        return networks;
    } else {
        for (int i = 0; i < numNetworks; ++i) {
            WifiNetwork network;
            network.ssid = WiFi.SSID(i);
            network.rssi = WiFi.RSSI(i);
            networks.push_back(network);
        }
        return networks;
    }
}

void handleRoot() {
    String html = R"=====(
    <!DOCTYPE html>
    <html>
    <head><title>Wi-Fi Setup</title></head>
    <body>
      <h1>Connect to Wi-Fi</h1>
      <form method="POST" action="/connect">
        <label for="ssid">Select Wi-Fi Network:</label>
        <select name="ssid" id="ssid">
    )=====";

    std::vector<WifiNetwork> networks = scanNetworks();
    if (networks.empty()) {
        html += "<option value=\"\">No networks found</option>";
    } else {
        for (const auto& network : networks) {
            html += "<option value=\"" + network.ssid + "\">" + network.ssid + " (RSSI: " + network.rssi + ")</option>";
        }
    }

    html += R"=====(
        </select>
        <label for="password">Password:</label>
        <input type="password" name="password" id="password">
        <input type="submit" value="Connect">
      </form>
    </body>
    </html>
    )=====";

    webServer.send(200, "text/html", html);
}

void handleConnect() {
    if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
        String ssid = webServer.arg("ssid");
        String password = webServer.arg("password");

        WiFi.begin(ssid.c_str(), password.c_str());
        webServer.send(200, "text/plain", "Connecting to " + ssid + "...");

        int timeout = 10000;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(500);
            timeout -= 500;
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to " + ssid);
            Serial.println("IP address: " + WiFi.localIP().toString());
            webServer.sendHeader("Location", "http://" + WiFi.localIP().toString());
            webServer.send(302, "text/plain", "");
        } else {
            Serial.println("\nConnection failed!");
            webServer.send(200, "text/plain", "Connection failed!");
        }
    } else {
        webServer.send(400, "text/plain", "Invalid request");
    }
}

void handleNotFound() {
    webServer.sendHeader("Location", "http://" + apIP.toString(), true);
    webServer.send(302, "text/plain", "");
}

void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT Broker");
    mqttClient.publish(mqtt_topic, 0, false, "ESP32 Connected");
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP32-Setup");

    dnsServer.start(DNS_PORT, "*", apIP);

    webServer.on("/", handleRoot);
    webServer.on("/connect", HTTP_POST, handleConnect);
    webServer.onNotFound(handleNotFound);
    webServer.begin();

    Serial.println("HTTP server started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    mqttClient.onConnect(onMqttConnect);
    mqttClient.setServer(mqtt_server, mqtt_port);
}

void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();

    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 5000) {
        lastCheck = millis();

        if (WiFi.status() == WL_CONNECTED) {
            if (!mqttClient.connected()) {
                mqttClient.connect();
            } else {
                static int counter = 0;
                char message[50];
                snprintf(message, sizeof(message), "Message #%d", counter++);
                Serial.print("Publishing: ");
                Serial.println(message);
                mqttClient.publish(mqtt_topic, 0, false, message);
            }
        }
    }
}
