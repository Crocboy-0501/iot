var mqtt = require('mqtt');
const mysql = require('mysql')
var opt = {
  port:1883,
  clientId: 'nodejs'
};
const connection = mysql.createConnection({
  host: 'localhost',
  user: 'IOT',
  password: 'Zhc990501',
  database: 'env'
})

var express = require("express");
var app = express();
var http = require('http').Server(app);
var io = require("socket.io")();
app.use(express.static('public'));
var server = app.listen(3000);

var client  = mqtt.connect('tcp://101.43.193.226', opt);
var sio = io.listen(server);

connection.connect();
console.log('已連接至MYSQL伺服器');
client.on('connect', function () {
  console.log('已連接至MQTT伺服器');
  client.subscribe("temp&hum");
  client.subscribe("photo");
});

sio.on('connection', function(socket){
  client.on('message', function (topic, msg) { 

      if(topic=="photo")
        {
          console.log('收到 ' + topic + ' 主題，訊息：' + msg.toString());
          socket.emit('photo', { 'msg': msg.toString() });
        }else{
          console.log('收到 ' + topic + ' 主題，訊息：' + msg.toString());
          var json = JSON.parse(msg.toString());
          connection.query('INSERT INTO  test (datetime, temp, hum, lux, pm25, pm10, windD, windV) VALUES (?, ?, ?, ?, ?, ?, ?, ?)',
          [json.datetime, json.temp, json.humid, json.lux, json.pm25, json.pm10, json.windD, json.windV],  (err, rows, fields) => {
            if (err) throw err
          
          })
          socket.emit('mqtt', { 'msg': msg.toString() });
        };
  });
});