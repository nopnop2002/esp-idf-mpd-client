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
idf.py menuconfig
idf.py flash monitor
```


# Configuration
Set the following items using menuconfig.
- CONFIG_ESP_WIFI_SSID   
SSID of your wifi.
- CONFIG_ESP_WIFI_PASSWORD   
PASSWORD of your wifi.
- CONFIG_ESP_MAXIMUM_RETRY   
Maximum number of retries when connecting to wifi.
- CONFIG_ESP_MPD_SERVER   
IP address or mDNS name of your MPD Server.   
- CONFIG_ESP_FONT   
The font to use.
- CONFIG_ENCODER   
Enable incremental rotary encoder as volume.   
See below.

![config-main](https://user-images.githubusercontent.com/6020549/100164797-fa8ac700-2efb-11eb-938b-319961c73cc9.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/188337603-955c45ec-c1fe-4c71-bc36-b834a1436bde.jpg)

# Operation
__There is no function to create or load a playlist.__   

## mpc toggle
Press Left button briefly.   
State is toggled.

## mpc paly
Press and hold Left button for at least 2 seconds.   

## mpc next
Press Middle button briefly.   

## mpc prev
Press and hold Middle button for at least 2 seconds.   

## mpc volume -5
Press Right button briefly.   

## mpc volume +5
Press and hold Right button for at least 2 seconds.   

# Incremental rotary encoder as volume
You can use incremental rotary encoder as volume.   
The source code is based on [here](https://github.com/UncleRus/esp-idf-lib/tree/master/components/encoder).

|Encoder||M5Stack|
|:-:|:-:|:-:|
|A-CH(CLK)|--|GPIO21 or GROVE White Line|
|B-CH(DT)|--|GPIO22 or GROVE Yellow Line|
|BUTTON(SW)|--|N/C|
|VCC|--|3.3V or GROVE Red Line|
|GND|--|GND or GROVE Black Line|

![mpd-client-encoder](https://user-images.githubusercontent.com/6020549/100492796-311e4700-3173-11eb-8968-15d0fbd4694e.JPG)

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


