# SSDP
SSDP is an abbreviated version of [UPnP SSDP](http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf) that provides just enough information to populate a UPnP device hierarchy (root, embedded devices, and Services) and allow query for device availability. The code is intended for Arduino devices ESP8266 and ESP32. This library requires an additional [UPnPDevice library](https://github.com/dltoth/UPnPDevice/) for device structure. 
<h2>Description</h2>
<br>The protocol implemented here is not strictly SSDP, but rather an abbreviated version of the protocol with four main goals: 
<ol>
  <li> Reduce chattiness of standard UPnP/SSDP by only responding to known search requests</li> 
  <li>Provide enough information to populate a device hierarchy of the environment</li>
  <li>Allow query to see if root devices are still available on the network and</li>
  <li>Find instances of a specific Device (or Service) type on the network</li>
</ol>
<br>
<p>SSDP is chatty and could easily consume a small device responding to unnecessary requests. To this end a custom Search Target header, ST.LEELANAUSOFTWARECO.COM,is added. Search requests without this header are silently ignored. This abreviated protocol does not advertise on startup or shutdown, thus avoiding a flurry of unnecessary UPnP activiy. Devices respond ONLY to specific queries, and ignore all other SSDP requests.</p>
<p>In order to succinctly describe device hierarchy, a custom response header, DESC.LEELANAUSOFTWARECO.COM, is added. Search responses without this header are ignored. The DESC header includes a custom field descriptor, puuid, which refers to the parent uuid of a given UPnPDevice (or UPnPService). In this implementation of UPnP, RootDevices can have UPnPServices and UPnPDevices, and UPnPDevices can only have UPnPServices. The maximum number of embedded devices (or services) is restricted 8, thus limiting the device hierarchy. The DESC header field can implicitly refer to a either a RootDevice, an embedded UPnPDevice, or a UPnPService. When coupled with the Unique Service Name (USN), a complete device description in context is given. For example, for a UPnPDevice with uuid <i>device-UUID</i> and type <i>deviceType</i>:</P>
<pre>
USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver
DESC.LEELANAUSOFTWARECO.COM: name:displayName:devices:num-devices:services:num-services
</pre>
<p>will describe a RootDevice with <i>num-devices</i> embedded UPnPDevices, <i>num-services</i> UPnPServices, and whose display name is set to <i>displayName</i> and</p>
<pre>
USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver
DESC.LEELANAUSOFTWARECO.COM: name:displayName:services:num-services:puuid:parent-uuid
</pre>
<p>will describe an embedded UPnPDevice with <i>num-services</i> UPnPServices, whose display name is set to <i>displayName</i> and whose RootDevice uuid is <i>parent-uuid</i>. So the combinition of <i>device-UUID</i> and <i>parent-uuid</i> can uniquely describe the device hierarchy.</p>
<p>Another important difference between this variant of SSDP and standard UPnP/SSDP is that the LOCATION header provides a URL of an HTML UI for a UPnPDevice (or RootDevice), or service interface for a UPnPService, rather than device description. For example:</p>
<pre>
LOCATION: http://10.0.0.165:80/rootDeviceTarget/
</pre>
<p>for the HTML UI of a RootDevice whose target is set to <i>rootDeviceTarget</i>, or </p>
<pre>
LOCATION: http://10.0.0.165:80/rootDeviceTarget/embeddedDeviceTarget
</pre>
<p>for the HTML UI of a UPnPDevice whose target is set to <i>embeddedDeviceTarget</i>.</p>
<p>A more comprehensive description of the protocol is given in the companion SSDP User Guide document.</p>
<h2>Simple Example</h2>
What follows below is an Arduino sketch example for ESP8266<br>
<code><pre>
#include "ssdp.h"
using namespace lsc;

#define AP_SSID "MySSID"
#define AP_PSK  "MyPSK"
#define SERVER_PORT 80

#include <ESP8266WiFi.h>
ESP8266WebServer  server(SERVER_PORT);
#define           BOARD "ESP8266"

//
//   WebContext is a Web Server abstraction framework with implementations for both ESP8266 and ESP32.
//
WebContext       ctx;

RootDevice       root;
UPnPDevice       d;
UPnPService      s;
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

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());

//
//   Start the Web Server and initialize WebContext
//
  server.begin();
  ctx.setup(&server,WiFi.localIP(),SERVER_PORT);
  Serial.printf("Web Server started on %s:%d/\n\n",ctx.getLocalIPAddress().toString().c_str(),ctx.getLocalPort());
  
//
//   Build the device heirarchy
//
  d.addService(&s);
  d.setDisplayName("Device 1");
  d.setTarget("embededDevice");
  s.setDisplayName("Service 1");
  s.setTarget("service1");
  root.setDisplayName("Device Test");
  root.setTarget("device");  
  root.addDevice(&d);
  root.setup(&ctx);
  
  RootDevice::printInfo(&root);  
}

void loop() {
   ssdp.doSSDP();            // Handle SSDP queries
   server.handleClient();    // Handle HTTP requests for the RootDevice
   root.doDevice();          // Do unit of work for device
}
</pre></code>
<p>Notice that the RootDevice display name is set to <i>Device Test</i> and target is set to <i>device</i>, the embedded UPnPDevice display name is set to <i>Device 1</i> and target is set to <i>embeddedDevice</i>, and the UPnPService display name is set to <i>Service 1</i> where target is set to <i>service1</i>.</p>
<p>Output from the Serial port will be:</p><br>
<pre>

Starting UPnPDevice Test for Board ESP8266
Connecting to Access Point My_SSID
...........
WiFi Connected to My_SSID with IP address: 10.0.0.165
Web Server started on 10.0.0.165:80/

Device Test:
   UUID: b2234c12-417f-4e3c-b5d6-4d418143e85d
   Type: urn:LeelanauSoftwareCo-com:device:RootDevice:1
   Location is http://10.0.0.165:80/device
   Device Test Services:
Device 1:
   UUID: 1fda2c59-0a8e-4355-bebd-68e3af78cbeb
   Type: urn:LeelanauSoftwareCo-com:device:Basic:1
   Location is http://10.0.0.165:80/device/embededDevice
   Device 1 Services:
      Service 1:
         Type: urn:LeelanauSoftwareCo-com:service:Basic:1
         Location is http://10.0.0.165:80/device/embededDevice/service1
</pre>
