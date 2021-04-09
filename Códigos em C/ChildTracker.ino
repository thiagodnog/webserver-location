//Importar bibliotecas para o LoRa
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

//Bibliotecas WiiFi
#include <WiFi.h>

#include "ESP32_MailClient.h"

//Biblioteca para leitura do GPS
#include <TinyGPS++.h>

//Biblioteca para o gerenciador de energia
#include <axp20x.h>

//Biblioteca para seperar strings
#include <Separador.h>

//Deixe esta linha descomentada para compilar o Master
//Comente ou remova para compilar o Slave
#define MASTER

Separador s;

AXP20X_Class axp; // define o objeto axp

#define SCK 5   // GPIO5  SCK
#define MISO 19 // GPIO19 MISO
#define MOSI 27 // GPIO27 MOSI
#define SS 18   // GPIO18 CS
#define RST 14  // GPIO14 RESET
#define DI00 26 // GPIO26 IRQ(Interrupt Request)

#define BAND 915E6 //Frequência do radio - exemplo : 433E6, 868E6, 915E6

//Constante para informar ao Slave que queremos os dados
const String GETDATA = "getdata";
const String CALL = "call";
//Constante que o Slave retorna junto com os dados para o Master
const String SETDATA = "setdata=";

//Configurações iniciais do LoRa
void setupLoRa(){ 
  //Inicializa a comunicação
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI00);

  //Inicializa o LoRa
  if (!LoRa.begin(BAND)){
    Serial.println("Erro ao inicializar o LoRa!");
    while (1);
  }

  //Ativa o crc
  LoRa.enableCrc();
  //Ativa o recebimento de pacotes
  LoRa.receive();
}

//Configurações iniciais do gerenciador de energia AXP 192
void setupAXP()
{
  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
}
