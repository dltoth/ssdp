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
#include <SoftwareClock.h>
#include <Thermometer.h>
#include <ESP8266WiFi.h>

using namespace lsc;
#define AP_SSID "My_SSID"
#define AP_PSK  "MY_PSK"
#define BOARD "ESP8266"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.printf("Starting SSDP Query Test for Board %s\n",BOARD);

  WiFi.begin(AP_SSID,AP_PSK);
  Serial.printf("Connecting to Access Point %s\n",AP_SSID);
  while(WiFi.status() != WL_CONNECTED) {Serial.print(".");delay(500);}

  Serial.printf("\nWiFi Connected to %s with IP address: %s\n",WiFi.SSID().c_str(),
                                                           WiFi.localIP().toString().c_str());
  
  // Perform an SSDP search for RootDevices and print display name and location
  char  nameBuff[64];
  char* name = nameBuff;
  char  locBuff[64];
  char* loc = locBuff;
  char  usnBuff[128];
  char* usn = usnBuff;
  char  descBuff[128];
  char* desc = descBuff;

/*
 *     Search Target MUST be one of the following:
 *        upnp:rootdevice
 *        uuid:Device-UUID                              For example - uuid: b2234c12-417f-4e3c-b5d6-4d418143e85d
 *        urn:domain-name:device:deviceType:ver         For example - urn:LEELANAUSOFTWARE-com:device:SoftwareClock:1
 *        urn:domain-name:service:serviceType:ver       For example - urn:LEELANAUSOFTWARE-com:service:GetDateTime:1
 *     Alternatively, the static upnpType() method can be used from any class:
 *        ExtendedDevice::upnpType()
 *     or
 *        SoftwareClock::upnpType()
 *     Note that RootDevice::upnpType() will only return those devices whose RootDevice has not been subclassed, to get all
 *     RootDevices use upnp:rootdevice
 *
 *     Search Request method:   searchRequest(const char* ST, SSDPHandler handler, IPAddress ifc, int timeout, boolean ssdpAll=false) 
 */
  Serial.printf("Starting RootDevice search...\n");
  SSDP::searchRequest("upnp:rootdevice",([name,loc,usn,desc](UPnPBuffer* b){
      name[0] = '\0';
      loc[0]  = '\0';
      usn[0]  = '\0';
      desc[0] = '\0';
      if( b->displayName(name,64) ) {
          b->headerValue("LOCATION",loc,64);
          b->headerValue("USN",usn,128);
          b->headerValue("DESC.LEELANAUSOFTWARE.COM",desc,128);
          Serial.printf("   Root Device %s \n      USN: %s \n      LOCATION: %s\n      DESC: %s\n",name,usn,loc,desc);
      }  
  }),WiFi.localIP(),10000);
  Serial.printf("...RootDevice search complete\n");

}


void loop() {
}
