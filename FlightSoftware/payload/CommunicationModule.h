#include <XBee.h>

class CommunicationModule {
  protected:
    AtCommandRequest atCommandRequest;
    AtCommandResponse atCommandResponse = AtCommandResponse();

    // Specify the address of the remote XBee (64-bit unique MAC Address)
    XBeeAddress64 payload1Address = XBeeAddress64(0x0013a200, 0x403e0f30);
    XBeeAddress64 payload2Address = XBeeAddress64(0x0013a200, 0x403e0f30);
    XBeeAddress64 groundAddress = XBeeAddress64(0, 0);
    XBeeAddress64 containerAddress = XBeeAddress64(0, 0);

    int receivePackets(XBee& xbee, ZBRxResponse& responseObj, ZBTxStatusResponse& requestStatusObj);
};
