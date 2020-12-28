#include "setup.h"
#include "setup_wifi.h"
#include "arduinoota_setup.h"
#include <PubSubClient.h>

/**************************************  GLOBALS **********************************/
WiFiClient espClient;
PubSubClient client(espClient);
HardwareSerial dataSerial(2);
const byte numChars = 32;
char sensorData[numChars]; // an array to store the received data
char cmd[numChars];
boolean newData = false;
int len;
int maxlen;
char topic[] = "rainsensor/cmds";

/****************************************************************************************/
void setup()
{
  Serial.begin(9600);
  dataSerial.begin(9600);                   // Used for communication with sensor
  setup_wifi();                             // initialize wifi
  client.setServer(MQTT_SERVER, MQTT_PORT); // initialize MQTT
  //client.setCallback(callback);
  setup_ota(); //initialize OTA
  ArduinoOTA.begin();
  pinMode(LED_BUILTIN, OUTPUT);             // Initialize the LED_BUILTIN pin as an output
  pinMode(SENSOR_PIN, INPUT_PULLUP);        // Declare sensor as input, pullup as we have a open drain sensor output
  Serial.println("Ready - setup complete"); //we're done
}

/****************************************************************************************/
void sendState()
{
  if (strlen(sensorData) == 4)
  {
    const char *r =  &sensorData[2];
    Serial.print("Numerical sensorData: ");
    Serial.println(sensorData[2]);
    Serial.print("r: ");
    Serial.println(r);
    char combinedArray[sizeof(MQTT_STATE_TOPIC_PREFIX) + sizeof(DEVICE_NAME)];
    sprintf(combinedArray, "%s%s", MQTT_STATE_TOPIC_PREFIX, DEVICE_NAME); // with word space
    client.publish(combinedArray, r, false);
  }
}

/****************************************************************************************/
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
      char combinedArray[sizeof(MQTT_STATE_TOPIC_PREFIX) + sizeof(DEVICE_NAME) + 4];
      sprintf(combinedArray, "%s%s/set", MQTT_STATE_TOPIC_PREFIX, DEVICE_NAME); // with word space
      client.subscribe(combinedArray);
      sendState();
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/****************************************************************************************/
void readSensorData()
{
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (dataSerial.available() > 0 && newData == false)
  {
    rc = dataSerial.read();

    if (rc != endMarker)
    {
      sensorData[ndx] = rc;
      ndx++;
      if (ndx >= numChars)
      {
        ndx = numChars - 1;
      }
    }
    else
    {
      sensorData[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void processSensorData()
{
  if (newData == true)
  {
    Serial.print("Sensor data received:  ");
    Serial.println(sensorData);
    newData = false;
  }
}
/****************************************************************************************/
void loop()
{

  if (WiFi.status() != WL_CONNECTED) //check wifi connectivity
  {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }
  if (!client.connected()) //check MQTT connection
  {
    reconnect();
  }

  client.loop();       // Check MQTT
  ArduinoOTA.handle(); // Check OTA Firmware Updates

  dataSerial.println("R"); //Push 'request update' to sensor

  readSensorData();
  processSensorData();
  sendState();

  len = strlen(sensorData);
  Serial.print("Length of sensorData is: ");
  Serial.println(len);
  delay(5000);
}