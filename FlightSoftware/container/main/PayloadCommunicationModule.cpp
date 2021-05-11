#include "PayloadCommunicationModule.h"

void PayloadCommunicationModule::init(XBee& xbeeDevice) {
  xbee = xbeeDevice;
}

void PayloadCommunicationModule::parseReceivedPacket(uint8_t* packetData, uint8_t packetLength) {
  if (packetData[0] == 'C') {
    parseCommandPacket(packetData, packetLength);
  }
}

void PayloadCommunicationModule::parseCommandPacket(uint8_t* packetData, uint8_t packetLength) {
  /*
    Examples:
    0 - Turn off teemetry
    1 - Turn on telemetry
  */
  if (packetData[0] == '0') { 
    setPayloadTelemetryActivated(packetData[0] == '1');
  }
}


void PayloadCommunicationModule::sendNextTelemetryPacket(){
  containerRequestObj = ZBTxRequest(containerAddress, telemetryPacketQueue.head->data, telemetryPacketQueue.head->dataLength);
  xbee.send(containerRequestObj);
}

void PayloadCommunicationModule::loop() {
  int containerReceiveStatus = receivePackets(xbee, containerResponseObj, containerRequestStatusObj);
  if (containerReceiveStatus == ZB_RX_RESPONSE) { // We received a message so we parse it and act on it.
    uint8_t* packetData = containerResponseObj.getData();
    uint8_t dataLength = containerResponseObj.getDataLength();
    parseReceivedPacket(packetData, dataLength);
    // if (containerResponseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
    //   //console.log('ST command package received and acknowledged');
    // } else {
    //   //console.log('ST command package received but sender didnt get an ack');
    // }
  } else if (containerReceiveStatus == ZB_TX_STATUS_RESPONSE) { // We received a status update on a previously sent packet
    if (containerRequestStatusObj.getDeliveryStatus() == SUCCESS) { // We got an ACK Wohoo!
      telemetryPacketQueue.removeHead(); 
      containerCommunicationState = STATE_IDLE;
    } else { //We got a status response but it wasn't an ACK, so we resend the packet
      sendNextTelemetryPacket();
    }
  } else {
    Serial.write("Container received package of type: ");
    Serial.write(containerReceiveStatus);
  }
  if (containerCommunicationState == STATE_IDLE && !telemetryPacketQueue.isEmpty()) {
    sendNextTelemetryPacket();
    containerCommunicationState = STATE_WAITING_FOR_CONTAINER_RESPONSE;
  }
}
