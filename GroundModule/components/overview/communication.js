var SerialPort = require('serialport');
var xbee_api = require('xbee-api');

var C = xbee_api.constants;

const GROUND_MAC_ADDRESS = '0013A20041A7952C'; 
const CONTAINER_MAC_ADDRESS = '0013A2004191B826';
const PAYLOAD_1_MAC_ADDRESS = '0013A20041B118E0';
const PAYLOAD_2_MAC_ADDRESS = '0013A2004191C55C';

var sendSimData = false;
var simCommands = getSimCommandListFromFile();
var currentSimCommandIndex = 0;

var xbeeAPI = new xbee_api.XBeeAPI({
  api_mode: 2
});

var serialport = new SerialPort("COM3", {
  baudRate: 9600,
  parser: xbeeAPI.rawParser()
});

serialport.on("open", function() {
  console.log("Serial port open... sending ATND");
  var frame = {
    type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
    destination64: CONTAINER_MAC_ADDRESS,
    data: 'CMD,HELLO,THERE'
  };

  serialport.write(xbeeAPI.buildFrame(frame), function(err, res) {
    if (err) throw(err);
    else     console.log(res);
  });
});


// Switches the port into "flowing mode"
serialport.on('data', function (data) {
  xbeeAPI.parseRaw(data);
})

xbeeAPI.on("frame_object", function(frame) {
  if (frame.data) parsePacketAndAddValues(String.fromCharCode.apply(null, frame.data));
});

xbeeAPI.on("frame_raw", function(frame) {
  console.log(frame);
});
xbeeAPI.on("error", function(frame) {
  console.log(frame);
});

setInterval(()=> {
  if (sendSimData) {
    sendCommand(simCommands[currentSimCommandIndex]);
    currentSimCommandIndex++;
  }
}, 1000);

function sendCommand(cmdData) {
  var frame = {
    type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
    destination64: CONTAINER_MAC_ADDRESS,
    data: cmdData
  };
  serialport.write(xbeeAPI.buildFrame(frame), function(err, res) {
    if (err) throw(err);
    else     console.log(res);
  });
}

function sendContainerSetTimeCommand(){
  const date = new Date();
  const utcTimeStr = date.toUTCString().split(" ")[4];
  sendCommand('CMD,ST,' + utcTimeStr);
}

function parsePacketAndAddValues(content) {
  const telemetryElements = content.split(',');
  if (telemetryElements.length === 19) { // Received container telemetry
    addValueToTelemetryChart(containerTelemetryChart, Number(telemetryElements[7]), telemetryElements[1]);
    addValueToTelemetryCsv(containerTelemetryWriteStream, content);
  } else { // Received payload telemetry
    if (telemetryElements[3] === 'S1') {
      addValueToTelemetryChart(payload1TelemetryChart, Number(telemetryElements[4]), telemetryElements[1]);
      addValueToTelemetryCsv(payload1TelemetryWriteStream, content);
    } else if (telemetryElements[3] === 'S2') {

      addValueToTelemetryChart(payload2TelemetryChart, Number(telemetryElements[4]), telemetryElements[1]);
      addValueToTelemetryCsv(payload2TelemetryWriteStream, content);
    } else {
      console.log('Received invalid payload telemetry packet:');
      console.log(telemetryElements[3]);
    }
  }
}
