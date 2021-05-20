#include "PayloadCommunicationModule.h"

void PayloadCommunicationModule::init(XBee& xbeeDevice) {
  containerXBee = xbeeDevice;
  Serial.begin(19200);
}

void PayloadCommunicationModule::parseCommandPacket(uint8_t* packetData, uint8_t packetLength) {
  /*
    Examples:
    0 - Turn off teemetry
    1 - Turn on telemetry
  */
  if (packetData[0] == '0') {
    setPayloadTelemetryActivated(false);
    Serial.println("Payload telemetry deactivated");
  }
  else if (packetData[0] == '1') {
    setPayloadTelemetryActivated(true);
    Serial.println("Payload telemetry deactivated");
  }
}


void PayloadCommunicationModule::sendNextTelemetryPacket(){
  Serial.write(telemetryPacketQueue.head->data, telemetryPacketQueue.head->dataLength);
  Serial.write('\n');
  containerRequestObj = ZBTxRequest(containerAddress, telemetryPacketQueue.head->data, telemetryPacketQueue.head->dataLength);
  containerXBee.send(containerRequestObj);
  telemetryPacketQueue.removeHead();
  packetCount++;
}

void PayloadCommunicationModule::loop() {
  int containerReceiveStatus = receivePackets(containerXBee, containerResponseObj, containerRequestStatusObj);
  if (containerReceiveStatus == ZB_RX_RESPONSE) { // We received a message so we parse it and act on it.
    uint8_t* packetData = containerResponseObj.getData();
    uint8_t dataLength = containerResponseObj.getDataLength();
    parseCommandPacket(packetData, dataLength);
     if (containerResponseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
       //console.log('ST command packet received and acknowledged');
     } else {
       //console.log('ST command packet received but sender didnt get an ack');
     }
  } else if (containerReceiveStatus == ZB_TX_STATUS_RESPONSE) { // We received a status update on a previously sent packet
    if (containerRequestStatusObj.getDeliveryStatus() == SUCCESS) { // We got an ACK Wohoo!
      Serial.println("Ack");
      containerCommunicationState = STATE_IDLE;
    } else { //We got a status response but it wasn't an ACK, so we resend the packet
//      sendNextTelemetryPacket();
    }
  } else {
//    Serial.write("Container received packet of type: ");
//    Serial.write(containerReceiveStatus);
//      Serial.write("No response");
  }
  if (containerCommunicationState == STATE_IDLE && !telemetryPacketQueue.isEmpty()) {
    Serial.write("Sending ");
    sendNextTelemetryPacket();
//    containerCommunicationState = STATE_WAITING_FOR_CONTAINER_RESPONSE;
  }
}
