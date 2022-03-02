const express = require("express");
const app = express();
const mqtt = require('mqtt');
const mysql = require('mysql');
const ejs = require('ejs');

const http = require('http').createServer(app);
const io = require('socket.io')(http)

const url = require('url');
const fs = require('fs');

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


app.use(express.static('public'));


var client  = mqtt.connect('tcp://101.43.193.226', opt);


connection.connect();
console.log('已連接至MYSQL伺服器');
client.on('connect', function () {
  console.log('已連接至MQTT伺服器');
  client.subscribe("temp&hum");
  client.subscribe("photo");
});

app.get('/login', function(req, res){
            
    // 假设定义从服务器请求的数据
    let msg = "数据库里面获取的数据"
    connection.query('SELECT * FROM test', 
    function (err, results, fields) {
        if (err) throw err;
        else console.log('Selected ' + results.length + ' row(s).');
        for (i = 0; i < results.length; i++) {
            console.log('Row: ' + JSON.stringify(results[i]));
        }

        ejs.renderFile('./views/demo.ejs', {
            msg: msg,
            list: results
        }, (err, data) => {
            res.writeHead(200, {
                'Content-Type': 'text/html;charset="utf-8"'
            });
            res.end(data)
        })


    })

    function exitHandler(options, err) {
        connection.end();
        if (options.cleanup)
            console.log('clean');
        if (err)
            console.log(err.stack);
        if (options.exit)
            process.exit();
    }
    
    //do something when app is closing
    process.on('exit', exitHandler.bind(null, {cleanup: true}));
    
});

// http.createServer((req, res) => {
//     // 路由
//     let pathname = url.parse(req.url).pathname;
//     if (pathname == '/login') {
        
//         // 假设定义从服务器请求的数据
//         let msg = "数据库里面获取的数据"
//         connection.query('SELECT * FROM test', 
//         function (err, results, fields) {
//             if (err) throw err;
//             else console.log('Selected ' + results.length + ' row(s).');
//             for (i = 0; i < results.length; i++) {
//                 console.log('Row: ' + JSON.stringify(results[i]));
//             }

//             ejs.renderFile('./views/demo.ejs', {
//                 msg: msg,
//                 list: results
//             }, (err, data) => {
//                 res.writeHead(200, {
//                     'Content-Type': 'text/html;charset="utf-8"'
//                 });
//                 res.end(data)
//             })


//         })

//         function exitHandler(options, err) {
//             connection.end();
//             if (options.cleanup)
//                 console.log('clean');
//             if (err)
//                 console.log(err.stack);
//             if (options.exit)
//                 process.exit();
//         }
        
//         //do something when app is closing
//         process.on('exit', exitHandler.bind(null, {cleanup: true}));


//     }
//     else if(pathname.includes('/layui')){
//             fs.readFile('./' + pathname, (err, data)=>{
//                 res.end(data);
//             });
//     }
 
// });

io.on('connection', function(socket){
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
  
http.listen(3000);