const readline = require('readline');
const fs = require('fs');

const CONTAINER_TELEMETRY_FILE_NAME = 'Flight_2764_C.csv'
const PAYLOAD_1_TELEMETRY_FILE_NAME = 'Flight_2764_SP1.csv'
const PAYLOAD_2_TELEMETRY_FILE_NAME = 'Flight_2764_SP2.csv'
var containerTelemetryWriteStream = fs.createWriteStream('telemetry/' + CONTAINER_TELEMETRY_FILE_NAME, {flags:'w'});
var payload1TelemetryWriteStream = fs.createWriteStream('telemetry/' + PAYLOAD_1_TELEMETRY_FILE_NAME, {flags:'w'});
var payload2TelemetryWriteStream = fs.createWriteStream('telemetry/' + PAYLOAD_2_TELEMETRY_FILE_NAME, {flags:'w'});
addValueToTelemetryCsv(containerTelemetryWriteStream,
  '<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<MODE>,<SP1_RELEASED>,\
  <SP2_RELEASED>,<ALTITUDE>,<TEMP>,<VOLTAGE>,<GPS_TIME>,<GPS_LATITUDE>,<GPS_LONGITUDE>,\
  <GPS_ALTITUDE>,<GPS_SATS>,<SOFTWARE_STATE>,<SP1_PACKET_COUNT>,<SP2_PACKET_COUNT>,<CMD_ECHO>');
addValueToTelemetryCsv(payload1TelemetryWriteStream,
    '<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<MODE>,<SP1_RELEASED>,\
    <SP2_RELEASED>,<ALTITUDE>,<TEMP>,<VOLTAGE>,<GPS_TIME>,<GPS_LATITUDE>,<GPS_LONGITUDE>,\
    <GPS_ALTITUDE>,<GPS_SATS>,<SOFTWARE_STATE>,<SP1_PACKET_COUNT>,<SP2_PACKET_COUNT>,<CMD_ECHO>');
addValueToTelemetryCsv(payload2TelemetryWriteStream,
  '<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<MODE>,<SP1_RELEASED>,\
  <SP2_RELEASED>,<ALTITUDE>,<TEMP>,<VOLTAGE>,<GPS_TIME>,<GPS_LATITUDE>,<GPS_LONGITUDE>,\
  <GPS_ALTITUDE>,<GPS_SATS>,<SOFTWARE_STATE>,<SP1_PACKET_COUNT>,<SP2_PACKET_COUNT>,<CMD_ECHO>');

function addValueToTelemetryCsv(stream, content) {
  stream.write(content + "\n");
}

function getSimCommandListFromFile() {
  let result = []
  const readInterface = readline.createInterface({
    input: fs.createReadStream('data/flight19v2.csv'),
    output: process.stdout,
    console: false
  });
  readInterface.on('line', function(line) {
    const processedLine = line.split('#')[0];
    if (processedLine.length) result.push(line);
  });
  return result;
}

// const createCsvWriter = require('csv-writer').createObjectCsvWriter;
// const csvWriter = createCsvWriter({
//   path: 'out.csv',
//   header: [
//     {id: 'name', title: 'Name'},
//     {id: 'surname', title: 'Surname'},
//     {id: 'age', title: 'Age'},
//     {id: 'gender', title: 'Gender'},
//   ]
// });

// const data = [
//   {
//     name: 'John',
//     surname: 'Snow',
//     age: 26,
//     gender: 'M'
//   }, {
//     name: 'Clair',
//     surname: 'White',
//     age: 33,
//     gender: 'F',
//   }, {
//     name: 'Fancy',
//     surname: 'Brown',
//     age: 78,
//     gender: 'F'
//   }
// ];

// csvWriter
//   .writeRecords(data)
//   .then(()=> console.log('The CSV file was written successfully'));

function lastElem(array) {
  return array[array.length - 1];
}