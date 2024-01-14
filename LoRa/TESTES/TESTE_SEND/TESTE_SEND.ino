#include <HardwareSerial.h>
#define rxLORA 25
#define txLORA 26

HardwareSerial lora(1); //n das portas Uart do esp
char end_to_send = '1';   // endereço do lora que vai recever esse pacote
void setup()
{
  //===================================
  //para ligar o transsitor:
  pinMode(32,OUTPUT);
  digitalWrite(32,HIGH);
  Serial.begin(115200);
  //lora.begin(115200);
  lora.begin(115200, SERIAL_8N1, rxLORA, txLORA);
  //lora.setTimeout(1000);
  delay(1500);
  //================================
  //definir endereço:
  Serial.print("Atual ADDRESS:");
  lora.println("AT+ADDRESS?");
  Serial.println(lora.readString());
  Serial.print("DEFINIR ID:");
  lora.println("AT+ADDRESS=1,M");
  Serial.println(lora.readString());
  delay(1000);
  Serial.print("Atual ADDRESS:");
  lora.println("AT+ADDRESS?");
  Serial.println(lora.readString());
  //================OBS=================
  //para salvae as configurações no módulo, digite ",M" depois da definição, por
  // exemplo -> AT+BAND=868000000,M 
  //           "AT+ADDRESS=1.M"
  // Por outro lado se quiser upar essas informações via software, pode ser usado
  // o seguinte trecho em setup():

  //---------------------------------------------------
  //lora.println("AT+BAND=868000000"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //lora.println("AT+ADDRESS=1"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //lora.println("AT+NETWORKID=18"); // AT+SEND=1,2,LO
  //Serial.println(lora.readString());
  //delay(1000);
  //---------------------------------------------------
}

void loop()
{        
    /*
    double lat = -41.123456;
    double lon = 6.123456;
    int vel = 2;
    float temp = 42.22;
    float hum = 53.44;
    float x = -2.02;
    float y = 4.04;
    float z = 8.41;
    float bat = 80;

    char mensagem[120];
    char data[70];
    
    sprintf(data, "A%.6fB%.6fC%iD%.2fE%.2fF%.2fG%.2fH%3.2fI%.0fJ",lat, lon, vel, temp, hum, x, y, z, bat); //atribui e organiza as informações em data
    //o caractere J indica o fim da mensagem
    int requiredBufferSize = snprintf(NULL, 0, "%s",data); //calcula tamanho string
    sprintf(mensagem, "AT+SEND=%c,%i,%s",end_to_send,requiredBufferSize,data); // junta as informações em "mensagem"
    lora.println(mensagem); //manda a mensagem montada para o módulo
    Serial.println(mensagem); //imprime no monitor a mensagem montada 
    Serial.println(lora.readString()); //lê a resposta do módulo
    */
    lora.println("AT+SEND=2,62,A41.796808-6.757620B1C26.25D57.08E0.04F10.28G-1.49H1I12142215K"); //manda a mensagem montada para o módulo
    Serial.println(lora.readString()); //lê a resposta do módulo
    Serial.println("PACOTE ENVIADO");
    delay(2000);

}

