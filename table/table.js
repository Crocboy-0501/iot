var mysql  = require('mysql');
var http = require("http");
var URL = require('url');
var request = require("request");
var querystring = require("querystring");
const express = require("express");
const app = express();
app.use(express.static('public'));


var connection = mysql.createConnection({
    host: 'localhost',
    user: 'IOT',
    password: 'Zhc990501',
    database: 'env'
});
connection.connect();
var  sql = 'SELECT * FROM workers';
//查询数据库的语句
app.get('/table', function(req, res){
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader("Content-Type", "text/html;charset=UTF-8");
//因为我连的是阿里云的服务器，需要解决跨域问题，需要写这个头
    var str=URL.parse(req.url,true).query;
    var arg = querystring.parse(URL.parse(req.url).query);
    //获取具体传过来的数据
    var page = parseInt(arg.page-1);
    var limit = parseInt(arg.limit);
    //开始拼接json文件
    var showData = "";
    //访问数据库
    connection.query(sql,function (err, results) {
        if(err){
            console.log('[SELECT ERROR] - ',err.message);
            return;
        }
        var data = {
            code: 0,
            msg: "成功",
            count: 5,
            data: results
        }
        var obj = JSON.stringify(data);
        res.write(obj);
        res.end();
    });
    }
).listen(8011);
//一定要挂起，处于运行状态
//不过有个问题就是，用了数据库之后需要关掉连接，但是我尝试了写了多个位置，都会报错，我就没有加关闭数据库的语句，如果有人发现有好的解决方法，可以告诉一下啦。