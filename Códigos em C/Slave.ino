//Compila apenas se MASTER não estiver definido no arquivo principal
#ifndef MASTER

//Contador que irá servir como o dados que o Slave irá enviar
int count = 0;

TinyGPSPlus gps;   // define o objeto gps
HardwareSerial GPS(1); // define a leitura na porta serial como GPS

// Função que faz um delay de forma mais esperta da porta Serial
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}

void setup(){
    Serial.begin(115200);
    //Chama a configuração inicial do AXP
    setupAXP();
    //Chama a configuração inicial do LoRa
    setupLoRa();
    Serial.println("Slave esperando...");
    GPS.begin(9600, SERIAL_8N1, 34, 12);   // Inicia a leitura dos dados de GPS pela UART2 | 34-TX 12-RX
}

void loop(){
  //Tenta ler o pacote
  int packetSize = LoRa.parsePacket();

  //Verifica se o pacote possui a quantidade de caracteres que esperamos
  if (packetSize == GETDATA.length()){
    String received = "";

    //Armazena os dados do pacote em uma string
    while(LoRa.available()){
      received += (char) LoRa.read();
    }

    if(received.equals(GETDATA)){
      axp.setChgLEDMode(AXP20X_LED_OFF);
      //Simula a leitura dos dados
      String data = readGpsData();
      //Cria o pacote para envio
      LoRa.beginPacket();
      LoRa.print(SETDATA + data);
      //Finaliza e envia o pacote
      LoRa.endPacket();

    }
  } else if (packetSize == CALL.length()){
      String received = "";

      //Armazena os dados do pacote em uma string
    while(LoRa.available()){
      received += (char) LoRa.read();
    }
    
    if(received.equals(CALL)){
      axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
      //Simula a leitura dos dados
      String data = readGpsData();
      Serial.println("Criando pacote para envio");
      //Cria o pacote para envio
      LoRa.beginPacket();
      LoRa.print(SETDATA + data);
      //Finaliza e envia o pacote
      LoRa.endPacket();
      Serial.println("Enviou: " + String(data));

    }
    
  }
}

//Função onde se faz a leitura dos dados que queira enviar
//Poderia ser o valor lido por algum sensor por exemplo
//Aqui vamos enviar apenas um contador para testes
//mas você pode alterar a função para fazer a leitura de algum sensor
String readGpsData(){
  String gpsLatitude = String(gps.location.lat(),6);
  String gpsLongitude = String(gps.location.lng(),6);
  String gpsPeriodo = String(gps.date.value());
  String gpsHora = String(gps.time.value());
  smartDelay(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("Nenhum dado GPS recebido: verifique a conexão"));
  return String(gpsLatitude + " " + gpsLongitude + " " + gpsPeriodo + " " + gpsHora);
}

#endif
