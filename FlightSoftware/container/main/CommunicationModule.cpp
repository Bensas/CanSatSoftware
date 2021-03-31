#include "CommunicationModule.h"

int CommunicationModule::receivePackets(XBee xbee, ZBRxResponse responseObj, ZBTxStatusResponse requestStatusObj) {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {    
      xbee.getResponse().getZBRxResponse(responseObj);
      if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        return ZB_RX_RESPONSE;
      } else {
        // we got it (obviously) but sender didn't get an ACK
      }
    } else if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(requestStatusObj);
      return ZB_TX_STATUS_RESPONSE;
    } else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atCommandResponse);
      return AT_COMMAND_RESPONSE;
    }
  } else if (xbee.getResponse().isError()) {
    Serial.println("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  }
  return false;
}