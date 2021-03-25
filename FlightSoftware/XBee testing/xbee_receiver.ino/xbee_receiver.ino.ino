#include <XBee.h>
#include <SoftwareSerial.h>

SoftwareSerial XbeeSerial(2, 3);

XBee xbee = XBee();
ZBRxResponse responseObj = ZBRxResponse();

ZBTxRequest requestObj;
ZBTxStatusResponse requestStatusObj = ZBTxStatusResponse();

uint8_t GROUND_NET_ID[1] {0xACC}; // 2764
uint8_t PAYLOADS_NET_ID[1] {0xAD1}; // 2769
uint8_t ID_CMD[2] {'I','D'};
AtCommandRequest atCommandRequest = AtCommandRequest(ID_CMD, GROUND_NET_ID, sizeof(GROUND_NET_ID));
AtCommandResponse atCommandResponse = AtCommandResponse();

// Specify the address of the remote XBee (this is the SH + SL)
XBeeAddress64 payload1Address = XBeeAddress64(0x0, 0x403e0f30);
XBeeAddress64 payload2Address = XBeeAddress64(0x0, 0x403e0f30);
XBeeAddress64 groundAddress = XBeeAddress64(0x00, 0xE8);

void setup() {
  // Start the serial port
  Serial.begin(9600);
  XbeeSerial.begin(9600);
  // Tell XBee to use Hardware Serial. It's also possible to use SoftwareSerial
  xbee.setSerial(XbeeSerial);

}

void loop() {
  if (receivePackets() == ZB_RX_RESPONSE) { //We received  telemetry
    uint8_t* packetData = responseObj.getData();
    uint8_t dataLength = responseObj.getDataLength();
    parseReceivedPacket(packetData, dataLength);
    if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
      //console.log('ST command package received and acknowledged');
    } else {
      //console.log('ST command package received but sender didnt get an ack');
    }
  }

}

void switchToNetId(uint8_t netId[]){
  atCommandRequest.setCommandValue(netId);
  xbee.send(atCommandRequest);
}

int receivePackets() {
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
      // if (txStatus.getDeliveryStatus() == SUCCESS) {
      //   return ZB_TX_STATUS_RESPONSE;
      // } else {
      //   // We sould resend the package
      // }
    } else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atCommandResponse);
      return AT_COMMAND_RESPONSE;
    }
  } else if (xbee.getResponse().isError()) {
    Serial.write("Error reading packet.  Error code: \n");  
    Serial.write(xbee.getResponse().getErrorCode());
    Serial.write("\n");
  }
  return false;
}

void parseReceivedPacket(uint8_t* packetData, uint8_t packetLength) {
  for (int i = 0; i < packetLength; i++) {
    Serial.write(packetData[i]);
  }
  Serial.write("\n");
}
