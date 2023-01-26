# ssdp
Abbreviated version of UPnP SSDP that provides enough information to populate a UPnP device hierarchy (root, embedded devices, and Services) and allow query for device availability. This library requires the additional [UPnPDevice library](https://github.com/dltoth/UPnPDevice/) for device structure. 
<br>This protocol is not strictly SSDP, but rather an abbreviated version of the protocol with four main goals: (1) Reduce chattiness of standard UPnP/SSDP by only responding to known search requests, (2) Provide enough information to populate a device hierarchy of the environment, (3) Allow query to see if root devices are still available on the network, and (4) Find instances of a specific Device (or Service) type on the network.
<br>Search requests are sent out over the multicast address 239.255.255.250 port 1900 and responses are sent to the unicast IP address and port of the request. 
<br>SSDP is chatty and could easily consume a small device responding to unnecessary requests. To this end we add a custom Search Target header, ST.LEELANAUSOFTWARECO.COM described in the Protocol section below. Search requests without this header are silently ignored. This abreviated protocol does not advertise on startup or shutdown, thus avoiding a flurry of unnecessary UPnP activiy. Devices respond ONLY to specific queries, and ignore all other SSDP requests.
<br>In order to succinctly describe device hierarchy, we add a custom response header, DESC.LEELANAUSOFTWARECO.COM. In this implementation of UPnP, RootDevices can have UPnPServices and UPnPDevices, and UPnPDevices can only have UPnPServices. The maximum number of embedded  devices (or services) is restricted 8, thus limiting the device hierarchy. We also add a custom field descriptor, puuid, which refers to the parent uuid of a given UPnPDevice (or UPnPService). The DESC header field, defined below, can implicitly refer to a either a RootDevice, an embedded UPnPDevice, or a UPnPService. When coupled with the USN, a complete device description in context is given.

