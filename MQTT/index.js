var mqtt = require('mqtt');
var client  = mqtt.connect({
  host: 'cansat.info',
  port: 1883,
  username: '2764',
  password: 'Nuyvlapy296!'
});

topic = 'teams/2764';
 
client.on('connect', function () {
  console.log('Connected');
  client.publish(topic, '2764,0:01:32,10,C,F,N,N,645,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON');
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
