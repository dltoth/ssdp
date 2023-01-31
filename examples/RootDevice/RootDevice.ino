#include <ssdp.h>
using namespace lsc;

#define AP_SSID "Dumbledore 1.0"
#define AP_PSK  "2badboys"
#define SERVER_PORT 80

#include <ESP8266WiFi.h>
ESP8266WebServer  server(SERVER_PORT);
#define           BOARD "ESP8266"

/**
 *   WebContext is a Web Server abstraction framework with implementations for both ESP8266 and 
 *   ESP32.
 */
WebContext       ctx;

RootDevice       root;
SSDP ssdp;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("Starting UPnPDevice Test for Board %s\n",BOARD);

  WiFi.begin(AP_SSID,AP_PSK);
  Serial.printf("Connecting to Access Point %s\n",AP_SSID);
  while(WiFi.status() != WL_CONNECTED) {Serial.print(".");delay(500);}

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),
                                                              WiFi.localIP().toString().c_str());

/**
 *   Start the Web Server and initialize WebContext
 */
  server.begin();
  ctx.setup(&server,WiFi.localIP(),SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n\n",ctx.getLocalIPAddress().toString().c_str(),
                                                   ctx.getLocalPort());
  
/**
 *   Set up the RootDevice
 */
  root.setDisplayName("Device Test");
  root.setTarget("device");  
  root.setup(&ctx);
  
  RootDevice::printInfo(&root);  
}

void loop() {
   ssdp.doSSDP();            // Handle SSDP queries
   server.handleClient();    // Handle HTTP requests for the RootDevice
   root.doDevice();          // Do unit of work for device
}
