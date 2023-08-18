#include <Arduino.h> // lib Arduino (opcional)
#include <Adafruit_GFX.h>    // Biblioteca de gráficos
#include <Adafruit_ST7735.h> // Biblioteca do hardware ST7735
#include <SPI.h> // Biblioteca para comunicação SPI
#include <Fonts/FreeSerif9pt7b.h> // Fonte Serif que é usada no display
#include <SimpleDHT.h> // lib do DHT
#include "FS_File_Record.h"  // Nossa lib personalizada do SPIFFS

// Pinos do display
#define TFT_DC 12 // A0
#define TFT_CS 13 // CS
#define TFT_MOSI 14 // SDA
#define TFT_CLK 27 // SCK                
#define TFT_RST 0  // RESET
#define TFT_MISO 0 // MISO

// Pino do botão de exclusão de arquivo
const int buttonPin = 34;

// Tamanho dos registros de temperatura e umidade (13 caracteres), exemplo:
// 100.00;100.00
// temp;umid
const int sizeOfRecord = 13;

//100.00
// Variáveis usadas em 'formatValue'
const int integralPartSize = 3, decimalPartSize = 2;

// Objeto da nossa lib que recebe o nome do arquivo e tamanho fixo de registro
FS_File_Record ObjFS("/dht22.bin", sizeOfRecord);

// Pino do DHT22
const int pinDHT22 = 32;

// Objeto do dht22
SimpleDHT22 dht22(pinDHT22);

// Objeto do display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

// Altura da fonte, usada na função resetDisplay
int fontHeight = 7;

// Variáveis usada para contagem/comparação de tempo (millis) na função timeout
long millisRefShowSpace;
bool flagShowSpace = false;

// String que recebe as mensagens de erro
String errorMsg;

// Variável que guarda o último registro obtido
String lastRecord = "";

// Existem duas tasks compartilhando o mesmo objeto do display
// Essas flags são usadas para evitar que as duas acessam este objeto ao mesmo tempo
bool flagDisplayisBusy = false, eventButton = false;

// Limpa o display e posiciona o cursor no início
void resetDisplay()
{
  // Verificamos se o display não está ocupado por alguma das tasks
  if(!flagDisplayisBusy)
  {
    flagDisplayisBusy = true;  
    tft.setFont(&FreeSerif9pt7b);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(0,fontHeight+5);
    tft.setTextSize(1);
    flagDisplayisBusy = false; 
  }
}

void showDisplay(String msg)
{
  // Verificamos se o display não está ocupado por alguma das tasks
  if(!flagDisplayisBusy)
  {    
    flagDisplayisBusy = true;  // Sinalizamos que o display está ocupado
    tft.println(msg);          // Exibimos no display a mensagem
    flagDisplayisBusy = false; // Sinalizamos que o display está desocupado
  }   
}

// Posiciona cursor no centro e exibe a mensagem "FILE DELETED" em amarelo
void showFileDeleted(bool sucess)
{
  // Posição y aonde o texto será exibido
  int y = (tft.height()/2)-(fontHeight*2);

  // Limpamos o display (aqui não usamos a função resetDisplay porque a flag displayisBusy neste momento está como true e o resetDisplay não será executado)
  // Portanto limpamos o display executando os comandos separadamente
  tft.setFont(&FreeSerif9pt7b);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  
  // Define cor do texto amarela
  tft.setTextColor(ST7735_YELLOW);
  // Posiciona no centro de eixo y
  tft.setCursor(0, y);    

  // Se foi possível excluir
  if(sucess)
  {
    // Exibe mensagem "FILE DELETED"
    tft.println("   FILE");    
    tft.println("   DELETED!");
  }
  else // Se não foi possível excluir
  {
    // Exibe mensagem "CANNOT DELETE"
    tft.println("   CANNOT");
    tft.println("   DELETE");
  }

  // Define cor do texto branca
  tft.setTextColor(ST7735_WHITE);
}

// Exibimos no display o arquivo, pausando a exibição a cada 6 registros
void showFile()
{
  int count = 0;
  String linha = "";  
  
  // Exibe na serial e no display o início do arquivo
  Serial.println("# Begin of file #");  
  showDisplay("# Begin of file #");

  errorMsg = "";

  // Posiciona o ponteiro do arquivo no início
  ObjFS.rewind();

  // Exibe todos os registros até o fim
  while(ObjFS.readFileNextRecord(&linha, &errorMsg) && linha != "")
  {
    Serial.println(linha);
    count++;

    // A cada 6 registros, pausamos e resetamos o display
    if(count % 6 == 0)
    {
      //Exibe "..." sinalizando que ainda não é o fim do arquivo
      showDisplay("...");
      // Aguarda 1.5s para poder visualizar os valores
      delay(1500);
      // Limpa display
      resetDisplay();
    }
    showDisplay(linha);
  }
    
  // Se existir mensagem de erro exibe na serial e no display
  if(errorMsg != "")
  {
    Serial.println(errorMsg);
    showDisplay(errorMsg);
  }

  // Exibe na serial e no display o fim do arquivo
  Serial.println("# End of file #");   
  showDisplay("# End of file #");
  delay(1500);
}

void setup() 
{
  // Inicializamos o display
  tft.initR(INITR_BLACKTAB);

  // Iniciamos a Serial com velocidade de 115200
  Serial.begin(115200);
  
  // Seta botão como entrada (INPUT)
  pinMode(buttonPin, INPUT);  

  // Exibe na serial "Starting..." para debug
  Serial.print("Starting...");

  // Se não foi possível iniciar o File System, exibimos erro e reiniciamos o ESP
  if(!ObjFS.init())
  {
    Serial.println("File system error");
    delay(1000);
    ESP.restart();
  }
  
  // Exibimos mensagem
  Serial.println("File system ok");
  
  // Se o arquivo não existe, criamos o arquivo
  if(!ObjFS.fileExists())
  {
     Serial.println("New file");
     ObjFS.newFile(); // Cria o arquivo
  }  

  // Iniciamos uma task que irá ler o botão de exclusão
 xTaskCreatePinnedToCore(
  buttonEvent,   //Função que será executada
  "buttonEvent", //Nome da tarefa
  10000,     //Tamanho da pilha
  &tft,      //Parâmetro da tarefa (no caso não usamos)
  2,         //Prioridade da tarefa
  NULL,      //Caso queira manter uma referência para a tarefa que vai ser criada (no caso não precisamos)
  0);        //Número do core que será executada a tarefa (usamos o core 0 para o loop ficar livre com o core 1)
  
  
  resetDisplay();

  // EXEMPLO DE BUSCA (FIND)
  // Obs: O primeiro registro se posiciona na pos 0 (zero) 
  // String reg = ObjFS.findRecord(10);
  // showDisplay(reg);  

  // Exibimos o arquivo
  showFile();   
}

// Formata valores colocando zero à esquerda, ex: 023.20;047.40 
String formatValue(float value)
{
  char auxDecimalPart[decimalPartSize+1], integralPart[integralPartSize+1], buf[sizeOfRecord+1]; //+1 para o '\0'
  String decimalPart;

  char format[5];

  // Copiamos para a formatação da parte decimal o texto: %0.2f
  strcpy(format, "%.");
  strcat(format, String(decimalPartSize).c_str());
  strcat(format, "f");
 
  sprintf (auxDecimalPart, format, value-(int)value);

  // Atribuímos para a variável decimalPart a parte decimal considerando os zeros à direita 
  decimalPart = String(auxDecimalPart);
  decimalPart.replace("0.","");  

  // Copiamos para a formatação da parte inteira o texto: %03d
  strcpy(format, "%0");
  strcat(format, String(integralPartSize).c_str());
  strcat(format, "d");

  // Atribuímos para a variável integralPart a parte inteira considerando os zeros à esquerda  
  sprintf (integralPart, format, (int)value);

  // Concatenamos a parte interia com a parte decimal separando com ponto
  sprintf (buf, "%s.%s", integralPart, decimalPart.c_str());

  return String(buf);
}

// Função que lê e formata os dados de temperatura e umidade
String readDHTValues()
{
  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;

  // Lê sensor
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) 
  {
    Serial.print("Read DHT22 failed, err="); 
    Serial.println(err);
    delay(1000);
    return "-------------"; //importante: deve possuir o mesmo sizeOfRecord, ou seja, 13 caracteres
  }

  // Retorna valores formatados, separados por ponto-virgula
  return formatValue(temperature)+";"+formatValue(humidity);
}

//Função que compara se o tempo foi atingido, sem travar o loop
bool timeout(const int DELAY, long *millisAnterior, bool *flag)
{
  if(*flag)
  {
    *millisAnterior = millis();
    *flag = false;
  }  

  if((*millisAnterior + DELAY) < millis())
  {
    *flag = true;
    return true;
  }

  return false;
}

// Exibe o espaço total, usado e disponível no display
void showAvailableSpace()
{
  long prevMillis;
  if(!eventButton && timeout(10000, &millisRefShowSpace, &flagShowSpace))
  {
    Serial.println("Space: "+String(ObjFS.getTotalSpace())+" Bytes");
    Serial.println("Used: "+String(ObjFS.getUsedSpace())+" Bytes");
    resetDisplay();
    showDisplay("Total space:");
    showDisplay(String(ObjFS.getTotalSpace())+" Bytes");
    tft.setTextColor(ST7735_YELLOW);
    showDisplay("Used space:");
    showDisplay(String(ObjFS.getUsedSpace())+" Bytes");
    tft.setTextColor(ST77XX_GREEN);
    showDisplay("Free space:");
    showDisplay(String(ObjFS.getTotalSpace()-ObjFS.getUsedSpace())+" Bytes");

    // Registramos 4 vezes enquanto a mensagem aparece no display
    for(int i=0; i<4; i++) 
    {
      // Lê temperatura e umidade do DHT
      String values = readDHTValues();

      // Escrevemos no arquivo e exibimos erro ou sucesso na serial para debug
      if(values != "" &&!ObjFS.writeFile(values, &errorMsg))
        Serial.println(errorMsg);
      else
        Serial.println("Write ok");

      prevMillis = millis();

      while(prevMillis+2500 > millis() && !eventButton);

      if(eventButton)     
      {
        eventButton = false;
        // aguarda exibição "FILE DELETED"
        delay(500);
        break;
      } 
    } //fim do for
  } //fim do if(timeout)
} //fim da função

// Exibe última amostra de temperatura e umidade obtida
void showLastRecord()
{
  resetDisplay();
  showDisplay("Last record:");
  showDisplay(lastRecord);
}

void loop() 
{  
  // Se não houver memória disponível, exibe e reinicia o ESP
  if(!ObjFS.availableSpace())
  {
    Serial.println("Memory is full!");
    showDisplay("Memory is full!");
    delay(10000);
    return;
  }

  // Lê temperatura e umidade do DHT
  String values = readDHTValues();

  // Escrevemos no arquivo e exibimos erro ou sucesso na serial para debug
  if(values != "" && !ObjFS.writeFile(values, &errorMsg))
    Serial.println(errorMsg);
  else
    Serial.println("Write ok");

  // Atribui para a variável global a última amostra
  lastRecord = values;
  // Exibe a última amostra no display
  showLastRecord();
  // Exibimos o espaço total, usado e disponível no display, de tempo em tempo
  showAvailableSpace();
  
  // Aguarda 2500ms
  delay(2500);
}

// Função executada pela task de evento do botão
void buttonEvent(void* display)
{   
  TickType_t taskDelay = 10 / portTICK_PERIOD_MS;

  // IMPORTANTE: A tarefa não pode terminar, deve ficar presa em um loop infinito
  while(true)
  {
    // Se o botão foi pressionado e a flag estiver "false"
    if(digitalRead(buttonPin) == HIGH && !eventButton)
    {    
      // Sinalizamos que o botão foi pressionado
      eventButton = true;
      // Tenta excluir o arquivo
      if(ObjFS.destroyFile())     
      {
        Serial.println("File deleted");

        // Enquanto o display estiver ocupado, aguardamos
        while(flagDisplayisBusy)
          vTaskDelay(taskDelay);

        // Sinalizamos que o display está ocupado
        flagDisplayisBusy = true;
        // Exibimos no display
        showFileDeleted(true);
        // Sinalizamos que o display está desocupado
        flagDisplayisBusy = false;

        lastRecord = "";
      }
      else
      {
        Serial.println("Failed to delete file");
        
        while(flagDisplayisBusy)
          vTaskDelay(taskDelay);

        // Sinalizamos que o display está ocupado
        flagDisplayisBusy = true;
        // Exibimos no display
        showFileDeleted(false);
        // Sinalizamos que o display está desocupado
        flagDisplayisBusy = false;
      }
    }
    else
    if(digitalRead(buttonPin) == LOW && eventButton) // Se o botão foi solto e a flag estiver "true"
    { 
      // Sinalizamos que o botão foi solto
      eventButton = false;
    }  
    
    // Executamos um delay de 10ms, os delays executado nas xTasks são diferentes
    vTaskDelay(taskDelay);

    //IMPORTANTE: SEMPRE DEIXAR UM DELAY PARA ALIMENTAR WATCHDOG
  }
}






