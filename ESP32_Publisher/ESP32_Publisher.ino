// MCP2515 Connection with ESP32
/*
 * MCP2515  = WEMOS ESP32
 * VCC      =       5V
 * GND      =      GND
 * CS       =       P5
 * SO       =      P19
 * SI       =      P23
 * SCK      =      P18
 * INT      =      P22
 */


//Libraries to include 
#include <WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <mcp_can.h>  // https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

long unsigned int rxId; // for CAN Packet ID
unsigned char len = 0; // Length of CAN Packet
unsigned char rxBuf[8]; // Default Buffer Size


#define CS 5  // Define CS PIN
MCP_CAN CAN0(CS);  // Initialize MCP_CAN                              

//WiFi Connection
const char* ssid = "SecureTechware2"; 
const char* password = "$7001123";
//MQTT Server Connection
#define mqtt_server "broker.hivemq.com"
#define owner "ArsalanSaleemSecureTechwarePublisher_3_0"
#define mqtt_user "ArsalanSaleemSecureTechwarePublisher_3_0"
#define mqtt_password "ArsalanSaleemSecureTechware_3_0"
#define status_topic "SecureTechware_MCP2515_Status_3_0"
#define data_topic "SecureTechware_MCP2515_Data_3_0"



WiFiClient espClient;
PubSubClient client(espClient);

//Variable sent over MQTT Server
String SendData;



void mqttconnect() {   // Loop until reconnected 
  while (!client.connected()) {
    Serial.print("connecting to MQTT Server ...");
    /* connect now */
    if (client.connect( owner, mqtt_user, mqtt_password, status_topic, 2, true, "offline")) {
      client.publish(status_topic, "Publisher is Connected!", true);
      Serial.println("Publisher is Connected!");
    } else {
      Serial.print("Failed, status code =");
      Serial.print(client.state());
      Serial.println("Try again in 5sec ...");  // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200); //baud rate 

  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  
  pinMode(CS, OUTPUT);  
  // Initialize MCP2515 running at MCP_8MHZ or MCP_16MHZ with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(22, INPUT);                            // Setting pin 22 for /INT input
  
  Serial.println("ESP32 --> MCP2515 --> MQTT Initialization. Complete Publisher Successed.");
}

void loop()
{
    /* if MQTT client was disconnected then try to reconnect again */
    if (!client.connected()) {
          Serial.print("MQTT reconnecting ...");
          mqttconnect();
    } 

    String DATENSATZ;  //Temp Variable
    
    if(!digitalRead(22))                        // If pin 22 is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
     
      DATENSATZ+=String(rxId,HEX).c_str(); //Set ID
      DATENSATZ+=String("#").c_str();

      for(int i=0;i<len;i++){
        DATENSATZ+=String(rxBuf[i],HEX).c_str(); //Set CAN Frame
        DATENSATZ+=" ";
      }

      if(SendData!=DATENSATZ){
            SendData=DATENSATZ;
            Serial.println(SendData); 
            client.publish(data_topic, String(SendData).c_str(), true); // MQTT-Server send
      }
    }

   
   

}



