# esp-idf-mpd-clinet
MPD(MusicPlayerDaemon) client for ESP-IDF.   
MPD protocol is [here](https://github.com/MusicPlayerDaemon/MPD/blob/master/doc/protocol.rst).   

![mpd-client](https://user-images.githubusercontent.com/6020549/100164763-e941ba80-2efb-11eb-83bd-d46323f56f65.JPG)

# Hardware requirements
M5Stack

# Installation

```
git clone https://github.com/nopnop2002/esp-idf-mpd-client
cd esp-idf-mpd-client
make menuconfig
make flash monitor
```
# Configuration
Set the following items using menuconfig.
- CONFIG_ESP_WIFI_SSID   
SSID of your wifi.
- CONFIG_ESP_WIFI_PASSWORD   
PASSWORD of your wifi.
- CONFIG_ESP_MAXIMUM_RETRY   
Maximum number of retries when connecting to wifi.
- CONFIG_ESP_MPD_IPV4_ADDR   
IP address of your MPD Server.   
Display update cycle (minutes)
- CONFIG_ESP_FONT   
The font to use.

![config-main](https://user-images.githubusercontent.com/6020549/100164797-fa8ac700-2efb-11eb-938b-319961c73cc9.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/100164808-0080a800-2efc-11eb-97a0-dd9cb795c7e6.jpg)

# Operation
__There is no function to create or load a playlist.__   

## mpc toggle
Press Left button briefly.   
State is toggled.

## mpc paly
Press and hold Left button.   

## mpc next
Press Middle button briefly.   

## mpc prev
Press and hold Middle button.   

## mpc volume -5
Press Right button briefly.   

## mpc volume +5
Press and hold Right button.   

# Font File   
You can add your original fonts.   
The format of the font file is the FONTX format.   
Your font file is put in font directory.   
Your font file is uploaded to SPIFFS partition using meke flash.   

Please refer [this](http://elm-chan.org/docs/dosv/fontx_e.html) page about FONTX format.   

```
FontxFile yourFont[2];
InitFontx(yourFont,"/spiffs/your_font_file_name","");
uint8_t ascii[10];
strcpy((char *)ascii, "MyFont");
uint16_t color = RED;
lcdDrawString(&dev, yourFont, x, y, ascii, color);
```

# Font File Editor(FONTX Editor)   
[There](http://elm-chan.org/fsw/fontxedit.zip) is a font file editor.   
This can be done on Windows 10.   
Developer page is [here](http://elm-chan.org/fsw_e.html).   

![FontxEditor](https://user-images.githubusercontent.com/6020549/78731275-3b889800-797a-11ea-81ba-096dbf07c4b8.png)


This library uses the following as default fonts:   
- font/ILGH24XB.FNT // 12x24Dot Gothic
- font/ILMH24XB.FNT // 12x24Dot Mincyo

Changing this file will change the font.


