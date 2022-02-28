var express = require('express');
var nodeExcel = require('excel-export');
const mysql = require('mysql')
var app = express();

const connection = mysql.createConnection({
    host: 'localhost',
    user: 'IOT',
    password: 'Zhc990501',
    database: 'env'
})

connection.connect();
console.log('已連接至MYSQL伺服器');

//导出所有用户信息到excel
app.use('/exportExcel', function (req, res) {
    console.log('已跳转');
    //创建一个写入格式map，其中cols(表头)，rows(每一行的数据);
    let conf = {};
    //手动创建表头中的内容
    let cols = ['id', 'datetime', 'temp','lux','hum','pm25','pm10','windD','windV'];
    //在conf中添加cols
    conf.cols = [];
    for(let i = 0; i < cols.length; i++) {
        //创建表头数据所对应的类型,其中包括 caption内容 type类型
        let tits = {};
        //添加内容
        tits.caption = cols[i];
        //添加对应类型，这类型对应数据库中的类型，入number，data但一般导出的都是转换为string类型的
        tits.type = 'string';
        //将每一个表头加入cols中
        conf.cols.push(tits);
    }

    //mysql查询数据库

    connection.query('SELECT * FROM test', 
        function (err, results, fields) {
            if (err) throw err;
            else{
                //创建一个和表头对应且名称与数据库字段对应数据，便于循环取出数据
                let rows = ['id', 'datetime', 'temp','lux','hum','pm25','pm10','windD','windV'];
                //用于承载数据库中的数据
                let datas =[];              
                //循环数据库得到的数据
                for(let i = 0; i < results.length; i++){
                    let row =[];//用来装载每次得到的数据
                    //内循环取出每个字段的数据
                    for(let j = 0; j < rows.length; j++){
                        row.push(results[i][rows[j]].toString());
                    }
                    //将每一个{ }中的数据添加到承载中
                    datas.push(row);
                }
                conf.rows = datas;
                //将所有数据写入nodeExcel中
                let result =nodeExcel.execute(conf);
                //设置响应头，在Content-Type中加入编码格式为utf-8即可实现文件内容支持中文
                res.setHeader('Content-Type', 'application/vnd.openxmlformats;charset=utf-8');
                //设置下载文件命名，中文文件名可以通过编码转化写入到header中。
                res.setHeader("Content-Disposition", "attachment; filename="+ encodeURI('检测系统数据库') + ".xlsx");
                //将文件内容传入
                res.end(result,'binary');
            }                 
        })

})
app.listen(3000);
console.log('Listening on port 3000');