#include <XBee.h>
#include <SoftwareSerial.h>

int LED_PIN = 13;
unsigned long lastMillis;


SoftwareSerial XbeeSerial(2, 3);

XBee xbee = XBee();
uint8_t GROUND_NET_ID[2] {0x27, 0x64}; // 2764
uint8_t PAYLOADS_NET_ID[2] {0x27, 0x69}; // 2769
uint8_t ID_CMD[2] {'I','D'};

ZBRxResponse responseObj = ZBRxResponse();
ZBTxRequest requestObj;
ZBTxStatusResponse requestStatusObj = ZBTxStatusResponse();
AtCommandRequest atCommandRequest = AtCommandRequest(ID_CMD, GROUND_NET_ID, sizeof(GROUND_NET_ID));
AtCommandResponse atCommandResponse = AtCommandResponse();

bool currentNetIdIsGround = false;

void setup() {
  lastMillis = millis();
  pinMode(LED_PIN, OUTPUT);
  XbeeSerial.begin(9600);
  xbee.setSerial(XbeeSerial);
  switchToNetId(GROUND_NET_ID);
}

void loop() {
  receivePackets();
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    if (currentNetIdIsGround) {
      switchToNetId(PAYLOADS_NET_ID);
      currentNetIdIsGround = false;
      digitalWrite(LED_PIN, LOW);
    } else {
      switchToNetId(GROUND_NET_ID);
      currentNetIdIsGround = true;
      digitalWrite(LED_PIN, HIGH);
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
