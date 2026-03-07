#include "src\mqtt.h"
#include "src\models.h"

// === WiFi Config  =====
const char* ssid       = "HUAWEI-B525-60FD";
const char* password   = "SFR-4G-SURF";
int wifiStatus = WL_IDLE_STATUS;

// === MQTT Config  =====
const char* mqttServer = "PI5-IOT-EDGE-U25.local" ; // 192.168.8.128";
const int   mqttPort   = 1883;

// === Publish Topics 
const char* mqttSensorsDataTopic  = "greenhouse/mcu/sensors_data";
const char* mqttCmdDataTopic  = "greenhouse/mcu/cmd_data";

// === Subscribe Topics 
const char* mqttSubscribeTopics[] = {
  "greenhouse/mcu/state",
  "greenhouse/mcu/cmd",
  "greenhouse/mcu/force",
};
enum {
  TOPIC_STATE,
  TOPIC_CMD, 
  TOPIC_FORCE,
};
const int mqttSubscribeTopicCount = sizeof(mqttSubscribeTopics) / sizeof(mqttSubscribeTopics[0]);

// ---Will Payload 
const char* mqttStateTopic    = "greenhouse/mcu/state";
const char* mqttWillPayloadOffline     = "offline";
const char* mqttWillPayloadOnline      = "online";

QueueHandle_t mqttMsgQueue;

McuSystemState mcuState = MCU_WIFI_CONNECTING;
// char mqttTmpBuffer[128];  //  temporary mqttTmpBuffer

// ===== OBJETS =====
WiFiClient espClient;
PubSubClient client(espClient);

// ===== Functions  =====
void initMqtt() {

 WiFi.mode(WIFI_STA);

 mqttMsgQueue = xQueueCreate(3, sizeof(MqttMessage_t));  // 3 messagess in buffer
 if (mqttMsgQueue == NULL) {
    Serial.println("Failed to create RTOS MQTT queue!");
    while (1);  //  Stop !! 
 }

}

// ===  MQTT Client callBack ===
void onMessageReceived(char* topic, byte* payload, unsigned int length) {

  char aMqttTmpBuffer[128];
  MqttMessage_t msg;

  // Secure copy to msg instance
  strncpy(msg.topic, topic, MQTT_MAX_TOPIC_LEN - 1);
  msg.topic[MQTT_MAX_TOPIC_LEN - 1] = '\0';
  msg.length = (length < MQTT_MAX_PAYLOAD_LEN) ? length : MQTT_MAX_PAYLOAD_LEN;
  memcpy(msg.payload, payload, msg.length);
  msg.payload[msg.length] = '\0';
  msg.timestamp = millis();

  logfTask("📥 onMessageReceived() Push message into xQueue at : [%lu] ", (unsigned long)msg.timestamp ) ;  

  // Envoi non bloquant dans la queue
  if (xQueueSend(mqttMsgQueue, &msg, 0) != pdPASS) {
    logfTask("⚠ MQTT queue full, message dropped." ) ;  
  }
}

// ===== CONNEXION WIFI =====
void connectWiFi() {
  
  if (WiFi.status() == WL_CONNECTED)
    return;
  
  logfTask("Attempting to connect to SSID: [%s]", ssid) ;  

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    logfTask("Waiting ...");
  }
  logfTask("🟢 Connected using IP:[%s]  rssi: [%d]", WiFi.localIP().toString().c_str(), WiFi.RSSI());
}

void processMqttQueueMessage() 
{
  MqttMessage_t msg;

  if (xQueueReceive(mqttMsgQueue, &msg, 0) != pdPASS)
    return;

  // Sécurisation payload
  char payloadBuffer[msg.length + 1];
  memcpy(payloadBuffer, msg.payload, msg.length);
  payloadBuffer[msg.length] = '\0';

  logfTask("Process Queue Msg [%lu] topic [%s] payload [%s].",(unsigned long)msg.timestamp, msg.topic,  payloadBuffer);

  // ================================
  // MQTT WILL STATE
  // ================================
  //if (strcmp(msg.topic, mqttStateTopic) == 0)
  if (strcmp(msg.topic, mqttSubscribeTopics[TOPIC_STATE]) == 0) 
  {
    if (strcmp(payloadBuffer, mqttWillPayloadOffline) == 0)
    {
      bool ok = client.publish(mqttStateTopic, mqttWillPayloadOnline, true);
      if (!ok) {
        logfTask("⚠ Publish failed. Retry MQTT connect.");
        mcuState = MCU_MQTT_CONNECTING;
      }
    }
  }

  // ================================
  // MQTT CMD 
  // ================================
  if (strcmp(msg.topic, mqttSubscribeTopics[TOPIC_CMD]) == 0) 
  {

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payloadBuffer);

    if (error)
    {
      logfTask("⚠ JSON parse error");
        return;
    }

    if (doc["samplingSensors"].is<int16_t>())
    {
      cmdModel.samplingSensors = doc["samplingSensors"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }
    if (doc["temperatureMaxi"].is<int16_t>())
    {
      cmdModel.temperatureMaxi = doc["temperatureMaxi"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }
    if (doc["temperatureMini"].is<int16_t>())
    {
      cmdModel.temperatureMini = doc["temperatureMini"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }

    if (doc["hygrometryMaxi"].is<int16_t>())
    {
      cmdModel.hygrometryMaxi = doc["hygrometryMaxi"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }

    if (doc["hygrometryMini"].is<int16_t>())
    {
      cmdModel.hygrometryMini = doc["hugrometryMini"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }

    if (doc["ambientLightMaxi"].is<int16_t>())
    {
      cmdModel.ambientLightMaxi = doc["ambientLightMaxi"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }
    if (doc["ambientLightMini"].is<int16_t>())
    {
      cmdModel.ambientLightMini = doc["ambientLightMini"].as<int16_t>();
      cmdModel.modelDirty = true ; 
    }

    logfTask("cmdModel %s", ( cmdModel.modelDirty ? "updated." : "⚠  no update." ));

  } 
  // ================================
  // CMD FORCAGE GPIO
  // ================================
  else if (strcmp(msg.topic, mqttSubscribeTopics[TOPIC_FORCE]) == 0) 
  {
    StaticJsonDocument<64> doc;
    DeserializationError err = deserializeJson(doc, payloadBuffer);
    if (err) {
      logfTask("⚠ JSON parse error");
      return;
    }

    uint8_t gpio  = doc["gpio"].as<uint8_t>();
    bool force    = doc.containsKey("force") ? doc["force"].as<bool>() : false;
    uint8_t value = doc.containsKey("value") ? doc["value"].as<uint8_t>() : 0;

    const uint8_t allowedGPIOs[] = {3,4,5,6,7,35};
    bool valid = false;

    for (uint8_t i = 0; i < sizeof(allowedGPIOs)/sizeof(allowedGPIOs[0]); i++)
    {
      if (gpio == allowedGPIOs[i]) {
        valid = true;
        break;
      }
    }

    if (!valid) {
      logfTask( "⚠ GPIO: [%i] not allowed ", gpio);
      return;
    }

    // Mapping PWM Heater  
    if (gpio == 7 ) 
    {
      applyForcePWMGPIO(gpio, force,value);
      logfTask("Force PWM Enabled:[0x%02X] Value:[%d]", cmdModel.pwm_force_enable, cmdModel.pwm_force_value[ (int)gpio]);
    
    } else 
    {
      if (gpio == 35 )  // Board_led linked to mask 1  
       gpio = 1;

      applyForceDigitalGPIO(gpio, force, value);
      logfTask("Force Enabled:[0x%02X] Value:[0x%02X]",  cmdModel.digital_force_enable, cmdModel.digital_force_value);
    }
    cmdModel.modelDirty = true ; 
  }
}

void applyForceDigitalGPIO(uint8_t gpio, bool force, uint8_t value)
{
    // sécurité : GPIO autorisées 0-7 pour un octet (adapter si plus)
    if (gpio > 7) return;

    uint8_t mask = (1 << gpio);

    if (force)
    {
        // active le forçage pour ce bit
        cmdModel.digital_force_enable |= mask;

        // applique la valeur forcée
        if (value)
            cmdModel.digital_force_value |= mask;  // met le bit à 1
        else
            cmdModel.digital_force_value &= ~mask; // met le bit à 0
    }
    else
    {
        // désactive le forçage pour ce bit
        cmdModel.digital_force_enable &= ~mask;
        // efface la valeur forcée (optionnel)
        cmdModel.digital_force_value &= ~mask;
    }
   
}

void applyForcePWMGPIO(uint8_t gpio, bool force, uint8_t value)
{
    // sécurité : GPIO autorisées 0-7 pour un octet (adapter si plus)
    if (gpio > 7) return;

    uint8_t mask = (1 << gpio);

    if (force)
    {
        // active le forçage pour ce bit
        cmdModel.pwm_force_enable |= mask;

        // applique la valeur forcée
        cmdModel.pwm_force_value[(int) gpio] = value ;
    }
    else
    {
        // désactive le forçage pour ce bit
        cmdModel.pwm_force_enable &= ~mask;
        // efface la valeur forcée (optionnel)
        cmdModel.pwm_force_value[(int) gpio] = 0 ; 
    }
   
}

void startMqttTask(){
  xTaskCreatePinnedToCore( taskMqtt,"WiFi MQTT", 4096, NULL, PRIORITY_MEDIUM,   NULL, CORE_1 );
}

// =========================================================
//   -- RTOS Task -- 
//   Connect to Wifi
//   Connect to MQTT Broker
//   Subcribe CMD Topic Commands and update Data Model  
//   Publish data models to supervision  
//   Manage incomming supervision commands     
// =========================================================
void taskMqtt(void *pvParameters)
{
  client.setServer(mqttServer, mqttPort);
  client.setBufferSize(512);  // increase Payload size > 230 bytes 
  client.setCallback(onMessageReceived);

  vTaskDelay(pdMS_TO_TICKS(1000));
  logfTask("▶️ started.");
  unsigned long  previousMillis = millis()  ; 

  while (true)
  {
    unsigned long currentMillis = millis();
    // =======================
    // STATE FLOW  
    // =======================
    switch (mcuState)
    {
      case MCU_WIFI_CONNECTING:
      {
        if (WiFi.status() != WL_CONNECTED)
          connectWiFi();
        else
          {
            logfTask("🟢 WiFi connected.");
            mcuState = MCU_MQTT_CONNECTING;
          }
        break;
      }

      case MCU_MQTT_CONNECTING:
      {
        logfTask("Attempting to connect to  [%s] MQTT broker." , mqttServer  );

        String clientId = "HeltecV3-Client-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str(), nullptr, nullptr, mqttStateTopic, 1, true, mqttWillPayloadOffline))
        {
          logfTask("🟢 MQTT Connected clientID: [%s]." ,  clientId.c_str()  );
          mcuState = MCU_MQTT_CONNECTED;
        }
        else
        {
          logfTask("🟡 Error MQTT rc: [%i]  retry in 1 Mn", client.state() );
          vTaskDelay(pdMS_TO_TICKS(60000));
        }
        break;
      }

      case MCU_MQTT_CONNECTED:
      {

        //const char* topics[] = { mqttCmdTopic, mqttStateTopic, mqttForceTopic };
        for (int i = 0; i < mqttSubscribeTopicCount; i++) {
          logfTask("Subscribe topic: [%s].", mqttSubscribeTopics[i]);
          client.subscribe(mqttSubscribeTopics[i]);
        }
             
        vTaskDelay(pdMS_TO_TICKS(1000));
        mcuState = MCU_RUNNING;
        break;
      }

      case MCU_RUNNING:
      {
        if (WiFi.status() != WL_CONNECTED)
        {
          logfTask("🔴 WiFi lost.");
          mcuState = MCU_WIFI_CONNECTING;
          break;
        }

        if (!client.connected())
        {
          logfTask("🔴 MQTT lost.");
          mcuState = MCU_MQTT_CONNECTING;
          break;
        }

        if (cmdModel.modelDirty)
        {
          cmdModel.rssi = WiFi.RSSI() ; 
          String aJson = cmdModelToJson(&cmdModel);
          client.publish(mqttCmdDataTopic, aJson.c_str());  
          logfTask("Publish new data on topic: [%s] ",mqttCmdDataTopic );
          cmdModel.modelDirty = false ; 
        }
                
        if(sensorsModel.modelDirty)
        {
          String json = sensorsModelToJson(&sensorsModel);
          client.publish(mqttSensorsDataTopic, json.c_str());  
          logfTask("Publish new data on topic: %s",mqttSensorsDataTopic );
          sensorsModel.modelDirty = false ; 
        }

        if (currentMillis - previousMillis >= 60000 ) 
        {
          sensorsModel.modelDirty = true; 
          cmdModel.modelDirty = true  ;
          previousMillis = currentMillis;
        }
        processMqttQueueMessage(); 
        break;
      }
    }

    // =======================
    // ⚡ client.loop() UNIQUE
    // =======================
    client.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
