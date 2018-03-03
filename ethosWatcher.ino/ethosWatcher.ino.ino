//#include <SPI.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#define OLED_RESET 4
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
extern "C" {
  #include "user_interface.h"
}

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
char wiFiHostname[ ] = "ethoswatcher";

char rig[] = "49c27d";
int status = WL_IDLE_STATUS;
float oldhash = 0;

WiFiClient client;

int reboots = 0;
bool hashOK = 0;
bool minerHasBootedUp = 0;
float ethosHash_web = 0;


//webserver----------------------------------
// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//int alle_x_sekunden=10;

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

  //Timer1.initialize(alle_x_sekunden*1000000);
  //Timer1.attachInterrupt(doLoop);
}

void connectWifi(){
 // Connect to Wi-Fi network with SSID and password
  Serial.println("----------- Connecting to ");
  Serial.print("            ");
  Serial.print(ssid);
  status = WiFi.begin(ssid, password);
  
  wifi_station_set_auto_connect(true);
  wifi_station_set_hostname(wiFiHostname);
  
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

void doLoop(){
  float ethosHash = getEthosHash();
  ethosHash_web = ethosHash;
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
}

void loop() {
  //doLoop();
  /*float ethosHash = getEthosHash();
  ethosHash_web = ethosHash;
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
  */
  webserver();
}

void webserver(){
  //http://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/
  //Serial.println(".");
  //Serial.println("..");
  //Serial.println("...");
  //Serial.println("....");
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
  Serial.println("found");
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
/*            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
*/          
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css\">");

            client.println("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js\"></script>");

            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.print("<p>threshold: ");
            client.print(threshold);
            client.println("</p>");
            
            client.print("<p>rig: ");
            client.print(rig);
            client.println("</p>");
            
            client.print("<p>ssid: ");
            client.print(ssid );
            client.println("</p>");
            
            client.print("<p>servername: ");
            client.print(servername);
            client.println("</p>");
            
            client.print("<p>reboots: ");
            client.print(reboots);
            client.println("</p>");
            
            client.print("<p>hashOK: ");
            client.print(hashOK);
            client.println("</p>");

            client.print("<p>minerHasBootedUp: ");
            client.print(minerHasBootedUp);
            client.println("</p>");
            
            client.print("<p>ethosHash: ");
            client.print(ethosHash_web);
            client.println("</p>");
            
            
            /*
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            */
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
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

