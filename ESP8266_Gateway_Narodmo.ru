#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
WiFiClient client;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;



ADC_MODE(ADC_VCC);

#include <DallasTemperature.h>     // https://github.com/milesburton/Arduino-Temperature-Control-Library
#define ONE_WIRE_BUS 12              
OneWire oneWire(ONE_WIRE_BUS);     // https://github.com/PaulStoffregen/OneWire
DallasTemperature sensors(&oneWire);
unsigned long TimeDET  = 0;

int count;
float TempDS[10];

const char *STAssid = "SSID!!!!!!!!!!!!!!!!!!!!!!!!";
const char *STApass = "PASS!!!!!!!!!!!!!!!!!!!!!!";
const char *APssid = "Gateway";
const char *APpass = "martinhol221";

String host = "narodmon.ru";
String narodmonBuf;

void setup() {
Serial.begin(115200), Serial.println();
pinMode(13, OUTPUT); digitalWrite(13, HIGH); 
Serial.println(""); Serial.println("");
Serial.println("Шлюз GET - Narodmon.ru");
Serial.print("AP up SSID: "), Serial.print(APssid), Serial.print(" PASS: "), Serial.println(APpass);
Serial.print("STA connect to SSID: "), Serial.print(STAssid), Serial.print(" PASS: "), Serial.println(STApass);
Serial.println("");

WiFi.mode(WIFI_AP_STA); 
WiFi.softAP(APssid, APpass);
WiFi.begin(STAssid, STApass);

Serial.print("Conecting: SSID: ");

while (WiFi.status() != WL_CONNECTED) {count++, delay(500),  Serial.print(".");                                     }
Serial.println(""); 
Serial.print("ESP8266 ID:"), Serial.println(ESP.getChipId());
Serial.print("IP: "),  Serial.println(WiFi.localIP());
Serial.print("MAC: "), Serial.println(WiFi.macAddress()), Serial.println();

sensors.begin(); sensors.requestTemperatures(); 
server.on("/",            handleRoot);
server.on("/sensor",      handleSensor);
server.begin();
httpUpdater.setup(&server);
}


void loop() {
  server.handleClient();
  if (millis()> TimeDET + 300000){narodmonSend() , TimeDET = millis();} 
            }


void narodmonSend () {
int inDS = 0;
sensors.requestTemperatures(); 
delay (1000);
String buf;
buf = "#" + WiFi.macAddress() + "#ESP"  +  ESP.getChipId()  + "\n";    // Заголовок с МАС адресом и ID чипа
buf = buf + "#RSSI#" + WiFi.RSSI() /* + "#Уровень вайфай" */ + "\n";   // Уровень вайфай
buf = buf + "#VBAT#" + ESP.getVcc()/* + "#Напряжение" */ + "\n";              // Напряжение
while (inDS < 10){
        float TempDS = sensors.getTempCByIndex(inDS);                  // читаем температуру
        if (TempDS == -127.00) break;                                  // пока не доберемся до неподключенного датчика
        inDS++;                                                               
        buf = buf + "#Temp"+inDS+"#"+TempDS+ "\n" /* + "#температура" */; // дописываем в строку с температурой
        } 


buf = buf + narodmonBuf;
buf = buf + "#Uptime#" +millis()/1000+"\n";                            // время работы       
buf = buf + "##";                                                      // закрываем пакет ##
narodmonBuf ="";  
Serial.println("Соеденение с сервером narodmon.ru...."); 
if (!client.connect(host, 8283)) {

    Serial.println("нет соединения"); return;}
client.print(buf);                                                  
Serial.println(buf);}  


void  handleSensor(){        
for (int i = 1; i < 9; ++i) {
if (server.arg("name"+String(i)) !="" && server.arg("val"+String(i)) !="") {  
narodmonBuf = narodmonBuf + "#"+server.arg("name"+String(i))+"#" +server.arg("val"+String(i)) + "\n"; }}
server.send(200, "text/html", "ok");}

// http://192.168.4.1/sensor?name1=TEMP4&val1=-12.24&name2=TEMP5&val2=14.68&name3=TEMP6&val3=-15.24&name4=TEMP7&val4=15.68

void handleRoot() {String http = "Debug mode <br>narodmonBuf:<br>" + narodmonBuf 
+ "<br> ResetReason: " + ESP.getResetReason()
+ "<br> FreeHeap: " + String(ESP.getFreeHeap())
+ "<br> Fragmentation: " + String(ESP.getHeapFragmentation())
+ "<br> FreeBlockSize: " + String(ESP.getMaxFreeBlockSize())
+ "<br> <a href=\"/update\">Update FW</a>";
server.send(200, "text/html", http);}
                    
                           
