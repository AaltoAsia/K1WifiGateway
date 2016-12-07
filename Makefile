#SKETCH = 

ESP_ROOT=$(HOME)/Arduino/esp8266

LIBS += $(ESP_LIBS)/ESP8266WiFi \
	$(ESP_LIBS)/ESP8266HTTPClient \
	$(ESP_LIBS)/ESP8266httpUpdate \
	KMEGLib \
	.

UPLOAD_PORT = /dev/ttyUSB0
BOARD = d1_mini

FLASH_DEF = 4M1M

SSH_UPLOAD_SERVER = otaniemi3d

################################################################
############# PIN CONNECTION CONFIGURATION #####################
# SERIAL CONFIGURATION:
# # Serial  UART0: Usb,           GPIO 1  TX (TX);  GPIO 3  RX (RX)
# # -DSWAPSERIAL0 puts Serial to  GPIO 15 D8 (TX);  GPIO 13 D7 (RX)
# # Serial1 UART1: RX from flash  GPIO 2  D4 (TX)
#
# # So use Serial1 for debug transmission (can only transmit) and Serial for hacking the bridge.
# # Now GPIO 1 is not used but is connected to usb serial for debugging, (not needed for production)
# # Connection to wifi-bridge can now send AT commands and receive everything TinyOs is sending
# # but cannot communicate to TinyOs MCU, which is not needed. Also everything sent is also echoed back.
# # NOTE: Not good, TX is High when not transmitting? => requires modifications to the original bridge
# #
# # USB Host        ESP8266               WizFi220        TinyOs MCU
# # USB RX <---+---< USB     TX >         < TX >----------> RX                
# #            |   < GPIO 15 TX >         > RX <-----+----< TX         
# #            `---< GPIO 2  TX >                    |  
# #                > GPIO 13 RX <--------------------´  
# #`


EXTRAFLAGS += -Wall -DDBG -DDBGSTREAM=Serial1 -DYGGDRASIL=Serial -DSWAPSERIAL0 # -DDEBUG_ESP_HTTP_CLIENT -DDEBUG_ESP_PORT=Serial1

C_EXTRA += $(EXTRAFLAGS)
CPP_EXTRA += $(EXTRAFLAGS)



include ./makeEspArduino/makeEspArduino.mk


# This version string is used for OTA update version check
UPLOAD_VERSION=$(SRC_GIT_VERSION)_$(BUILD_TIME)
version: all
	echo "\n  WHOLE VERSION STRING: $(UPLOAD_VERSION)"

httpserver_upload: version
	scp $(MAIN_EXE) $(SSH_UPLOAD_SERVER):.
	ssh $(SSH_UPLOAD_SERVER) -t "/opt/kbridge-update.sh $(MAIN_NAME).bin $(UPLOAD_VERSION)"

setup:
	./setup.sh

