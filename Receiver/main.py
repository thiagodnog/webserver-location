#---------------------------------------------Importações de bibliotecas locais--------------------------------------------------

from libraries.ulora.sx127x import SX127x                      # Importa a Classe SX127x que configura todo o funcionamento do módulo LoRa sx127
from libraries.ulora.config import *                           # Configuração dos pinos, banda e frequência do LoRa 
from libraries.uaxp202 import axp202                           # Importa o módulo axp202 que configura o gerenciador de energia (axp192) 
from libraries.umail import umail                              # importa o módulo umail que utiliza o protocolo SMTP para envio de e-mails     

#---------------------------------------------Importações de bibliotecas padrões--------------------------------------------------

import machine                                                  # Importa o módulo machine que fornece funções de manipulação de hardware
from machine import Pin, SPI                                    # Importa a Classe Pin (controlar os pinos I/O) e a Classe SPI (protocolo de
                                                                # comunicação)
import time                                                     # Importa o módulo time que fornece funções de manipulação de tempo
import network                                                  # Importa o módulo network que forcene drivers de rede e configurações de 
                                                                # roteamento
import gc                                                       # Importa o módulo gc que fornece funções de coleta de lixo (gerência de 
                                                                # memória automatizada)
try:
  import usocket as socket                                      # Importa o módulo usocket que fornece acesso à interface de soquete BSD 
                                                                # (soquete de rede)
except:
  import socket

#-------------------------------------------------------Configuração de rede-------------------------------------------------------

gc.collect()                                                    # Habilita a coleta

ssid = 'REPLACE_WITH_YOUR_SSID'                                 # Nome da rede sem fio
password = 'REPLACE_WITH_YOUR_PASSWORD'                         # Senha da rede sem fio

station = network.WLAN(network.STA_IF)

station.active(True)
station.connect(ssid, password)                                 # Tenta estabelecer uma conexão WiFi com o ssid e password fornecidos

while station.isconnected() == False:                           # Verifica se a conexão foi estabelecida
  pass

print('Conexão bem sucedida')
print(station.ifconfig())                                       # Verifica o endereço IP

#-------------------------------------------------Definição dos pinos de comunicação------------------------------------------------

device_spi = SPI(1, baudrate = 10000000,                        # 0 ou 1 para amostrar dados na primeira ou na segunda transição do clock
        polarity = 0, phase = 0, bits = 8, 
        firstbit = SPI.MSB,
        sck = Pin(device_config['sck'],                         # SCK definido em device_config em config.py
                    Pin.OUT, Pin.PULL_DOWN),
        mosi = Pin(device_config['mosi'],                       # MOSI definido em device_config em config.py
                    Pin.OUT, Pin.PULL_UP),
        miso = Pin(device_config['miso'],                       # MISO definido em device_config em config.py 
                    Pin.IN, Pin.PULL_UP))

lora = SX127x(device_spi, pins=device_config, parameters=lora_parameters)

#-----------------------------------------------------Implementação das funções-----------------------------------------------------

def receive(lora):                                              # Recebe dados via LoRa e os atribui a uma lista
    print("LoRa Receiver")

    while True:
        if lora.received_packet():                              
            lora.blink_led()
            print('something here')
            payload = str(lora.read_payload())                  
            dados = tokenizar(payload, "," + "(" + ")" + " ")   
            print (dados)
            break
    return dados

def caratereNoString(c, texto):                                 # Verifica se há caracteres no string lido
    for x in texto:
        if c == x:
            return True
    return False

def tokenizar(payload, separadores):                            # Lê uma string e distribui em uma lista de acordo com os separadores definidos
    palavra = ""
    resultado = []
    pos = 0
    for c in payload:
        pos = pos + 1
        if caratereNoString(c, separadores):                    
            if len(palavra) > 0:
                resultado.append(palavra)  
            palavra = ""                                                                            
        elif pos == len(payload):
            resultado.append(c)
        else:
            palavra = palavra + c
    return resultado   

def dadosOrganizados(dados, tipoDoDado):                            # Organiza os dados e retorna os tipos escolhidos
    dados = dados
    if tipoDoDado == 'data':
        data = str(dados[1] + "-" 
                + dados[2] + "-" 
                + dados[3])                                         # ano-mês-dia
        return data
    elif tipoDoDado == 'horario': 
        horaPadrao = dados[4]
        horaBrasil = str(int(horaPadrao) - 3)                       # Timezone -3
        horario = str(horaBrasil + ":"                              # hora no Brasil
                    + dados[5] + ":"                                # minuto
                    + dados[6])                                     # segundo
        return horario
    elif tipoDoDado == 'latitude':
        latitude = str(dados[9])
        return latitude
    elif tipoDoDado == 'longitude':
        longitude = str(dados[10])
        return longitude

def web_page():                                                    # Cria uma página web

  html = """<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <title>ESP32 (LoRa + GPS Location Web Server)</title>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    body {
      margin: 0;
      font-family: Arial, Helvetica, sans-serif;
      text-align: center;
    }
    header {
      margin: 0;
      padding-top: 5vh;
      padding-bottom: 5vh;
      overflow: hidden;
      background-size: cover;
      color: white;
    }
    h2 {
      font-size: 2.0rem;
    }
    p { font-size: 1.2rem; }
    .units { font-size: 1.2rem; }
    .readings { font-size: 2.0rem; }
  </style>
</head>
<body>
  <header>
  <h2>ESP32 (LoRa + GPS Location Web Server)</h2>
  <p><strong>Last received packet:<br/><span>"""+dadosOrganizados(dados,'data')+ " " +dadosOrganizados(dados,'horario')+"""</span></strong></p>
</header>
<main>
  <p>
    <i class="fas fa-globe-europe" style="color:#059e8a;"></i> 
    <span class="dht-labels">Latitude</span> 
    <span>"""+dadosOrganizados(dados,'latitude')+"""</span>
  </p>
  <p> 
    <i class="fas fa-globe-americas" style="color:#00add6;"></i>
    <span class="dht-labels">Longitude</span>
    <span>"""+dadosOrganizados(dados,'longitude')+"""</span>
  </p>
  <img src="https://maps.googleapis.com/maps/api/staticmap?center=-8.118288,-34.910167&zoom=17&size=400x400&markers=color:blue%7Clabel:C%7C-8.118288,-34.910167&key=YOUR_API_KEY">
</body>
</html>"""
  return html

def areaLimite():                                              # Delimita uma área e retorna True se o ponto dado for dentro dela
    latitude = float(dadosOrganizados(dados, 'latitude'))
    longitude = float(dadosOrganizados(dados, 'longitude'))

    if latitude > -8.119628 and latitude < -8.119628 or \
       longitude < -34.910536 and longitude > -34.910038:
        return True
    else:
        return False

def send_email():                                              # Envia um e-mail de aviso
    smtp = umail.SMTP('smtp.gmail.com', 587, 
                    username='my@gmail.com',                   # endereço de e-mail remetente
                    password='mypassword')                     # senha do e-mail remetente
    smtp.to('thiagonogueira1995@gmail.com')                    # endereço de e-mail destinatário
    smtp.send("Criança fora do limite estabelecido.")          # mensagem a ser enviada
    smtp.quit()

#-----------------------------------------------Loop de Implementação do programa--------------------------------------------------

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)          # Cria um sockete usando uma família de endereço fornecida
s.bind(('', 80))                                               # Vincula o socket 
s.listen(5)                                                    # Ativa um servidor para aceitar conexões

while True:
    dados = receive(lora)
    web_page()
    if areaLimite() == True:
        send_email()
    conn, addr = s.accept()
    print('Got a connection from %s' % str(addr))
    request = conn.recv(1024)
    print('Content = %s' % str(request))
    response = web_page()
    conn.send('HTTP/1.1 200 OK\n')
    conn.send('Content-Type: text/html\n')
    conn.send('Connection: close\n\n')
    conn.sendall(response)                                  # Envia um bloco de dados para o socket (a página web como string)
    conn.close()
