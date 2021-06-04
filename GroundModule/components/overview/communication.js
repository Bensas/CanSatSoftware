var SerialPort = require('serialport');
var xbee_api = require('xbee-api');

var C = xbee_api.constants;

const GROUND_MAC_ADDRESS = '0013A20041A7952C'; 
const CONTAINER_MAC_ADDRESS = '0013A20041B11802';
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

function toggleSendSimData() {
  sendSimData = !sendSimData;
  document.getElementById('sim-on-indicator').style.display = sendSimData ? 'block' : 'none';
  document.getElementById('sim-off-indicator').style.display = sendSimData ? 'none' : 'block';
  console.log('SIM activated: ' + sendSimData);
  if (sendSimData) currentSimCommandIndex = 0;
}

function sendCommand(cmdData) {
  var frame = {
    type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
    destination64: CONTAINER_MAC_ADDRESS,
    data: cmdData
  };
  console.log("Sending command " + cmdData);
  serialport.write(xbeeAPI.buildFrame(frame), function(err, res) {
    if (err) throw(err);
    else console.log(res);
  });
}

function sendContainerSetTimeCommand(){
  sendCommand('CMD,ST,' + getUtcTimeStr());
}

function getUtcTimeStr() {
  const date = new Date();
  return date.toUTCString().split(" ")[4];
}

function parsePacketAndAddValues(content) {
  const telemetryElements = content.split(',');
  if (telemetryElements.length === 19) { // Received container telemetry
    addValueToTelemetryChart(containerTelemetryChart, Number(telemetryElements[7]), telemetryElements[1]);
    addValueToTelemetryCsv(containerTelemetryWriteStream, content);
    setCurrentTemperature('container-telemetry-temperature', telemetryElements[8]);
    setCurrentBatteryVoltage('container-telemetry-battery-voltage', telemetryElements[9]);
    setCurrentGPSCoords('container-telemetry-gps-coordinates', telemetryElements[11], telemetryElements[12]);
    publishMQTTMessage(content);
  } else { // Received payload telemetry
    if (telemetryElements[3] === 'S1') {
      telemetryElements[1] = getUtcTimeStr();
      addValueToTelemetryChart(payload1TelemetryChart, Number(telemetryElements[4]), telemetryElements[1]);
      addValueToTelemetryCsv(payload1TelemetryWriteStream, content);
      setCurrentTemperature('payload-1-telemetry-temperature', telemetryElements[5]);
      setCurrentRotationRate('payload-1-telemetry-rotation-rate', telemetryElements[6]);
      publishMQTTMessage(content);
    } else if (telemetryElements[3] === 'S2') {
      addValueToTelemetryChart(payload2TelemetryChart, Number(telemetryElements[4]), telemetryElements[1]);
      addValueToTelemetryCsv(payload2TelemetryWriteStream, content);
      setCurrentTemperature('payload-2-telemetry-temperature', telemetryElements[5]);
      setCurrentRotationRate('payload-2-telemetry-rotation-rate', telemetryElements[6]);
      publishMQTTMessage(content);
    } else {
      console.log('Received invalid payload telemetry packet:');
      console.log(telemetryElements[3]);
    }
  }
}
