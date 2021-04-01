#include "ContainerCommunicationModule.h"

void ContainerCommunicationModule::init(XBee groundXBeeDevice, XBee payloadsXBeeDevice, DS3231 RTC) {
  // Start the serial port
  groundXBee = groundXBeeDevice;
  payloadsXBee = payloadsXBeeDevice;
  rtc = RTC;
}

void ContainerCommunicationModule::setRtcTimeFromPacket(uint8_t* packetData, uint8_t packetLength) {
  //Example command: CMD,2764,ST,13:35:59
  if (packetLength != 20 || packetData[9] != 'S' || packetData[10] != 'T'){
    // console.log('Rtc time set package is invalid!');
    return;
  }
  //console.log('Parsing time package');
  uint8_t buffer[3] = {0, 0, 0};

  memcpy(buffer, &packetData[12], 2);
  rtc.setHour(atoi(buffer)); 

  memcpy(buffer, &packetData[15], 2);
  rtc.setMinute(atoi(buffer)); 

  memcpy(buffer, &packetData[18], 2);
  rtc.setSecond(atoi(buffer)); 
}

void ContainerCommunicationModule::parseReceivedPacket(uint8_t* packetData, uint8_t packetLength) {
  if (packetData[0] == 'C') {
    parseCommandPacket(packetData, packetLength);
  } else {
    parseTelemetryPacket(packetData, packetLength);
  }
}

void ContainerCommunicationModule::parseCommandPacket(uint8_t* packetData, uint8_t packetLength) {
  /*
    Examples:
    CMD,2764,CX,ON activates Container telemetry
    CMD,2764,ST,13:35:59 sets the mission time to the value given (we already parse these commmands using setRtcTimeFromPacket)
    CMD,2764,SIM,ENABLE and CMD,1000,SIM,ACTIVATE commands are required to begin simulation mode.
    CMD,2764,SP1X,ON will trigger the Container to relay a command to theScience Payload 1 to begin telemetry transmissions.
    CMD,1000,SIMP,101325 provides a simulated pressure reading to the Container (101325 Pascals = approximately sea level). Note: this command is to be used only in simulation mode.
  */
  if (packetData[9] == 'C') { //CX command
    setContainerTelemetryActivated(packetData[12] == 'O' && packetData[13] == 'N');
  }
  else if (packetData[9] == 'S' && packetData[10] == 'I') { //SIM or SIMP command
    if (packetData[12] == ',') { //SIM command
      if (packetData[13] =='D') {
        setContainerSimulationMode(SIMULATION_DISABLED);
      }
      else if (packetData[13] =='E') {
        setContainerSimulationMode(SIMULATION_ENABLED);
      }
      else {
        setContainerSimulationMode(SIMULATION_ACTIVATED);
      }
    } else { //SIMP command
      uint8_t pressureValueBuffer[8];
      uint8_t i;
      for (i = 14; i < packetLength; i++) {
        pressureValueBuffer[i-14] = packetData[i];
      }
      pressureValueBuffer[i] = 0;
      setLatestSimulationPressureValue(atoi(pressureValueBuffer));
    }
  }
  else if (packetData[9] == 'S' && packetData[10] == 'P') { //SP1X or SP2X command
    if (packetData[14] == 'O' && packetData[15] == 'N'){
      if (packetData[11] == '1') {
        payload1CommandQueue.add('1');
      } else {
        payload2CommandQueue.add('1');
      }
    } else {
      if (packetData[11] == '1') {
        payload1CommandQueue.add('0');
      } else {
        payload2CommandQueue.add('0');
      }
    }
  }
  memcpy(lastCommandEcho, packetData, packetLength);
  lastCommandEcho[packetLength] = 0;
}

void ContainerCommunicationModule::parseTelemetryPacket(uint8_t* packetData, uint8_t packetLength) {
  uint8_t packetDataCopy[packetLength];
  memcpy(packetDataCopy, packetData, packetLength);
  telemetryPacketQueue.add(packetDataCopy, packetLength);
}

void ContainerCommunicationModule::sendNextPayload1Command() {
  payloadsRequestObj = ZBTxRequest(payload1Address, payload1CommandQueue.head->data, sizeof(uint8_t));
  payloadsXBee.send(payloadsRequestObj);
}

void ContainerCommunicationModule::sendNextPayload2Command() {
  payloadsRequestObj = ZBTxRequest(payload2Address, payload2CommandQueue.head->data, sizeof(uint8_t));
  payloadsXBee.send(payloadsRequestObj);
}

void ContainerCommunicationModule::sendNextTelemetryPacket(){
  groundRequestObj = ZBTxRequest(groundAddress, telemetryPacketQueue.head->data, telemetryPacketQueue.head->dataLength);
  groundXBee.send(groundRequestObj);
}

void ContainerCommunicationModule::loop() {
  manageGroundCommunication();
  managePayloadsCommunication();
}

void ContainerCommunicationModule::manageGroundCommunication() {
  int groundReceiveStatus = receivePackets(groundXBee, groundResponseObj, groundRequestStatusObj);
  if (groundReceiveStatus == ZB_RX_RESPONSE) { // We received a message so we parse it and act on it.
    uint8_t* packetData = groundResponseObj.getData();
    uint8_t dataLength = groundResponseObj.getDataLength();
    parseReceivedPacket(packetData, dataLength);
    // if (groundResponseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
    //   //console.log('ST command package received and acknowledged');
    // } else {
    //   //console.log('ST command package received but sender didnt get an ack');
    // }
  } else if (groundReceiveStatus == ZB_TX_STATUS_RESPONSE) { // We received a status update on a previously sent packet
    if (groundRequestStatusObj.getDeliveryStatus() == SUCCESS) { // We got an ACK Wohoo!
      telemetryPacketQueue.removeHead(); 
      groundCommunicationState = STATE_IDLE;
    } else { //We got a status response but it wasn't an ACK, so we resend the packet
      sendNextTelemetryPacket();
    }
  } else {
    Serial.println("Container received package of type: ");
    Serial.println(groundReceiveStatus);
  }
  if (groundCommunicationState == STATE_IDLE && !telemetryPacketQueue.isEmpty()) {
    sendNextTelemetryPacket();
    groundCommunicationState = STATE_WAITING_FOR_GROUND_RESPONSE;
  }
}

void ContainerCommunicationModule::managePayloadsCommunication() {
  int payloadsReceiveStatus = receivePackets(payloadsXBee, payloadsResponseObj, payloadsRequestStatusObj);
  if (payloadsReceiveStatus == ZB_RX_RESPONSE) { // We received a message so we parse it and act on it.
    uint8_t* packetData = payloadsResponseObj.getData();
    uint8_t dataLength = payloadsResponseObj.getDataLength();
    parseReceivedPacket(packetData, dataLength);
    // if (payloadsResponseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
    //   //console.log('ST command package received and acknowledged');
    // } else {
    //   //console.log('ST command package received but sender didnt get an ack');
    // }
  } else if (payloadsReceiveStatus == ZB_TX_STATUS_RESPONSE) { // We received a status update on a previously sent packet
    if (payloadsRequestStatusObj.getDeliveryStatus() == SUCCESS) { // We got an ACK Wohoo!
      switch (payloadCommunicationState) {
        case STATE_WAITING_FOR_PAYLOAD_1_RESPONSE:
          payload1CommandQueue.removeHead();
          break;
        case STATE_WAITING_FOR_PAYLOAD_2_RESPONSE:
          payload2CommandQueue.removeHead();
          break;
      }
      payloadCommunicationState = STATE_IDLE;
    } else { //We got a status response but it wasn't an ACK, so we resend the packet
      switch (payloadCommunicationState) {
        case STATE_WAITING_FOR_PAYLOAD_1_RESPONSE:
          sendNextPayload1Command();
          break;
        case STATE_WAITING_FOR_PAYLOAD_2_RESPONSE:
          sendNextPayload2Command();
          break;
      }
    }
  } else {
    Serial.println("Container received package of type: ");
    Serial.println(payloadsReceiveStatus);
  }

  if (payloadCommunicationState == STATE_IDLE) {
    if (!payload1CommandQueue.isEmpty()){
      sendNextPayload1Command();
      payloadCommunicationState = STATE_WAITING_FOR_PAYLOAD_1_RESPONSE;
    } else {
      sendNextPayload2Command();
      payloadCommunicationState = STATE_WAITING_FOR_PAYLOAD_2_RESPONSE;
    }
  }
}