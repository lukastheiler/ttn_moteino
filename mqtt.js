var mqtt = require('mqtt');

var client = mqtt.connect('tcp://croft.thethings.girovito.nl:1883');
// nodes/5A480881/packets
// gateways/B827EBFFFEC7F595/status

var i=0;

client.on('connect', function () {
    nodes=['nodes/FEEDBEEF/packets'];
    nodes.forEach(n => {
    	   console.log("connected, subscribed "+n);
       	 client.subscribe(n);
        })
});

client.on('message', function (topic, message) {
  console.log("["+i+"] "+new Date()+" from "+topic.toString());
  var msg = JSON.parse(message);
  msg.data=new Buffer(msg.data, 'base64').toString('ascii')
  console.log(msg);
  ++i;
});
