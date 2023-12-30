/*
  30.12.2023

  Sensor de umidade e temperatura MODBUS RTU com display
  por R&M Company - v1.1

  ModbusRTU ESP8266/ESP32
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
  modified 13 May 2020
  by brainelectronics

  This code is licensed under the BSD New License. See LICENSE.txt for more info.

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <ModbusRTU.h>

#define REGN 0
#define SLAVE_ID 1
ModbusRTU mb;                // no Modbus RTU usar a Serial2 -> pinos IO17(TX) e IO16(RX)

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

unsigned long previousMillis = 0;     // usado na atualização do display e leitura do sensor
float  t = 0;
float  h = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
bool enableHeater = false;                       // verificar no manual do sensor 

void setup() {
  Serial.begin(115200);
    delay(20);

  Serial.println("Testando o sensor SHT31...");
  if (! sht31.begin(0x44)) {   // i2c addr
    Serial.println("Sensor SHT31 não encontrado!");
    while (1) delay(1);
  }
  Serial.println("Sensor SHT31 OK.");

  Serial.print("Heater Enabled State: ");        // verificar no manual do sensor "Status do Aquecedor"?!
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  u8g2.begin();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(9,36,"R&M Company");  // (coluna, linha, "menssagem")
  } while ( u8g2.nextPage() );

// SETUP MODBUS RTU
Serial2.begin(9600, SERIAL_8N1);
  delay(20);
  mb.begin(&Serial2);
  mb.slave(SLAVE_ID);
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 2000) {
    previousMillis = currentMillis;
     t = sht31.readTemperature();   // ou int8_t t = 
     h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Falha ao ler a temperatura!");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Falha ao ler a umidade!");
  }
  // rotina para mostrar no display
    u8g2.firstPage();
    u8g2.setFont(u8g2_font_ncenB18_tr);
    do {
        u8g2.setCursor(36,27);
        u8g2.print(h);
        u8g2.setCursor(36,57);
        u8g2.print(t);
    } while( u8g2.nextPage() );
  }
  mb.addHreg(0);
  mb.Hreg(0, (h*100));           //  (h*100)  envia a umidade multiplicada por 100
  mb.addHreg(1);
  mb.Hreg(1, (t*100));           //  (t*100)  envia a temperatura multiplicada por 100

  mb.task();
  yield();
}