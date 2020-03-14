#include <AHT10.h>
#include <Wire.h>
uint8_t readStatus = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
WiFiClient client;
#define PIN_POWER_DS 13                    // Шина питания датчика температуры AHT10
const char* ssid = "AAAA";                  // имя удаленной точки доступа роутера 
const char* password = "BBBB";   // пароль удаленной точки доступа
unsigned long Time1 = 0; 
ADC_MODE(ADC_VCC);
float Vbat,V_min = 3.00;                   // напряжение батарейки, и минимальный порог напряжения для разрешения работы

void setup()   {
Serial.begin(115200);  
Serial.print("Загрузка модуля: ");
Vbat =  ESP.getVcc();         // читаем напряжение на ноге VCC модуля ESP8266
Vbat =  Vbat / 1023;          
Serial.print("Заряд батареи: "), Serial.print(Vbat), Serial.println(" вольт");
if (Vbat < V_min ) Serial.println("Низкий заряд батареи, засыпаю на  30 минут"), ESP.deepSleep(1800*1000000);
pinMode     (PIN_POWER_DS, OUTPUT); 
digitalWrite(PIN_POWER_DS, HIGH); 
WiFi.mode   (WIFI_AP_STA);         // запускаем смешенный режим 
WiFi.softAP ("ESP","martinhol");   // поднимаем соaт точку доступа
WiFi.begin  (ssid, password);
Serial.println("Подключаюсь к сети "),      Serial.println(ssid);
int count = 0;
while (WiFi.status() != WL_CONNECTED)  {
  delay(500), Serial.print("."), count++ ;
  if (count > 60) Serial.println(" cон на 10 минут"), ESP.deepSleep(10*60*1000000); // в случае не подключения засыпаем на 10 минут
  };
Serial.print("WiFi подключен, ChipId: "), Serial.println(ESP.getChipId());
Serial.print("IP Адрес: "),               Serial.println(WiFi.localIP());
Serial.print("MAC Адрес: "),              Serial.println(WiFi.macAddress()), Serial.println();
while (myAHT10.begin(4,5) != true)       {Serial.println(F("AHT10/AHT15 не активен или не подключён")); delay(5000);}
Serial.print(F("Температура: "));         Serial.print(myAHT10.readTemperature()); Serial.print(F(" +-0.3C")); //by default "AHT10_FORCE_READ_DATA"
Serial.print(F(" Влажность...: "));       Serial.print(myAHT10.readHumidity());    Serial.println(F(" +-2%"));   //by default "AHT10_FORCE_READ_DATA"

               }

void loop()    {
//if (millis()> Time1 + 300000) Time1 = millis(), narodmonSend ();       // выполняем функцию narodmonSend каждые 10 сек для теста

narodmonSend ();
Serial.println("Засыпаем на 5 минут"); ESP.deepSleep(5*60*1000000);          // спать на 5 минут пины D16 и  RST должны быть соеденены между собой
                }


void narodmonSend () {
//float vbat = analogRead(A0); 
// vbat = vbat / 3; 
delay (1000);
String buf;
buf = "#" + WiFi.macAddress() + "#ESP"  +  ESP.getChipId()  + "\n";    // Заголовок с МАС адресом и ID чипа
buf = buf + "#RSSI#" + WiFi.RSSI() /* + "#Уровень вайфай" */ + "\n";   // Уровень вайфай
buf = buf + "#VBAT#" + ESP.getVcc()/* + "#Напряжение" */ + "\n";              // Напряжение
buf = buf + "#Temp#" + myAHT10.readTemperature()+ "\n"; // дописываем в строку с температурой
buf = buf + "#Hum#" + myAHT10.readHumidity()+ "\n"; // дописываем в строку с влажностью
buf = buf + "#Uptime#" +millis()/1000+"\n";                            // время работы       
buf = buf + "##";                                                      // закрываем пакет ##
  
Serial.println("Соеденение с сервером narodmon.ru...."); 
if (!client.connect("narodmon.ru", 8283)) {
//if (!client.connect("185.245.187.13", 8283)) {  
    Serial.println("нет соединения");
    return;
                                          }
client.print(buf);                                                     // и отправляем данные   
Serial.println(buf);
delay(1000);  
                    }              
