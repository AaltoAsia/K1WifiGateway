# K1WifiGateway

Program for using ESP8266 to hack a certain 6LoWPAN to WiFi bridge system to use O-MI.
Data is sniffed from a serial connection.


Installation to an ESP8266
--------------------------

Certificate and key need to be in unencrypted (empty passphrase) DER format.

Example for making keys:
generate the private key (a 1024 bit key is used in this example)

`> openssl genrsa -out my_private_key.pem 1024`

convert the private key into DER format

`> openssl rsa -in ./my_private_key.pem -out ./my_private_key -outform DER`

generate the certificate request using the previously generated private key

`> openssl req -new -key my_private_key.pem -out my_cert_req.pem`

get the certificate from Certificate Authority
(see other guides if you are managing your own CA)

convert the certificate into DER format 

`> openssl x509 -in my_cert.pem -out my_cert -outform DER`

Then you can use SPIFFS or convert the keys to C code header with

`> xxd -i client.crt > MyCertificates.h`
`> xxd -i client.key >> MyCertificates.h`



Hardware setup
==============


SERIAL CONFIGURATION:
-------------------

Serial pin configuration can be changed in the Makefile. The default
configuration is below, NodeMCU pin name is in parens.

Description                 | Transmit,TX   | Receive (RX)
----------------------------|---------------|------------------
Serial  UART0: Usb,         | GPIO 1  (TX)  | GPIO 3  (RX)
-DSWAPSERIAL0 puts Serial to| GPIO 15 (D8)  | GPIO 13 (D7)
Serial1 UART1: RX from flash| GPIO 2  (D4)  |

Use Serial1 for debug transmission (it can only transmit) and Serial for
hacking the bridge.  Now GPIO 1 is not used but is connected to usb serial for
debugging, (not needed for production) Connection to wifi-bridge can now send
AT commands and receive everything TinyOs is sending but cannot communicate to
TinyOs MCU, which is not needed. Also everything sent is also echoed back.
NOTE: Not good, TX is High when not transmitting? => requires modifications to
the original bridge

```
USB Host        ESP8266               WizFi220        TinyOs MCU
USB RX <---+---< USB     TX >         < TX >----------> RX                
           |   < GPIO 15 TX >         > RX <-----+----< TX         
           `---< GPIO 2  TX >                    |  
               > GPIO 13 RX <--------------------´  
```

OTHER CONNECTIONS: (for restarting in case of crash)
--------------------------------------------------

D8 GPIO15| D3 GPIO0 | D4 GPIO2 |Mode
---------|----------|----------|-----------------
0V       |0V        |3.3V      |Uart Bootloader
0V       |3.3V      |3.3V      |BOOT SKETCH
3.3V     |x         |x         |SDIO mode (not used for Arduino)

