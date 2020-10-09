//Compila apenas se MASTER estiver definido no arquivo principal
#ifdef MASTER

//Intervalo entre os envios
#define INTERVAL 500

//Tempo do último envio
long lastSendTime = 0;

// Valor limite para identificar o toque
const int VALUE_THRESHOLD = 30;

String gpsData = "";

String gpsDataType = "";

String latitude;
String longitude;

String header;

String callstate = "off";// state of call child 

const char* ssid     = "UM_SSDID_QUALQUER";
const char* password = "UMA_SENHA_QUALQUER";

#define GMAIL_SMTP_SEVER "smtp.gmail.com"
#define GMAIL_SMTP_USERNAME "um_email_qualquer@gmail.com"
#define GMAIL_SMTP_PASSWORD "uma_senha_qualquer"
#define GMAIL_SMTP_PORT 465  

SMTPData smtpData;

WiFiServer server(80);

void setup(){
  Serial.begin(115200);
  //Chama a configuração inicial do AXP
  setupAXP();
  //Chama a configuração inicial do LoRa
  setupLoRa();
  Serial.println("Master");
  // connecting to your WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");    // this is the address to use for viewing the map
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop(){
  //Se passou o tempo definido em INTERVAL desde o último envio
  if (millis() - lastSendTime > INTERVAL){
    //Marcamos o tempo que ocorreu o último envio
    lastSendTime = millis();
    //Envia o pacote para informar ao Slave que queremos receber os dados
    send();
  }

  //Verificamos se há pacotes para recebermos
  receive();
  
  WiFiClient client = server.available();    // listen for incoming clients

  if (client) {  
    String latitude = dataProcessing("latitude");
    String longitude = dataProcessing("longitude");

    if (crossedLimitedArea( latitudeFloat, longitudeFloat) == true) {
       String result = sendEmail("ALERTA!!!", "ESP32", "Criança saiu do perímetro pré estabelecido!", "destinatario@gmail.com", false);
    }  
                               
    Serial.println("new client");          
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            
      if (client.available()) {             // if there's client data
          char c = client.read();          // read a byte
          Serial.write(c);                    // print it out the serial monitor
          header += c;
          if (c == '\n') {                      // check for newline character,
            if (currentLine.length() == 0) {  // if line is blank it means its the end of the client HTTP request

              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // turns the GPIOs on and off
              if (header.indexOf("GET /call/on") >= 0) {
                Serial.println("call child on");
              callstate = "on";
              } else if (header.indexOf("GET /call/off") >= 0) {
                Serial.println("call child off");
              callstate = "off";
              }
              
              client.println("<!DOCTYPE html>"); // open wrap the web page
              client.print("<html>"
                              "<head>"
                                "<meta name='viewport' content='width=device-width, initial-scale=1'><meta charset='utf-8'>"
                                "<link rel='icon' href='data:,'>"
                                "<meta http-equiv='refresh' content='5'>"
                                "<title>ESP32 (LoRa + GPS Location Web Server)</title>"
                                "<link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>"
                                    "<style>"
                                        "#map {height: 280px; width: 100%;}html, body {height: 100%;margin: 0;padding: 0;}"
                                        "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
                                        ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;"
                                        "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
                                        ".button2 {background-color: #77878A;}"
                                    "</style>"
                              "</head>");
              
              client.print("<center><body><h1>ESP32 (LoRa + GPS Location Web Server)</h1>"
                           "<p><i class='fas fa-globe-europe' style='color:#059e8a;'></i><span class='dht-labels'> Latitude: </span><span>" + latitude + "</span></p>"
                           "<p><i class='fas fa-globe-americas' style='color:#00add6;'></i><span class='dht-labels'> Longitude: </span><span>" + longitude + "</span></p>");
              client.println("<p>Call Child - State " + callstate + "</p>");

              if (callstate == "off") {
                client.println("<p><a href=\"/call/on\"><button class=\"button\">ON</button></a></p>");
              } else {
                client.println("<p><a href=\"/call/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
              
              client.print("<div id='map'>"
                           "</div>"
                           "<script>"
                              "function initMap(){"
                              "var child = {lat: " + latitude + ", lng: " + longitude + "}; "
                              "var map = new google.maps.Map(document.getElementById('map'),{center: child, zoom: 18, mapTypeId: 'roadmap'}); "
                              "var marker = new google.maps.Marker({position: child, map: map});}"
                           "</script>"
                           "<script async defer "
                           "src='https://maps.googleapis.com/maps/api/js?key=AIzaSyBYW3OTD8PgQ7aQhiHmmYOm1CBZFn-K0Hs&callback=initMap'>"
                           "</script>");
  
  
  
              client.print("</body></html>"); // close wrap the web page
  
              // The HTTP response ends with another blank line:
              client.println();
            // break out of the while loop:
            break;
          } else {   currentLine = ""; }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
         // here you can check for any keypresses if your web server page has any
      }
    }
    // close the connection:
    header = "";
    client.stop();
    Serial.println("client disconnected");
    }
}

void send(){
  //Inicializa o pacote
  LoRa.beginPacket();
  if(callstate == "on"){
    //Envia o que está contido em "CALL"
    LoRa.print(CALL);
    //Finaliza e envia o pacote
    LoRa.endPacket();  
  } else {
  //Envia o que está contido em "GETDATA"
  LoRa.print(GETDATA);
  //Finaliza e envia o pacote
  LoRa.endPacket();
  }
}

void receive(){
  //Tentamos ler o pacote
  int packetSize = LoRa.parsePacket();
  
  //Verificamos se o pacote tem o tamanho mínimo de caracteres que esperamos
  if (packetSize > SETDATA.length()){
    String received = "";
    //Armazena os dados do pacote em uma string
    while(LoRa.available()){
      received += (char) LoRa.read();
    }
    //Verifica se a string possui o que está contido em "SETDATA"
    int index = received.indexOf(SETDATA);
    if(index >= 0){
      //Recuperamos a string que está após o "SETDATA",
      //que no caso serão os dados de nosso interesse
      String data = received.substring(SETDATA.length());
      //Tempo que demorou para o Master criar o pacote, enviar o pacote,
      //o Slave receber, fazer a leitura, criar um novo pacote, enviá-lo
      //e o Master receber e ler
      String waiting = String(millis() - lastSendTime);
      //Mostra no monitor serial os dados e o tempo que a operação demorou
      //Serial.println("Recebeu: " + data);
      //Serial.println("Tempo: " + waiting + "ms");
      gpsData = data;
    }
  }
}

String dataProcessing(String gpsDataType){
  String gpsDataReceived = gpsData;
  String latitude = s.separa(gpsDataReceived, ' ', 0);
  String longitude = s.separa(gpsDataReceived, ' ', 1);
  String periodo = s.separa(gpsDataReceived, ' ', 2);
  String hora = s.separa(gpsDataReceived, ' ', 3);
  if (gpsDataType == "latitude"){
   Serial.println("A latitude é:" + latitude);
   return latitude; 
  }
  if (gpsDataType == "longitude"){
   return longitude; 
  }
  if (gpsDataType == "periodo"){
   return longitude; 
  }
  if (gpsDataType == "hora"){
   return longitude; 
  }
}

bool crossedLimitedArea(float latitude, float longitude) {
  //float latitudeNum = latitude.toFloat();
  //float longitudeNum = longitude.toFloat();

  if (latitude > -8.117636 or latitude < -8.119630 or longitude < -34.910873 or longitude >  -34.910211) {
    return true;
  } else {
    return false;
  }
}

String sendEmail(char *subject, char *sender, char *body, char *recipient, boolean htmlFormat) {
  smtpData.setLogin(GMAIL_SMTP_SEVER, GMAIL_SMTP_PORT, GMAIL_SMTP_USERNAME, GMAIL_SMTP_PASSWORD);
  smtpData.setSender(sender, GMAIL_SMTP_USERNAME);
  smtpData.setSubject(subject);
  smtpData.setMessage(body, htmlFormat);
  smtpData.addRecipient(recipient);
  if (!MailClient.sendMail(smtpData)) 
    return MailClient.smtpErrorReason();
  
  return "";
}
#endif
