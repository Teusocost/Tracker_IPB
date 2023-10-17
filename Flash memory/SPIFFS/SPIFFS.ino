#include "SPIFFS_Utils.h"

SPIFFS_Utils spiffsUtils;

void setup() {
  Serial.begin(115200);
  //------------------------------------
  //spiffsUtils.appendToFile("/dados.txt", "variável teste :D\n"); //grava um novo valor
  //spiffsUtils.appendToFile("/dados.txt", "variável teste 2\n"); //grava um novo valor
  //spiffsUtils.appendToFile("/dados.txt", "variável teste 3\n"); //grava um novo valor
  //float oi = 5;
  //char variavel[100];
  //sprintf(variavel, "Esta é uma string de exemplo para escrita no arquivo %.0f.\n", oi);
  //spiffsUtils.appendToFile("/dados.txt", variavel);
  //------------------------------------
  //String lastValue = spiffsUtils.readLastValue("/dados.txt"); //devolve o ultimo valor gravado
  //Serial.println("Último valor gravado:");
  //Serial.println(lastValue); //imprime o ultimo valor gravado
  //if(lastValue == NULL) Serial.println("não há nada aqui");
  //------------------------------------
  spiffsUtils.listFiles(); //lista todos os dados gravados
  Serial.println("--------------------------------------------------------------");
  spiffsUtils.removeLastValue("/dados.txt"); //remove o primeiro valor gravado
  Serial.println("--------------------------------------------------------------");
  spiffsUtils.listFiles(); //lista todos os dados gravados
  //------------------------------------
  //------------------------------------
  //spiffsUtils.format(); //formatar memoria
  //------------------------------------
  //spiffsUtils.bytes(); //volve a quantidade de bytes usados e disponíveis na memória
}

void loop() {
    // Nada a fazer no loop neste exemplo
}
