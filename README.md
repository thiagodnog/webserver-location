<h1>esp32LoRa-gps-location-webserver-with-micropython-or-arduinoIDE</h1>

Implementação de um sistema de localização com placas TTGO TBeam ESP32 utilizando micropython com VSCode ou C++ com Arduino IDE.

<h2>Arquitetura</h2>

O dispositivo <em>Slave</em> envia dados de localização (Latitude e Longitude), via LoRa, sempre que o <em>Master</em> requisitar. O <em>Master</em> por sua vez, processa esses dados e envia a um servidor web. 

![image](https://user-images.githubusercontent.com/54367680/114217804-06bd7100-993f-11eb-9bdd-6a4fb394b9b1.png)

<h2>Instação Micropython</h2>

<ul>
  <li>Instale o <a href="https://code.visualstudio.com/">VsCode</a></li>
  <li>Instale a linguagem <a href="https://www.python.org/downloads/">Python</a></li>
  <li>Instale o <em>firmware</em> do <a href="https://www.python.org/downloads/">Micropython</a> na placa</li>
  <li>Clone este repositório ou faça o <em>download</em> .zip. As bibliotecas já estão inclusas.</li>
<ul>
