#include <ssdp.h>
#include <ESP8266WiFi.h>

using namespace lsc;
#define AP_SSID "My_SSID"
#define AP_PSK  "MYPSK"
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

  Serial.printf("Starting RootDevice search...\n");
  SSDP::searchRequest("upnp:rootdevice",([name,loc,usn,desc](UPnPBuffer* b){
      name[0] = '\0';
      loc[0]  = '\0';
      usn[0]  = '\0';
      desc[0] = '\0';
      if( b->displayName(name,64) ) {
         b->headerValue("LOCATION",loc,64);
         b->headerValue("USN",usn,128);
         b->headerValue("DESC.LEELANAUSOFTWARECO.COM",desc,128);
         Serial.printf("   Device %s \n      USN: %s \n      LOCATION: %s\n      DESC: %s\n",name,usn,loc,desc);
      }  
  }),WiFi.localIP(),10000);
  Serial.printf("...RootDevice search complete\n");
}

void loop() {
}
