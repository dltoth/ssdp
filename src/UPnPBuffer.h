
/**
 * 
 */

#ifndef UPNPBUFFER_H
#define UPNPBUFFER_H

#include <Arduino.h>
#include <ctype.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  
class UPnPBuffer {
  public:
  UPnPBuffer(const char* buff);          // Construct with with null terminated packet buffer

//  Return false if header does not exist, otherwise return true with header value filled in buffer
    boolean headerValue(const char* cheader, char buffer[], size_t len); 
    boolean headerValue_P(PGM_P header, char buffer[], size_t len); 
    
    boolean displayName(char buffer[], size_t len); // Return true if DESC header is present and fill buffer with the :name: value                       
    
    boolean isSearchRequest();                      // Return true if this message is a Search Request
    boolean isSearchResponse();                     // Return true if this message is a Search Response

/** Line processing
 *  
 */
    int         maxLineLength();
    const char* getNextLine(const char* lineStart, char buffer[], size_t bufferLen);
    boolean     hasNextLine(const char* startLine);
                                             
  private:
    const char*   _buffer;
    int           _maxLen = 0;

    int           maxLen();

};

} // End of namespace lsc

#endif
