 #include "comunicacion.h"

#if USE_WIFI_MQTT

  #include <WiFi.h>
  #include <PubSubClient.h>
  #include "variables.h"
  #include "modos.h"

  // ---- Configura aquí tus credenciales ----
  static const char* WIFI_SSID     = "TU_WIFI";
  static const char* WIFI_PASSWORD = "TU_PASS";
  static const char* MQTT_HOST     = "192.168.1.100";
  static const uint16_t MQTT_PORT  = 1883;
  static const char* MQTT_USER     = "";   // opcional
  static const char* MQTT_PASS     = "";   // opcional
  static const char* MQTT_CLIENTID = "esp32-agua-01";

  // ---- Tópicos (ajústalos a tu Home Assistant / broker) ----
  static const char* T_STATE = "agua/estado";          // JSON con lecturas
  static const char* T_MODE  = "agua/modo";            // publica modo actual
  static const char* T_CMD   = "agua/cmd";             // recibe comandos

  static WiFiClient espClient;
  static PubSubClient mqtt(espClient);

  // --- Helpers ---
  static void wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000) delay(200);
  }

  static void mqttConnect() {
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback([](char* topic, byte* payload, unsigned int length){
      // Payload simple → comando de modo por texto, p.e. "LLENADO_L1"
      String msg;
      for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
      msg.trim();

      // Mapear mensajes a modos (añade lo que necesites)
      if      (msg == "LLENADO_L1") activarModo(LLENADO_L1);
      else if (msg == "LLENADO_L2") activarModo(LLENADO_L2);
      else if (msg == "LLENADO_L3") activarModo(LLENADO_L3);
      else if (msg == "LLENADO_L4") activarModo(LLENADO_L4);
      else if (msg == "EXTRACCION_E1A") activarModo(EXTRACCION_E1A);
      else if (msg == "EXTRACCION_E1B") activarModo(EXTRACCION_E1B);
      else if (msg == "EXTRACCION_E1C") activarModo(EXTRACCION_E1C);
      else if (msg == "EXTRACCION_E2A") activarModo(EXTRACCION_E2A);
      else if (msg == "EXTRACCION_E2B") activarModo(EXTRACCION_E2B);
      else if (msg == "EXTRACCION_E2C") activarModo(EXTRACCION_E2C);
      else if (msg == "EXTRACCION_E3")  activarModo(EXTRACCION_E3);
      else if (msg == "CONSUMO_C1")     activarModo(CONSUMO_C1);
      else if (msg == "CONSUMO_C2")     activarModo(CONSUMO_C2);
      else if (msg == "MODO_NINGUNO")   desactivarModo();
    });

    while (!mqtt.connected()) {
      if (mqtt.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASS)) {
        mqtt.subscribe(T_CMD);
      } else {
        delay(1000);
      }
    }
  }

  static void publishState() {
    // JSON minimalista (añade lo que quieras)
    // Para HA es cómodo publicar en un solo topic un JSON compacto.
    char buf[256];
    // Ojo con snprintf para no desbordar
    snprintf(buf, sizeof(buf),
      "{\"nivelPct\":%.1f,\"volumen\":%.1f,\"p1\":%.2f,\"p2\":%.2f,\"p3\":%.2f,"
      "\"yf1\":%.2f,\"yf2\":%.2f,\"dist\":%.1f,\"bomba\":%s,\"ev1\":%s,\"ev2\":%s,"
      "\"alerta\":%s}",
      nivelAguaPct, volumenAgua, presion1, presion2, presion3,
      flujoYF1, flujoYF2, distancia,
      bombaEncendida ? "true" : "false",
      electrovalvula1 ? "true" : "false",
      electrovalvula2 ? "true" : "false",
      alertaSeguridad ? "true" : "false"
    );
    mqtt.publish(T_STATE, buf, true);

    // Publica modo actual como texto legible
    mqtt.publish(T_MODE, obtenerNombreModo(modoActual), true);
  }

#endif // USE_WIFI_MQTT

void initComunicacion() {
#if USE_WIFI_MQTT
  wifiConnect();
  mqttConnect();
#endif
}

void gestionarComunicacion() {
#if USE_WIFI_MQTT
  if (WiFi.status() != WL_CONNECTED) wifiConnect();
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= 2000) { // publica cada 2s
    publishState();
    last = now;
  }
#endif
}
