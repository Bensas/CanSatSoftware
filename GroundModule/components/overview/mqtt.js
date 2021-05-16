var mqtt = require('mqtt');

var mqttActivated = false;

var client  = mqtt.connect({
  host: 'cansat.info',
  port: 1883,
  username: '2764',
  password: 'Nuyvlapy296!'
});

topic = 'teams/2764';
 
client.on('connect', function () {
  console.log('Connected');
  client.subscribe('presence', function (err) {
    if (!err) {
      console.log('Received presence!');
      client.publish('presence', 'Hello mqtt')
    } else {
      console.log('Error:');
      console.log(err);
    }
  });
})
 
client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())
  client.end()
})


function toggleMQTTActivated() {
  mqttActivated = !mqttActivated;
  document.getElementById('mqtt-on-indicator').style.display = mqttActivated ? 'block' : 'none';
  document.getElementById('mqtt-off-indicator').style.display = mqttActivated ? 'none' : 'block';
  console.log('MQTT activated: ' + mqttActivated);
}

//Example telemetry '2764,0:01:32,10,C,F,N,N,645,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON'
function publishMQTTMessage(message){
  client.publish(topic, topic);
}