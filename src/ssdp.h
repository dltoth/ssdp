/**
 * ssdp.h
 *
 *  This protocol is not really SSDP, but rather an abbreviated version of the protocol with two main goals:
 *  (1) Provide enough information to populate a device hierarchy of the environment and
 *  (2) Query if devices are still available on the network
 *  Search requests are sent out over the multicast address 239.255.255.250 port 1900 and responses are sent to the unicast
 *  IP address and port of the request.
 * 
 *  Unique Service Name - USN is always uuid:device-UUID::urn:domain-name:device:deviceType:ver for a device or
 *                                      uuid:device-UUID::urn:domain-name:service:serviceType:ver:serviceID for a service
 *                                      where serviceID is a unique service identifier within it's device
 *  Unique Parent Name  - UPN.LEELANAUSOFTWARECO.COM is always the USN of the parent of an embedded device or
 *                                      upnp:rootdevice if the device is a root device
 *
 ** ST Search Target and USN Response values 
 *  
 *  (1)    M-SEARCH Request header:
 *         ST:  upnp:rootdevice        Responds once for each root device
 *         ST.LEELANAUSOFTWARECO.COM:  ssdp:all (or empty). If ssdp:all, responds for each embedded device and once for each service for each Root Device
 *                                     If empty, responds for each root device only. This is a required header
 *         
 *         Response Headers: 
 *         USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver for a device or
 *              uuid:device-UUID::urn:domain-name:service:serviceType:ver for a service
 *         UPN.LEELANAUSOFTWARECO.COM: uuid:device-UUID::urn:domain-name:device:deviceType:ver of parent device or 
 *                                     upnp:rootdevice if device is a root device
 *              
 *  (2)    M-SEARCH Request Header:
 *         ST:  uuid:root-device-UUID  Root Devices respond if uuid matches
 *         ST.LEELANAUSOFTWARECO.COM:  ssdp:all (or empty). If ssdp:all, responds for each embedded device and once for each service for this Root Device
 *                                     If empty, responds for root device only. This is a required header
 *         
 *         Response Headers:
 *         USN: uuid:device-UUID::urn:domain-name:device:deviceType:ver for a device or
 *              uuid:device-UUID::urn:domain-name:service:serviceType:ver for a service
 *         UPN.LEELANAUSOFTWARECO.COM: uuid:device-UUID::urn:domain-name:device:deviceType:ver of parent device or 
 *                                     upnp:rootdevice if device is a root device
 *         
 *   SSDP Device Response:
 *      HTTP/1.1 200 OK
 *      CACHE-CONTROL: max-age = 1800
 *      LOCATION: Device URL
 *      ST: 
 *      USN: device USN
 *      UPN.LEELANAUSOFTWARECO.COM: device UPN
 *   
 *   SSDP Service Response:
 *      HTTP/1.1 200 OK
 *      CACHE-CONTROL: max-age = 1800
 *      LOCATION: Service URL relative to the Device URL
 *      ST: 
 *      USN: service USN
 *      UPN.LEELANAUSOFTWARECO.COM: service UPN
 *   
 */
 
#ifndef SSDP_H
#define SSDP_H
#include <ctype.h>
#include "UPnPBuffer.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <UPnPDevice.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {

#define LOC_SIZE 32

#ifndef NULL
#define NULL 0
#endif

#ifndef EMPTY_STRING
#define EMPTY_STRING {""}
#endif

#define UDP_PORT   1900                // local UDP port to listen on

typedef enum {
  SSDP_OK = 0,
  SSDP_ERR_UDP = 1,
  SSDP_ERR_SEND = 2,
  SSDP_ERR_ST = 3
} SSDPResult;

typedef std::function<void(UPnPBuffer*)> SSDPHandler;

class SSDP {

  public:
  SSDP();
  
  void         begin(RootDevice* root);                  // RootDevice to handle search requests
  void         doSSDP();                                 // Read both Unicast and Multicast UDP channels and respond accordingly
  int          getUDPPort();                             // Return unicast UDP channel port
  int          getMulticastPort();                       // Return Multicast UDP channel port
  
  static boolean   isLocalIP(IPAddress addr);            // Return true if addr is on the localIP network
  static boolean   isSoftAPIP(IPAddress addr);           // Return true if addr is on the softAPIP network
  static IPAddress interfaceAddress(IPAddress addr);     // Return the network interface (either local or softAP) of addr

/**
 *  Send an SSDP Search request and parse responses for timeout milliseconds.
 *  Each response is handed to an SSDPHandler for processing.
 *  Input Parameters:
 *     ST      - Search Type MUST be one of the following:
 *                 upnp:rootdevice
 *                 uuid:Device-UUID
 *                 urn:domain-name:device:deviceType:ver         For example - urn:LeelanauSoftwareCo-com:device:SoftwareClock:1
 *                 urn:domain-name:service:serviceType:ver       For example - urn:LeelanauSoftwareCo-com:service:GetDateTime:1
 *     handler - An SSDPHandler function called on each response to the request
 *     ifc     - The network interface to bind the request to (either WiFi.localIP() or WiFi.softAPIP())
 *     timeout - Listen for responses for timeout milliseconds and then return to caller. If ST is uuid:Devce-UUID, processing returns
 *               after the specific device responds or timeout expires, otherwise processing returns after timeout milliseconds.
 *     ssdpAll - Applies only to upnp:rootdevice searches, if true, ALL RootDevices, embedded UPnPDevices, 
 *               and UPnPServices respond, otherwise only RootDevices respond.
 */
  static SSDPResult      searchRequest(const char* ST, SSDPHandler handler, IPAddress ifc, int timeout=2000, boolean ssdpAll=false);

/**
 *  Set/Get/Check Logging Level. Logging Level can be NONE, INFO, FINE, and FINEST
 */
  static void             logging(LoggingLevel level)             {_logging = level;}
  static LoggingLevel     logging()                               {return _logging;}
  static boolean          loggingLevel(LoggingLevel level)        {return(logging() >= level);}

  private:
  RootDevice*                _root;                      // RootDevice to expose through SSDP
  WiFiUDP                    _mUdp;                      // Multicast Discovery
  WiFiUDP                    _udp;                       // Unicast Discovery and resopnse
  static LoggingLevel        _logging;
  
  std::function<void(void)>  _postHandler = []{};


  void      doChannel(WiFiUDP& channel);                                                          // Check for incoming search requests and respond
  void      setPostHandler(std::function<void(void)> handler) {_postHandler = handler;}           // Set post response handler
  boolean   readChannel(WiFiUDP& channel);                                                        // Read bytes from channel, returns true if response required
  void      postAllResponse(UPnPDevice* d, const char* st, IPAddress remoteAddr, int port );      // post search response for all embedded devices and services
  void      postAllMatching(UPnPDevice* d, const char* st, IPAddress remoteAddr, int port );      // post search response for matching devices and services
  void      postAllReverse(UPnPDevice* d, const char* st, IPAddress remoteAddr, int port );       // post search all response in reverse
  void      postDeviceResponse(UPnPDevice* d, const char* st, IPAddress remoteAddr, int port );   // post search response for device, returns USN
  void      postServiceResponse(UPnPService* s, const char* st, IPAddress remoteAddr, int port ); // post search response for service

};

} // End of namespace lsc

#endif
