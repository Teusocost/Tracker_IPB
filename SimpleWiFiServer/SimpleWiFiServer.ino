#include <WiFi.h>

const char* ssid = "NOS-2E40";
const char* password = "2TJA5RZ9";
WiFiServer server(80);
int c;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");
  server.begin();

  Serial.println("Servidor iniciado.");
  Serial.println("IP address: ");
Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><body>");
    client.println("<h1>Dados de Debug</h1>");

    // Leitura dos dados do Serial e envio para o cliente

      while(1){
          if ( millis()>=i){
      c += 1; 
      client.println(c);
      client.println("<br>");
      //client.write('\n'); // Adiciona uma quebra de linha
      i = millis() + 1000;
      }
      }  

    client.println("</body></html>");
    client.stop();
    Serial.println("Cliente desconectado");
  }
}
