#---------------------------------------------Importações de bibliotecas locais--------------------------------------------------

from libraries.uaxp202 import axp202                           # Importa o módulo axp202 que configura o gerenciador de energia (axp192) 
from libraries.ulora.config import *                           # Configuração dos pinos, banda e frequência do LoRa 
from libraries.ulora.sx127x import SX127x                      # Importa a Classe SX127x que configura todo o funcionamento do módulo LoRa sx127

#---------------------------------------------Importações de bibliotecas padrões--------------------------------------------------

import time                                                     # Importa o módulo time que fornece funções de manipulação de tempo
from time import sleep
import machine                                                  # Importa o módulo machine que fornece funções de manipulação de hardware
from machine import Pin, SPI                                    # Importa a Classe Pin (controlar os pinos I/O) e a Classe SPI (protocolo de
                                                                # comunicação)

#-------------------------------------------------Definição dos pinos de comunicação------------------------------------------------
device_spi = SPI(1, baudrate = 10000000, 
        polarity = 0, phase = 0, bits = 8, 
        firstbit = SPI.MSB,
        sck = Pin(device_config['sck'],                         # SCK definido em device_config em config.py
                        Pin.OUT, Pin.PULL_DOWN),
        mosi = Pin(device_config['mosi'],                       # MOSI definido em device_config em config.py
                        Pin.OUT, Pin.PULL_UP),
        miso = Pin(device_config['miso'],                       # MISO definido em device_config em config.py 
                        Pin.IN, Pin.PULL_UP))

lora = SX127x(device_spi, pins=device_config, parameters=lora_parameters)

GPS_RX_PIN = 34
GPS_TX_PIN = 12

uart = machine.UART(2, rx=GPS_RX_PIN, tx=GPS_TX_PIN, baudrate=9600, bits=8, parity=None, stop=1, timeout=1500, buffer_size=1024, lineend='\r\n')

#-------------------------------------------------Definição do gerenciador de energia-----------------------------------------------

axp = axp202.PMU(address=axp202.AXP192_SLAVE_ADDRESS)
axp.setLDO2Voltage(3300)                                        # T-Beam LORA VDD   3v3
axp.setLDO3Voltage(3300)                                        # T-Beam GPS  VDD    3v3
axp.enablePower(axp202.AXP192_LDO3)
axp.enablePower(axp202.AXP192_LDO2)

#-----------------------------------------------------Implementação das funções-----------------------------------------------------

def send(lora,uart):                                           # Envia dados via LoRa 
        counter = 0
        print("LoRa Sender")

        gps = machine.GPS(uart)
        gps.init()
        gps.startservice()
        gps.service()

        while True:
                data = gps.getdata()[0]
                latitude = gps.getdata()[1]
                longitude = gps.getdata()[2]
                rssi = lora.packet_rssi()
                payload = str(data) + " " + str(latitude) + " " + str(longitude) + " " + str(rssi) + " " + '({0})'.format(counter)
                print("Sending packet: \n{}\n".format(payload))
                lora.println(payload)

                counter += 1
                sleep(5)

send(lora,uart)