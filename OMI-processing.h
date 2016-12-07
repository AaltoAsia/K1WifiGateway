
#include "Config.h"

void omiHeader();
bool omiAddObject(const char * objectName) ;
bool omiAddInfoItem(const char * itemName, const char * valueStr) ;
bool omiCloseObject();
bool omiFooter();
bool createBridgeBootOMI() ;
bool createOMI(NodeStr * packetData, uint8_t len) ;

#ifdef OMI_CERT
bool trySend(HttpsCertificateClient& http) ;
#else
bool trySend(HTTPClient& http) ;
#endif
