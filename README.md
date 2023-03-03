# SSDP #
The most common way to find ESP devices on a local network is to use mDNS and give each device a hard coded host name. This means that device developers  have to keep track all device names on the local network and avoid naming conflicts. Simple Service Discovery Protocol (SSDP) is part of the Universal Plug and Play (UPnP) standard, and provides a means to find devices on a local network automatically without mDNS. One device can be named and all others can be discovered.
 
This SSDP library is an abbreviated version of [UPnP SSDP](http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf) that provides just enough information to populate a UPnP device hierarchy (root, embedded devices, and Services) and allow query for device availability. The code is intended for Arduino devices ESP8266 and ESP32. This library requires the additional [UPnPDevice library](https://github.com/dltoth/UPnPDevice/) for device structure, which in turn requires the [CommonUtil library](https://github.com/dltoth/CommonUtil/) for device user interface.

## Description ##

The protocol implemented here is not strictly SSDP, but rather an abbreviated version of the protocol with four main goals: 

1. Reduce chattiness of standard UPnP/SSDP by only responding to known search requests 
2. Provide enough information to populate a device hierarchy of the environment
3. Allow query to see if root devices are still available on the network and
4. Find instances of a specific Device (or Service) type on the network

SSDP is chatty and could easily consume a small device responding to unnecessary requests. To this end a custom Search Target header, ST.LEELANAUSOFTWARE.COM, is added. Search requests without this header are silently ignored. This abreviated protocol does not advertise on startup or shutdown, thus avoiding a flurry of unnecessary UPnP activiy. Devices respond ONLY to specific queries, and ignore all other SSDP requests.

In order to succinctly describe device hierarchy, a custom response header, DESC.LEELANAUSOFTWARE.COM, is added. Search responses without this header are ignored. The DESC header includes a custom field descriptor, puuid, which refers to the parent uuid of a given UPnPDevice (or UPnPService). In this implementation of UPnP, RootDevices can have UPnPServices and UPnPDevices, and UPnPDevices can only have UPnPServices. The maximum number of embedded devices (or services) is restricted 8, thus limiting the device hierarchy. The DESC header field can implicitly refer to a either a RootDevice, an embedded UPnPDevice, or a UPnPService. When coupled with the Unique Service Name (USN), a complete device description in context is given. For example, for a UPnPDevice with uuid <i>device-UUID</i> and type <i>deviceType</i>:

```
USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver
DESC.LEELANAUSOFTWARECO.COM: name:displayName:devices:num-devices:services:num-services
```

will describe a RootDevice with <i>num-devices</i> embedded UPnPDevices, <i>num-services</i> UPnPServices, and whose display name is set to <i>displayName</i> and

```
USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver
DESC.LEELANAUSOFTWARECO.COM: name:displayName:services:num-services:puuid:parent-uuid
```

will describe an embedded UPnPDevice with <i>num-services</i> UPnPServices, whose display name is set to <i>displayName</i> and whose RootDevice uuid is <i>parent-uuid</i>. So the combinition of <i>device-UUID</i> and <i>parent-uuid</i> can uniquely describe the device hierarchy.

Another important difference between this variant of SSDP and standard UPnP/SSDP is that the LOCATION header provides a URL of an HTML UI for a UPnPDevice (or RootDevice), or service interface for a UPnPService, rather than device description. For example:

```
LOCATION: http://10.0.0.165:80/rootDeviceTarget/
```

for the HTML UI of a RootDevice whose target is set to *rootDeviceTarget*, or

```
LOCATION: http://10.0.0.165:80/rootDeviceTarget/embeddedDeviceTarget
```

for the HTML UI of a UPnPDevice whose target is set to *embeddedDeviceTarget*.

## Simple UPnPDevice Example ##
A Simple sketch of conisting of a RootDevice that responds to SSDP queries can be found in [examples/RootDevice](https://github.com/dltoth/ssdp/blob/main/examples/RootDevice/RootDevice.ino); the important parts are:

All UPnP and SSDP libraries use the namespace **lsc**

```
using namespace lsc;
```

WebContext is a WebServer abstraction for both ESP8266 and ESP32.

```
ESP8266WebServer server(SERVER_PORT);
WebContext       ctx;
RootDevice       root;
SSDP             ssdp;
```

Initialize WebContext and start the WebServer. 

```
  server.begin();
  ctx.setup(&server,WiFi.localIP(),SERVER_PORT);
```

Initialize the RootDevice with its display name, target, and WebContext, then print UPnP info to Serial.

```
  root.setDisplayName("Device Test");
  root.setTarget("device");  
  root.setup(&ctx);
  
  RootDevice::printInfo(&root);
```

Lastly, and most importantly, initialize SSDP with the RootDevice

```
  ssdp.begin(&root);
```

The main loop consists of three lines, one to handle SSDP queries, one to handle HTTP requests, and one to do any device specific work.

```
void loop() {
   ssdp.doSSDP();            // Handle SSDP queries
   server.handleClient();    // Handle HTTP requests for the RootDevice
   root.doDevice();          // Do unit of work for device
}
```
  
Output from the Serial port will be something like:

```
Starting SSDP for Board ESP8266
Connecting to Access Point My_SSID
...........
WiFi Connected to My_SSID with IP address: 10.0.0.165
Web Server started on 10.0.0.165:80/

SSDP Test:
   UUID: b2234c12-417f-4e3c-b5d6-4d418143e85d
   Type: urn:LeelanauSoftwareCo-com:device:RootDevice:1
   Location is http://10.0.0.165:80/device
   SSDP Test has no Services
SSDP Test has no Devices
```

The device is now on the network responding to SSDP queries. 

## Simple Query Example  ##
An Arduino sketch for ESP8266 that queries for RootDevices and prints selected headers to Serial can be found in [examples/SearchDevices](https://github.com/dltoth/ssdp/blob/main/examples/SearchDevices/SearchDevices.ino); the important parts are:

Again, the namespace is **lsc**

```
using namespace lsc;
```
Character buffers are defined to pass into a lambda function for the search request

```
  char  nameBuff[64];
  char* name = nameBuff;
  char  locBuff[64];
  char* loc = locBuff;
  char  usnBuff[128];
  char* usn = usnBuff;
  char  descBuff[128];
  char* desc = descBuff;
```

The Search Target (ST) is *upnp:rootdevice* and the lambda function fills display name, location, USN, and DESC into the buffers provided, and then prints them to Serial.

```
  SSDP::searchRequest("upnp:rootdevice",([name,loc,usn,desc](UPnPBuffer* b){
      name[0] = '\0';
      loc[0] = '\0';
      usn[0] = '\0';
      desc[0] = '\0';
      if( b->displayName(name,64) ) {
         b->headerValue("LOCATION",loc,64);
         b->headerValue("USN",usn,128);
         Serial.printf("   Device %s \n      USN: %s \n      LOCATION: %s\n",name,usn,loc);
      }  
  }),WiFi.localIP(),10000);

```

Now flash one device with [examples/RootDevice](https://github.com/dltoth/ssdp/blob/main/examples/RootDevice/RootDevice.ino) and power it up, but not from the Arduino IDE Serial port. Flash another device with [examples/SearchDevices](https://github.com/dltoth/ssdp/blob/main/examples/SearchDevices/SearchDevices.ino) and output from the Arduino IDE Serial port will look something like:

```
Starting SSDP Query Test for Board ESP8266
Connecting to Access Point My_SSID
...........
WiFi Connected to My_SSID with IP address: 10.0.0.165
Starting RootDevice search...
Root Device SSDP Test 
      USN: uuid:b2234c12-417f-4e3c-b5d6-4d418143e85d::urn:LeelanauSoftwareCo-com:device:RootDevice:1 
      LOCATION: http://10.0.0.165:80/
      DESC: :name:Device Test:devices:0:services:0:
...RootDevice search complete
```

Notice that in the USN reported above, the UUID and urn are the same that were reported in the RootDevice sketch.
In the search code above, the static SSDP::searchRequest method has the following form:

```
static SSDPResult searchRequest(const char* ST, SSDPHandler handler, IPAddress ifc, int timeout=2000, 
                                boolean ssdpAll=false);
```

where

```
ST - Search Target MUST be one of the following:
           upnp:rootdevice                          For RootDevice searches
           uuid:Device-UUID                         For example - uuid: b2234c12-417f-4e3c-b5d6-4d418143e85d
           urn:domain-name:device:deviceType:ver    For example - urn:LeelanauSoftwareCo-com:device:SoftwareClock:1
           urn:domain-name:service:serviceType:ver  For example - urn:LeelanauSoftwareCo-com:service:GetDateTime:1
handler - An SSDPHandler function called on each response to the request
ifc     - The network interface to bind the request to (either WiFi.localIP() or WiFi.softAPIP())
timeout - (Optional) Listen for responses for timeout milliseconds and then return to caller. If ST is 
          uuid:Devce-UUID, processing returns after the specific device responds or timeout expires, otherwise 
          processing returns after timeout milliseconds.
ssdpAll - (Optional) Applies only to upnp:rootdevice searches, if true, ALL RootDevices, embedded UPnPDevices, 
          and UPnPServices respond, otherwise only RootDevices respond.
```

and SSDP response Header names and values will be one of the following:

```
 CACHE-CONTROL:               - For example, max-age = 1800
 LOCATION:                    - Device or Service URL
 ST:                          - Search Target from search request
 USN: Unique Service Name     - uuid:device-UUID::urn:domain-name:device:deviceType:ver for a UPnPDevice or
                                uuid:device-UUID::urn:domain-name:service:serviceType:ver for a service
 DESC.LEELANAUSOFTWARECO.COM: - name:displayName:devices:num-devices:services:num-services for the RootDevice or
                                name:displayName:services:num-services:puuid:parent-uuid for an embedded device or
                                name:displayName:puuid:parent-uuid: for a UPnPService
```

The timeout parameter defaults to 2 seconds, which is most likely too slow so the example above uses 10. Also, SSDP uses UDP, which is inherently unreliable. If you don't see all of the devices you expect, either increase the timeout or re-run the query.

For an example of device search see the nearbyDevices method of [ExtendedDevice](https://github.com/dltoth/DeviceLib/blob/main/src/ExtendedDevice.cpp) in the [DeviceLib library](https://github.com/dltoth/DeviceLib/)

