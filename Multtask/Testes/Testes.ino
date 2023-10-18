//pinos usados
#include <WiFi.h>

const char* ssid = "NOS-2E40";
const char* password = "2TJA5RZ9";
WiFiServer server(80);
int c;
WiFiClient client;

const uint8_t pin_led = 21;

//variaveis que indicam o núcleo
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne  = 1;



void setup() {
  Serial.begin(115200);
  pinMode(pin_led, OUTPUT);

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

  //cria uma tarefa que será executada na função coreTaskZero, com prioridade 1 e execução no núcleo 0
  //coreTaskZero: piscar LED e contar quantas vezes
  xTaskCreatePinnedToCore(
                    coreTaskZero,   /* função que implementa a tarefa */
                    "coreTaskZero", /* nome da tarefa */
                    10000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,          /* prioridade da tarefa (0 a N) */
                    NULL,       /* referência para a tarefa (pode ser NULL) */
                    taskCoreZero);         /* Núcleo que executará a tarefa */
                    
  delay(500); //tempo para a tarefa iniciar

  //cria uma tarefa que será executada na função coreTaskOne, com prioridade 2 e execução no núcleo 1
  //coreTaskOne: atualizar as informações do display
  xTaskCreatePinnedToCore(
                    coreTaskOne,   /* função que implementa a tarefa */
                    "coreTaskOne", /* nome da tarefa */
                    5000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    2,          /* prioridade da tarefa (0 a N) */
                    NULL,       /* referência para a tarefa (pode ser NULL) */
                    taskCoreOne);         /* Núcleo que executará a tarefa */

    delay(500); //tempo para a tarefa iniciar

   //cria uma tarefa que será executada na função coreTaskTwo, com prioridade 2 e execução no núcleo 0
   //coreTaskTwo: vigiar o botão para detectar quando pressioná-lo
   xTaskCreatePinnedToCore(
                    coreTaskTwo,   /* função que implementa a tarefa */
                    "coreTaskTwo", /* nome da tarefa */
                    5000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    2,          /* prioridade da tarefa (0 a N) */
                    NULL,       /* referência para a tarefa (pode ser NULL) */
                    taskCoreZero);         /* Núcleo que executará a tarefa */
    delay(500); //tempo para a tarefa iniciar
   
}

void loop() {
}

//essa função ficará mudando o estado do led a cada 1 segundo
//e a cada piscada (ciclo acender e apagar) incrementará nossa variável blinked
void coreTaskZero( void * pvParameters ){
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
 
    while(true){
      Serial.println("TESTE");
      vTaskDelay(2000);
    } 
}

//essa função será responsável apenas por atualizar as informações no 
//display a cada 100ms
void coreTaskOne( void * pvParameters ){
  while(true){
    client = server.available();
    digitalWrite(pin_led, LOW);
    if (client) {
      //digitalWrite(pin_led, HIGH);
      digitalWrite(pin_led, LOW);
      Serial.println("Cliente conectado");

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>Dados de Debug</h1>");

      // Leitura dos dados do Serial e envio para o cliente
        int i = 100;
        while(1){
          if (millis()>=i){

            c += 1; 
            client.println(c);
            client.println("<br>");
            i = millis() + 50;
            digitalWrite(pin_led, HIGH);
            vTaskDelay(10);
            digitalWrite(pin_led, LOW);
            if(!client) { 
              break;
            }
          }
        }  

      client.println("</body></html>");
      client.stop();
      Serial.println("Cliente desconectado");
    }
  } 
}

//essa função será responsável por ler o estado do botão
//e atualizar a variavel de controle.
void coreTaskTwo( void * pvParameters ){
     int i=0;
     while(true){
      printf("Incremento %i\n", i++);
      vTaskDelay(100);
    } 
}
