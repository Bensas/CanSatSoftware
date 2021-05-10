#include "CommunicationModule.h"

int CommunicationModule::receivePackets(XBee& xbee, ZBRxResponse& responseObj, ZBTxStatusResponse& requestStatusObj) {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    Serial.write("Got XBEE Response:");
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {    
      xbee.getResponse().getZBRxResponse(responseObj);
      Serial.write("ZB_RX_RESPONSE");
      if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        return ZB_RX_RESPONSE;
      } else {  
        // we got it (obviously) but sender didn't get an ACK
      }
    } else if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(requestStatusObj);
      Serial.write("ZB_TX_STATUS_RESPONSE");
      return ZB_TX_STATUS_RESPONSE;
    } else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      Serial.write("AT_COMMAND_RESPONSE");
      xbee.getResponse().getAtCommandResponse(atCommandResponse);
      return AT_COMMAND_RESPONSE;
    }
  } else if (xbee.getResponse().isError()) {
//    Serial.write("Error reading packet.  Error code: ");  
//    Serial.write(xbee.getResponse().getErrorCode());
  } else {
    Serial.write("No response from XBEE :(");
  }
    
 
  return false;
}
