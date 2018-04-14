#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#define ONE_WIRE_BUS 2                         // Шина датчика температуры с подтяжеой 4,7к на VCC
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Ticker flipper;


const char *ssid =  "111111111111111";         // имя точки доступа вашей домашней Wi-Fi сети
const char *pass =  "22222222222222";          // пароль доступа вашей домашней Wi-Fi сети
const char *mqtt_server = "m99.cloudmqtt.com"; // сервер MQTT броккера
const int mqtt_port = 10077;                   // порт MQTT броккера
const char *mqtt_user = "drive2ru";            // логин MQTT броккера
const char *mqtt_pass = "martinhool221";       // пароль MQTT броккера
const char *host = "ESP8266";                  // идентификатор в локальной сети
ADC_MODE(ADC_VCC);                             // активация режима чтения Внутренним АЦП напряжения с VCC 

#define BUFFER_SIZE 100;
WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

int count = 5;                                 // счетчик таймера для ежеминутной отправки данных
int inDS = 0;                                  // индекс датчика в массиве температур
int sleep = 5;                                 // таймер сна в минутах , необходимо соеденить GPIO16 и RESET
float TempDS[11];                              // массив хранения температуры c рахных датчиков 
float Vbat,V_min = 3.00;                       // напряжение батарейки, и минимальный порог напряжения для разрешения работы
String EspTopic = "";    

void callback(const MQTT::Publish& pub){  
  String payload = pub.payload_string();       // читаем топики, если что скармливаем переменной  
  if(String(pub.topic()) == EspTopic+"/sleep") sleep = payload.toInt(), Serial.print ("Установка таймера сна на: "), Serial.print (sleep), Serial.println (" минут.");
  
  
  
  
  }

void MqttSend(){
  inDS = 0;
  sensors.requestTemperatures();              // читаем температуру с трех датчиков
  while (inDS < 10){
        TempDS[inDS] = sensors.getTempCByIndex(inDS);        // читаем температуру
    if (TempDS[inDS] == -127.00) break;                     // пока не доберемся до неподключенного датчика
              inDS++;} 
  for (int i=0; i < inDS; i++){
  Serial.print("Температура с датчика "), Serial.print(i), Serial.print(": "),Serial.println(TempDS[i]);
  client.publish(EspTopic+"/dsw"+ String(i),   String(TempDS[i]));}
  client.publish(EspTopic+"/adc",               String(Vbat));
  client.publish(EspTopic+"/rssi",              String(WiFi.RSSI()));
  client.publish(EspTopic+"/uptime",            String(millis()/1000));
  Serial.print("Данные отправлены, IP Адрес: "),         Serial.println(WiFi.localIP());
  if (sleep != 0) Serial.print("Засыпаю на "), Serial.print(sleep), Serial.println(" мин."), ESP.deepSleep(sleep*60000000);
               }

void ConMqtt () {
  if (WiFi.status() == WL_CONNECTED){
    if (!client.connected()){
       if (client.connect(MQTT::Connect(host)
           .set_auth(mqtt_user, mqtt_pass))){     // Читаем эти топики         
           client.set_callback(callback);  

              
           client.subscribe(EspTopic+"/sleep"); }
                            }
    if (client.connected()) client.loop();
                                    }
                }

void setup() {
  Serial.begin(115200);
  Serial.println("");
  EspTopic = "ESP" + String (ESP.getChipId());
  Serial.print("Загрузка модуля: "), Serial.println(EspTopic);
  Vbat =  ESP.getVcc();         // читаем напряжение на ноге VCC модуля ESP8266
  Vbat =  Vbat / 1023;          
  //Vbat = analogRead(A0);      // читаем напряжение на ноге ADC модуля ESP8266
  //Vbat = Vbat / 500 ;
  Serial.print("Заряд батареи: "), Serial.print(Vbat), Serial.println(" вольт");
  if (Vbat < V_min ) Serial.println("Низкий заряд батареи, засыпаю на  30 минут"), ESP.deepSleep(30*60000000);
  
  sensors.begin();
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);       //  пин для подключения запитки датчика DS18B20
  flipper.attach(1, flip);
}

void flip(){count--, Serial.print("Счетчик: "), Serial.println(count);}

void loop(){
if (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass); 
    if (WiFi.waitForConnectResult() != WL_CONNECTED);
    return;}

 httpServer.handleClient();      // проверяем http запрос
 ConMqtt();                      // проверяем соеденение и подписываемся на топики
 
 if (count <= 0) flipper.detach(), MqttSend(), count = 60, flipper.attach(1, flip);  // замеряем и отправляем данные
 //if (millis()> 60000) Serial.println("Низкий заряд, засыпаю на 20 минут"), ESP.deepSleep(20*60000000);
}

