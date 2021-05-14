var SerialPort = require('serialport');
var xbee_api = require('xbee-api');

var C = xbee_api.constants;

const GROUND_MAC_ADDRESS = '0013A2004191B826';
const CONTAINER_MAC_ADDRESS = '0013A2004191C55C';


var xbeeAPI = new xbee_api.XBeeAPI({
  api_mode: 2
});

var serialport = new SerialPort("COM5", {
  baudRate: 9600,
  parser: ()=>console.log('Heeey mona')
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
  xbeeAPI.rawParser()
  console.log('Data:', data)
})

// Pipe the data into another stream (like a parser or standard out)
const lineStream = serialport.pipe(new Readline())

xbeeAPI.on("frame_object", function(frame) {
  console.log(frame);
  parsePacketAndAddValues(frame.content);
});

xbeeAPI.on("frame_raw", function(frame) {
  console.log(frame);
});
xbeeAPI.on("error", function(frame) {
  console.log(frame);
});


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
    addValueToTelemetryChart(containerTelemetryChartConfig, Number(telemetryElements[7]), telemetryElements[1]);
    addValueToTelemetryCsv(containerTelemetryWriteStream, content);
  } else if (telemetryElements.length === 7) { // Received payload telemetry
    if (telemetryElements[3] === 'SP1') {
      addValueToTelemetryChart(payload1TelemetryChartConfig, Number(telemetryElements[7]), telemetryElements[1]);
      addValueToTelemetryCsv(payload1TelemetryWriteStream, content);
    } else if (telemetryElements[3] === 'SP2') {
      addValueToTelemetryChart(payload2TelemetryChartConfig, Number(telemetryElements[7]), telemetryElements[1]);
      addValueToTelemetryCsv(payload2TelemetryWriteStream, content);
    } else {
      console.log('Received invalid payload telemetry packet:');
      console.log(content);
    }
  } else {
    console.log('Received invalid payload telemetry packet:');
    console.log(content);
  }
}
