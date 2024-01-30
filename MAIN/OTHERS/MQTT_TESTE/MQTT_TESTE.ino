#include <WiFi.h>
#include <PubSubClient.h>

// Substitua pelos detalhes da sua rede WiFi
const char* ssid = "NOS-2E40";
const char* password = "2TJA5RZ9";

// Substitua pelos detalhes do seu broker MQTT
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttUser = "IPB";
const char* mqttPassword = "noiottracker";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const int interval = 5000;  // Intervalo de tempo entre mensagens em milissegundos

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  Serial.print("Conteúdo: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("IPB/TESTE/TRACKER/05", mqttUser, mqttPassword)) {
      Serial.println("Conectado ao broker MQTT");
      client.subscribe("seu-topico");  // Substitua pelo seu tópico MQTT
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    char msg[50];
    snprintf(msg, 50, "Mensagem enviada a cada 5 segundos: %lu", now / 1000);
    Serial.print("Publicando mensagem: ");
    Serial.println(msg);
    client.publish("seu-topico", msg);  // Substitua pelo seu tópico MQTT
  }
}
