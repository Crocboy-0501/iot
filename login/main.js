var express = require('express');
var mysql  = require('mysql');

var app = express();

var response={};
var Result='';
app.use(express.static('public'));
const ejs = require('ejs');


//监听到客户端连接后，将index.html 发送给客户端显示
app.get('/', function (req, res) {
    console.log(123);
    console.log(__dirname + "/" + "index.html" );
    res.sendFile( __dirname + "/" + "index.html" );
})

//监听到客户端连接后，进行具体的业务操作（在这里是进行数据库的查找）
//get 得到的客户端的连接，有可能是客户端多种请求连接方式。（在这里是表单提交 给的目标地址 和 GET）
app.get('/process_get', function (req, res) {
    // 输出 JSON 格式
    response = {
        "first_name":req.query.username,
        "last_name":req.query.password
    };

    //创建sql连接对象
    var connection = mysql.createConnection({
        host: 'localhost',
        user: 'IOT',
        password: 'Zhc990501',
        database: 'env'
    });

    //连接数据库
    connection.connect();

    //数据库查询语句
    sql = 'SELECT * FROM users where name="'+response.first_name+'"&&psw='+response.last_name;
    //打印输出语句是否正确
    console.log(sql);
    //请求数据库 开始查询
    connection.query(sql,function (err, result) {
        if(err){
            //[SELECT ERROR] -  connect ECONNREFUSED 127.0.0.1:3306
            // 原因:  数据库服务没有打开
            console.log('[SELECT ERROR] - ',err.message);
            return;
        }

        console.log('--------------------------SELECT----------------------------');
        console.log(result);
        Result=result;
        console.log('------------------------------------------------------------\n\n');
    });

    //中断数据库的连接
   connection.end();
   console.log(Result.length);
   if(response.first_name == 'xiaozhang' && response.last_name == '1234')
   {
        res.render('./table.html');
       res.end("success!");
   }
   else
   {
       res.end("fail....");
   }

   res.end(JSON.stringify(response));

})

 var server = app.listen(8011, function () {
     var host = server.address().address
     var port = server.address().port
     console.log("应用实例，访问地址为 http://%s:%s", host, port)
 })
