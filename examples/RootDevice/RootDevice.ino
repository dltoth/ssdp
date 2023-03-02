/**
 * 
 *  ssdp Library
 *  Copyright (C) 2023  Daniel L Toth
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or any 
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 *  The author can be contacted at dan@leelanausoftware.com  
 *
 */


#include <ssdp.h>
using namespace lsc;

#define AP_SSID "My_SSID"
#define AP_PSK  "MY_PSK"
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
  Serial.printf("Starting SSDP for Board %s\n",BOARD);

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
  root.setDisplayName("SSDP Test");
  root.setTarget("device");  
  root.setup(&ctx);
  
  ssdp.begin(&root);
  
  RootDevice::printInfo(&root);  
}

void loop() {
   ssdp.doSSDP();            // Handle SSDP queries
   server.handleClient();    // Handle HTTP requests for the RootDevice
   root.doDevice();          // Do unit of work for device
}
