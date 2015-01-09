#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h> // http://knolleary.net/arduino-client-for-mqtt/
#include "nRF24L01.h"
#include "RF24.h"
 
// MAC Adresse des Ethernet Shields
byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
// IP des MQTT Servers
byte server[] = { 10, 0, 1, 187 };
// Ethernet Client zur Kommunikation des MQTT Clients
EthernetClient ethClient;
// MQTT Client zur Kommunikation mit dem Server
// Server - Variable des Types byte mit Serveradresse
// 1883 - Ist der Standard TCP Port
// callback - Function wird aufgerufen wen MQTT Nachrichten eintreffen. Am ende des Sketches
// ethClient - Angabe des Ethernet Clients
PubSubClient mqttClient(server, 1883, callback, ethClient);
 
//Funk
RF24 radio(7,8);
const uint64_t pipes[10] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL,0xF0F0F0F0E3LL,0xF0F0F0F0E4LL,0xF0F0F0F0E5LL,0xF0F0F0F0E6LL,0xF0F0F0F0E7LL,0xF0F0F0F0E8LL,0xF0F0F0F0E9LL,0xF0F0F0F0E0LL };

// Timervariable für eine Verzögerung. Als alternative zu delay was die verarbeitung anhält.
int timer = 0;
int timer2 = 0;
int EndTimer = 6000; 
int EndTimer2 = 500; 
int TimeOutC = 100; //Funk Timeout
 
// Pins des RGB LED Strip
int LEDred=5;
int LEDgreen=4;
int LEDir=3;
int IRRec=6;

boolean on[10] = {false,false,false,false,false,false,false,false,false,false};
boolean curon[10] = {true,true,true,true,true,true,true,true,true,true}; //Am Start wird davon ausgegangen, dass alles an ist

int rot[10] = {0,0,0,0,0,0,0,0,0,0}; //Initialwert zum resetten
int blau[10] = {0,0,0,0,0,0,0,0,0,0};
int gruen[10] = {0,0,0,0,0,0,0,0,0,0};

int currot[10] = {0,0,0,0,0,0,0,0,0,0};
int curblau[10] = {0,0,0,0,0,0,0,0,0,0};
int curgruen[10] = {0,0,0,0,0,0,0,0,0,0};

int retry[10] = {0,0,0,0,0,0,0,0,0,0};
int latenz[10] = {50,50,50,50,50,50,50,50,50,50};
int curlatenz[10] = {50,50,50,50,50,50,50,50,50,50};

boolean lanok = false;

String intToHex(int i)
{
  if(i < 16)
    return "0"+String(i,HEX);
  else
    return String(i,HEX);
}

boolean sendCommand(String genString)
{
  digitalWrite(LEDgreen,LOW);
  boolean ret = false;
  //Sende Befehl
  Serial.println("Funke: "+genString);
  char data[10];
  genString.toCharArray(data,10);
  radio.write(&data, sizeof(char)*10);
  
  
  //Warte auf Antwort
  radio.startListening();
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > TimeOutC )
      timeout = true;
        
  if (timeout)
  {
    digitalWrite(LEDred, HIGH);
    Serial.println("Timeout");
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    char recd[10];
    radio.read( &recd, sizeof(char)*10 );
    Serial.println("Recv "+String(recd));
    digitalWrite(LEDgreen, HIGH);
    digitalWrite(LEDred, LOW);
    ret = true;
  }
  radio.stopListening();
  return ret;
}

void sendLatenz(int id, int n)
{
  //Retry Timeout - Schalte Licht aus
  if(retry[id] > 6)
  {
    retry[id] = 0;
    on[id] = false;
    curon[id] = false;
    latenz[id] = curlatenz[id];
    return;
  }
  
  if(sendCommand(""+String(id)+"l"+String(latenz[id])))
  {
    retry[id] = 0;
    curlatenz[id] = latenz[id];
    Serial.println("Set Latenz: "+String(id)+" - "+String(latenz[id]));
  } else
    ++retry[id];
}

void sendOnOff(int id, boolean n)
{
  //Retry Timeout - Schalte Licht aus
  if(retry[id] > 6)
  {
    retry[id] = 0;
    on[id] = false;
    curon[id] = false;
    return;
  }
  
  String onoff;
  char pubonoff[2];
  if(n == true)
  {
    onoff = "on";
    String("1").toCharArray(pubonoff,2);
  }
  else
  {
    onoff = "off";
    String("0").toCharArray(pubonoff,2);
  }
    
  if(sendCommand(""+String(id)+onoff))
  {
    String command = "/arduout/"+String(id)+"/STATUS";
    char pubcommand[30];
    
    command.toCharArray(pubcommand,30);
    retry[id] = 0;
    curon[id] = on[id];
    mqttClient.publish(pubcommand, pubonoff);
    Serial.println("Publish: "+command+" - "+pubonoff);
  } else
    ++retry[id];
}

void sendColor(int id,int r,int g, int b)
{
  //Retry Timeout - Schalte Licht aus
  if(retry[id] > 20)
  {
    retry[id] = 0;
    on[id] = false;
    curon[id] = false;
    return;
  }
    
  if(sendCommand(""+String(id)+intToHex(r)+intToHex(g)+intToHex(b)))
  {
    currot[id] = r;
    curblau[id] = b;
    curgruen[id] = g;
    retry[id] = 0;
  } else
    ++retry[id];
}

void setup()
{
 Serial.begin(57600);

 // Setzen der PINS als Ausgang
 pinMode(LEDir, OUTPUT);
 pinMode(LEDred, OUTPUT);
 pinMode(LEDgreen, OUTPUT);
 
 digitalWrite(LEDred,HIGH);
 Serial.println("[NET INIT]");
 
 // Initialisierung des Ethernets
 if (Ethernet.begin(mac) == 0) 
 {
   Serial.println("[NET ERROR]");
   while (true); //Beende Script
 }
 else 
 {
   // Wenn DHCP OK ist dann grün setzen
   Serial.println("[NET DHCP OK]");
   lanok = true;
 }
 
  
 Serial.println("[FUNK INIT]");
 radio.begin();
 radio.setRetries(15,15);
 radio.setPayloadSize(8);
 
 //Wähle Empfangs und Sendekanal
 radio.openWritingPipe(pipes[0]);
 radio.openReadingPipe(1,pipes[1]);
}
 
void loop()
{
 // Aufbau der Verbindung mit MQTT falls diese nicht offen ist.
 if (!mqttClient.connected()) {
 mqttClient.connect("arduinoBridge");
 // Abonieren von Nachrichten mit dem angegebenen Topics
 /*
 mqttClient.subscribe("/arduino/1/SWITCH/#");
 mqttClient.subscribe("/arduino/1/RED/#");
 mqttClient.subscribe("/arduino/1/GREEN/#");
 mqttClient.subscribe("/arduino/1/BLUE/#");
 mqttClient.subscribe("/arduino/2/SWITCH/#");
 mqttClient.subscribe("/arduino/2/RED/#");
 mqttClient.subscribe("/arduino/2/GREEN/#");
 mqttClient.subscribe("/arduino/2/BLUE/#");
 mqttClient.subscribe("/arduino/3/SWITCH/#");
 mqttClient.subscribe("/arduino/3/RED/#");
 mqttClient.subscribe("/arduino/3/GREEN/#");
 mqttClient.subscribe("/arduino/3/BLUE/#");
 mqttClient.subscribe("/arduino/4/SWITCH/#");
 mqttClient.subscribe("/arduino/4/RED/#");
 mqttClient.subscribe("/arduino/4/GREEN/#");
 mqttClient.subscribe("/arduino/4/BLUE/#");
 mqttClient.subscribe("/arduino/5/SWITCH/#");
 mqttClient.subscribe("/arduino/5/RED/#");
 mqttClient.subscribe("/arduino/5/GREEN/#");
 mqttClient.subscribe("/arduino/5/BLUE/#");
 mqttClient.subscribe("/arduino/6/SWITCH/#");
 mqttClient.subscribe("/arduino/6/RED/#");
 mqttClient.subscribe("/arduino/6/GREEN/#");
 mqttClient.subscribe("/arduino/6/BLUE/#");
 mqttClient.subscribe("/arduino/7/SWITCH/#");
 mqttClient.subscribe("/arduino/7/RED/#");
 mqttClient.subscribe("/arduino/7/GREEN/#");
 mqttClient.subscribe("/arduino/7/BLUE/#");
 mqttClient.subscribe("/arduino/8/SWITCH/#");
 mqttClient.subscribe("/arduino/8/RED/#");
 mqttClient.subscribe("/arduino/8/GREEN/#");
 mqttClient.subscribe("/arduino/8/BLUE/#");
 mqttClient.subscribe("/arduino/9/SWITCH/#");
 mqttClient.subscribe("/arduino/9/RED/#");
 mqttClient.subscribe("/arduino/9/GREEN/#");
 mqttClient.subscribe("/arduino/9/BLUE/#");
 */
 // Alternative Abonierung aller Topics unter /arduino/Nachtlicht
 mqttClient.subscribe("/arduino/#");
 Serial.println("[NETBRIDGE SUBSCRIBED]");
 
 
 }
 
 if (timer2 <= EndTimer2) 
   timer2++;
 else 
 {
   //Wird alle 5 Sek getriggert
   timer2 = 0;
   if(lanok == true)
   {
     digitalWrite(LEDred, LOW);
     digitalWrite(LEDgreen, HIGH);
   } else
   {
     digitalWrite(LEDred, !digitalRead(LEDred));
   }
   
 }

 for(int i=0;i<=9;++i)
 {
   if(on[i] != curon[i])
   {
     sendOnOff(i,on[i]);
   }
   
   if(curlatenz[i] != latenz[i])
   {
     sendLatenz(i,latenz[i]);
   }
   
   if(on[i])
   {
     if(rot[i] != currot[i] || blau[i] != curblau[i] || gruen[i] != curgruen[i])
     {
       sendColor(i,rot[i],gruen[i],blau[i]);
     }
   }
 }
 
 mqttClient.loop(); // Schleife für MQTT
 delay(25);
}
 
// ===========================================================
// Callback Funktion von MQTT. Die Funktion wird aufgerufen
// wenn ein Wert empfangen wurde.
// ===========================================================
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.println("[RECV LAN]");
 // Zähler
 int i = 0;
 // Hilfsvariablen für die Convertierung der Nachricht in ein String
 char message_buff[100];
 
 // Kopieren der Nachricht und erstellen eines Bytes mit abschließender \0
 for(i=0; i<length; i++) {
 message_buff[i] = payload[i];
 }
 message_buff[i] = '\0';
 
 
 // Konvertierung der nachricht in ein String
 String msgString = String(message_buff);
 int lichtid = String((String(topic)).charAt(9)).toInt();
 
 /*
 Serial.println(topic);
 Serial.println(msgString);
 */
 
 // Überprüfung des Topis und setzen der Farbe je nach übermittelten Topic
 if (String(topic).charAt(11) == 'S')
  { 
    //Switch On/Off Command
    if(msgString.toInt() == 1)
      on[lichtid] = true;
    else
      on[lichtid] = false;
  }
 if (String(topic).charAt(11) == 'L')
  { 
    latenz[lichtid] = round(msgString.toInt());
  }
 if (String(topic).charAt(11) == 'R')
  { 
    rot[lichtid] = round(msgString.toInt() * 2.55);
  }
 if ((String(topic)).charAt(11) == 'G')
  { 
    gruen[lichtid] = round(msgString.toInt() * 2.55);
  }
 if ((String(topic)).charAt(11) == 'B')
  {
    blau[lichtid] = round(msgString.toInt() * 2.55);
  }
}
