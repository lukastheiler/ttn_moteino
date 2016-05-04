var mqtt = require('mqtt');

// The username is the EUI and the password is the Access Key you get from `ttnctl applications`
var client = mqtt.connect('tcp://staging.thethingsnetwork.org:1883',
              { username: 'xxxxxx',
                password: 'xxxxxx'
              });

var i=0;

client.on('connect', function () {
  client.subscribe("+/devices/+/up");
});

client.on('message', function (topic, buffer) {
  console.log("["+i+"] "+new Date()+" from "+topic.toString());
  var data=new Buffer(buffer, 'base64').toString('ascii')
  var message = JSON.parse(data);
  message.payload_decrypted=new Buffer(message.payload, 'base64').toString('ascii')
  console.log(message);
  ++i;
});
