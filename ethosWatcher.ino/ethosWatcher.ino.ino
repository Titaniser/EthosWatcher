//#include <SPI.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#define OLED_RESET 4
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SPI.h>

const int GPIO12_green  = 12;
const int GPIO13_blue   = 13;
const int GPIO15_red    = 15;
const int GPIO16        = 16;
const int GPIO4         = 4;
const int blueLED       = 2;
const float threshold = 60.0;
const int calltime_ms = 10000;

// network credentials
char ssid[]     = "WLAN-987745";
char password[] = "8619417772260054";
char servername[] = "tombou.ethosdistro.com";
char rig[] = "49c27d";
int status = WL_IDLE_STATUS;
float oldhash = 0;

WiFiClient client;

int reboots = 0;
bool hashOK = 0;
bool minerHasBootedUp = 0;

void setup() {  
 // Initialize Serial port
  Serial.begin(9600);
  delay(200);
  Serial.println("");
  Serial.println("setup");

  connectWifi();
    
  // Initialize the LED_BUILTIN pin as an output
  pinMode(GPIO4, INPUT_PULLUP);  
  pinMode(GPIO12_green, OUTPUT);   
  pinMode(GPIO13_blue, OUTPUT);   
  pinMode(GPIO15_red, OUTPUT);   
  pinMode(GPIO16, OUTPUT);   
  digitalWrite(GPIO12_green, LOW);
  digitalWrite(GPIO13_blue, LOW);
  digitalWrite(GPIO15_red, LOW);
  digitalWrite(GPIO16, LOW);

  digitalWrite(GPIO12_green, HIGH);
  digitalWrite(GPIO13_blue, HIGH);
  digitalWrite(GPIO15_red, HIGH);
 
  delay(500);
  
  digitalWrite(GPIO12_green, LOW);
  digitalWrite(GPIO13_blue, LOW);
  digitalWrite(GPIO15_red, LOW);

  reboot();
}

void connectWifi(){
 // Connect to Wi-Fi network with SSID and password
  Serial.println("----------- Connecting to ");
  Serial.print("            ");
  Serial.print(ssid);
  status = WiFi.begin(ssid, password);
  int count =1;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (count < 30){
    Serial.print(".");
    count++;
    }else{
      Serial.print(".");
      status = WiFi.begin(ssid, password);
      count = 0;
    }
    Serial.print(".");
  }
   // Print local IP address and start web server
  Serial.print("           ");
  Serial.println(" WiFi connected.");
  Serial.println("            IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("           ");
  Serial.println("----------- Wifi connected ");
  Serial.println("           ");
}

float getEthosHash(){
  client.setTimeout(10000);
  if (!client.connect(servername, 80)) {
    Serial.println(F("Connection failed"));
    return -1;
  }

  // Send HTTP request
  client.println(F("GET /?json=yes HTTP/1.0"));
  client.println(F("Host: tombou.ethosdistro.com"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return -1;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return -1;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return -1;
  }
  float hash = parseJson();
  client.stop();
  return hash;
}
float parseJson(){
  // Allocate JsonBuffer
  // use http://arduinojson.org/assistant/ to allocate JsonBuffer
  const size_t bufferSize = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(34) + 2459;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  //const char* json = "{\"rigs\":{\"49c27d\":{\"condition\":\"autorebooted\",\"version\":\"1.2.7\",\"driver\":\"amdgpu\",\"miner\":\"claymore\",\"gpus\":\"6\",\"miner_instance\":\"6\",\"miner_hashes\":\"23.70 13.54 11.56 13.61 13.59 13.51\",\"bioses\":\"113-2E3471U.O63 113-34830M4-U02 113-34830M4-U02 113-34830M4-U02 113-34830M4-U02 113-34830M4-U02\",\"meminfo\":\"GPU0:02.00.0:Radeon RX 480:113-2E3471U.O63:Elpida EDW4032BABG:GDDR5:Polaris10\nGPU1:05.00.0:Radeon RX 560:113-34830M4-U02:Micron MT51J256M3:GDDR5:Polaris11\nGPU2:06.00.0:Radeon RX 560:113-34830M4-U02:Micron MT51J256M3:GDDR5:Polaris11\nGPU3:07.00.0:Radeon RX 560:113-34830M4-U02:Micron MT51J256M3:GDDR5:Polaris11\nGPU4:08.00.0:Radeon RX 560:113-34830M4-U02:Micron MT51J256M3:GDDR5:Polaris11\nGPU5:0a.00.0:Radeon RX 560:113-34830M4-U02:Micron MT51J256M3:GDDR5:Polaris11\",\"vramsize\":\"4 4 4 4 4 4\",\"drive_name\":\"Cruzer Fit 4C530002011022111010\",\"mobo\":\"H110 Pro BTC \",\"lan_chip\":\"Intel Corporation Ethernet Connection (2) I219-V (rev 31)\",\"connected_displays\":\"\",\"ram\":\"7\",\"rack_loc\":\"\",\"ip\":\"192.168.2.104\",\"server_time\":1519563031,\"uptime\":\"281\",\"miner_secs\":213,\"rx_kbps\":\"0.00\",\"tx_kbps\":\"0.00\",\"load\":\"0.24\",\"cpu_temp\":\"30\",\"freespace\":24.9,\"hash\":89.51,\"pool\":\"\",\"temp\":\"60.00 58.00 60.00 56.00 59.00 59.00\",\"powertune\":\"5 5 5 5 5 5\",\"voltage\":\"1.100 1.075 1.150 1.075 1.100 1.075\",\"watts\":null,\"fanrpm\":\"3282 3282 3282 3282 3282 3282\",\"core\":\"1250 1250 1250 1250 1250 1250\",\"mem\":\"1900 1900 1900 1900 1900 1900\"}},\"total_hash\":89.51,\"alive_gpus\":6,\"total_gpus\":6,\"alive_rigs\":1,\"total_rigs\":1,\"current_version\":\"1.2.9\",\"avg_temp\":58.67,\"capacity\":\"100.0\",\"per_info\":{\"claymore\":{\"hash\":90,\"per_alive_gpus\":6,\"per_total_gpus\":6,\"per_alive_rigs\":1,\"per_total_rigs\":1,\"per_hash-gpu\":\"15.0\",\"per_hash-rig\":\"90.0\",\"current_time\":1519563067}}}";
  //Serial.println("------ client string start -------");
  //Serial.println(client);
  //Serial.println("------ client string end   -------");
  JsonObject& root = jsonBuffer.parseObject(client);
  JsonObject& rigs = root["rigs"][rig];
  
  float rigs_hash = rigs["hash"]; // 89.51
  const char* rigs_temp = rigs["temp"]; // "60.00 58.00 60.00 56.00 59.00 59.00"
  //Serial.print("Hash: ");
  //Serial.println(rigs_hash);

  /*
  JsonObject& per_info_claymore = root["per_info"]["claymore"];
  long per_info_claymore_current_time = per_info_claymore["current_time"];
  Serial.print("current_time: ");
  Serial.println(per_info_claymore_current_time);*/
  
  //float total_hash = root["total_hash"]; // 89.51
  return rigs_hash;
}


void loop() {
  float ethosHash = getEthosHash();
  if ((float)ethosHash != (float)oldhash){
    Serial.println("");
    Serial.print("Hash: ");
    Serial.print(ethosHash);
    oldhash = ethosHash;
  } else{
    Serial.print(".");
  }
  
  //Serial.println("--loop start--");
  // checks the hash value to be over threshold at least once (booting)
  if (minerHasBootedUp == 0){
    if ((float)ethosHash >= (float)threshold){
      hashOK = true;
      minerHasBootedUp = 1;
    } 
    indicateNOK();
  }else{
    if ((float)ethosHash >= (float)threshold){
      hashOK = 1;
      indicateOK(); // fast green blinking
    } else{
      hashOK = 0;
      minerHasBootedUp = 0; //back to beginning
      reboot(); //blue is rebooting, red is indicating reboot counter
    }
  }
  
  //repeat after 10 seconds
  delay(calltime_ms);
  
}

void indicateOK(){
  //Serial.println("indicateOK");
  for (int i=0; i <= 10; i++){
    digitalWrite(GPIO12_green, HIGH);
    delay(50);
    digitalWrite(GPIO12_green, LOW);
    delay(50);
  }
}
void indicateNOK(){
  //Serial.println("indicateNOK");
  for (int i=0; i <= 5; i++){
    digitalWrite(GPIO12_green, HIGH);
    delay(50);
    digitalWrite(GPIO12_green, LOW);
    delay(50);
    digitalWrite(GPIO15_red, HIGH);
    delay(50);
    digitalWrite(GPIO15_red, LOW);
    delay(50);
  }
}

void reboot(){
  //send Reboot (LED) for 0.5 sec
  Serial.println("reset for 0.5 sec");
  Serial.println("blue led for 0.5 sec");
  digitalWrite(GPIO16, HIGH);
  for (int i=0; i <= 10; i++){
    digitalWrite(GPIO13_blue, HIGH);
    delay(20);
    digitalWrite(GPIO13_blue, LOW);
    delay(20);
  }
  digitalWrite(GPIO16, LOW);
  
  Serial.println("Miner has been resetted");
  
  delay(500);
  reboots++; //persistent ; reboot counter increase

  // indicates with blinking how often a reboot has been set
  for (int i=0; i <= reboots; i++){
    digitalWrite(GPIO15_red, HIGH);
    delay(500);
    digitalWrite(GPIO15_red, LOW);
    delay(500);
  }
  Serial.print("should blink red ");
  Serial.print(reboots);
  Serial.println(" times");
  
  Serial.println("----------------");
}

