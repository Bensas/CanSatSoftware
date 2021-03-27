#include <XBee.h>
#include <SoftwareSerial.h>

SoftwareSerial XbeeSerial(2, 3);

XBee xbee = XBee();
ZBRxResponse responseObj = ZBRxResponse();

ZBTxRequest requestObj;
ZBTxStatusResponse requestStatusObj = ZBTxStatusResponse();

uint8_t GROUND_NET_ID[2] {0x27, 0x69}; // 2764
uint8_t PAYLOADS_NET_ID[4] {'2', '7', '6', '9'}; // 2769
uint8_t ID_CMD[2] {'I','D'};
AtCommandRequest atCommandRequest = AtCommandRequest(ID_CMD, GROUND_NET_ID, sizeof(GROUND_NET_ID));
AtCommandResponse atResponse = AtCommandResponse();

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

  switchToNetId(GROUND_NET_ID);

}

void loop() {
  int pack = receivePackets();
  if (pack == ZB_RX_RESPONSE) { //We received  telemetry
    uint8_t* packetData = responseObj.getData();
    uint8_t dataLength = responseObj.getDataLength();
    parseReceivedPacket(packetData, dataLength);
    if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
      //console.log('ST command package received and acknowledged');
    } else {
      //console.log('ST command package received but sender didnt get an ack');
    }
  } else if (pack == AT_COMMAND_RESPONSE) {
    if (atResponse.isOk()) {
        Serial.print("Command [");
        Serial.print(atResponse.getCommand()[0]);
        Serial.print(atResponse.getCommand()[1]);
        Serial.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          Serial.print("Command value length is ");
          Serial.println(atResponse.getValueLength(), DEC);

          Serial.print("Command value: ");
          
          for (int i = 0; i < atResponse.getValueLength(); i++) {
            Serial.print(atResponse.getValue()[i], HEX);
            Serial.print(" ");
          }

          Serial.println("");
        }
      } 
      else {
        Serial.print("Command return error code: ");
        Serial.println(atResponse.getStatus(), HEX);
      }
  } else {
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
      xbee.getResponse().getAtCommandResponse(atResponse);
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
