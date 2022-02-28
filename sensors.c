/* TTY testing utility (using tty driver)
 * Copyright (c) 2017
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I /path/to/cross-kernel/include
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>  
#include <termios.h>  
#include <errno.h>   
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>  
#include <json-c/json.h>
#include "MQTTClient.h"
#include <math.h>
#include "time.h"

#define ADDRESS     "tcp://101.43.193.226:1883" //更改此处地址
#define CLIENTID    "ExampleClientPub"     //更改此处客户端ID
#define TOPIC       "temp&hum"                 //更改发送的话题
#define PAYLOAD     "Hello, GetIoT.tech!"  //更改信息内容
#define QOS         1
#define TIMEOUT     10000L

int speed_arr[] = {
	B115200,
	B57600,
	B38400,
	B19200,
	B9600,
	B4800,
	B2400,
	B1200,
	B300
};
 
int name_arr[] = {
	115200,
	57600,
	38400,
	19200,
	9600,
	4800,
	2400,
	1200,
	300
};
 
/**
 * libtty_setopt - config tty device
 * @fd: device handle
 * @speed: baud rate to set
 * @databits: data bits to set
 * @stopbits: stop bits to set
 * @parity: parity set
 *
 * The function return 0 if success, or -1 if fail.
 */
int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity)
{
	struct termios newtio;
	struct termios oldtio;
	int i;
	
	bzero(&newtio, sizeof(newtio));
	bzero(&oldtio, sizeof(oldtio));
	
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");    
		return -1; 
	}
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
 
	/* set tty speed */
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {      
			cfsetispeed(&newtio, speed_arr[i]); 
			cfsetospeed(&newtio, speed_arr[i]);   
		} 
	}
	
	/* set data bits */
	switch (databits) {
	case 5:                
		newtio.c_cflag |= CS5;
		break;
	case 6:                
		newtio.c_cflag |= CS6;
		break;
	case 7:                
		newtio.c_cflag |= CS7;
		break;
	case 8:    
		newtio.c_cflag |= CS8;
		break;  
	default:   
		fprintf(stderr, "unsupported data size\n");
		return -1; 
	}
	
	/* set parity */
	switch (parity) {  
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB;    /* Clear parity enable */
		newtio.c_iflag &= ~INPCK;     /* Disable input parity check */
		break; 
	case 'o':  
	case 'O':    
		newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break; 
	case 'e': 
	case 'E':  
		newtio.c_cflag |= PARENB;    /* Enable parity */   
		newtio.c_cflag &= ~PARODD;   /* Even parity instead of odd */  
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break;
	default:  
		fprintf(stderr, "unsupported parity\n");
		return -1; 
	} 
	
	/* set stop bits */ 
	switch (stopbits) {  
	case 1:   
		newtio.c_cflag &= ~CSTOPB; 
		break;
	case 2:   
		newtio.c_cflag |= CSTOPB; 
		break;
	default:   
		perror("unsupported stop bits\n"); 
		return -1;
	}
 
	newtio.c_cc[VTIME] = 0;	  /* Time-out value (tenths of a second) [!ICANON]. */
	newtio.c_cc[VMIN] = 7;    /* Minimum number of bytes read at once [!ICANON]. */
	//输入模式  
    newtio.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);  
	//设置数据流控制  
    newtio.c_iflag &= ~(IXON|IXOFF|IXANY);
	tcflush(fd, TCIOFLUSH);  
	
	if (tcsetattr(fd, TCSANOW, &newtio) != 0)  
	{
		perror("tcsetattr");
		return -1;
	}
	return 0;
}
 
/**
 * libtty_open - open tty device
 * @devname: the device name to open
 *
 * In this demo device is opened blocked, you could modify it at will.
 */
int libtty_open(const char *devname)
{
	int fd = open(devname, O_RDWR | O_NOCTTY); //阻塞的方式进行读取
//	int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY); //非阻塞的方式进行读取

	int flags = 0;
	
	if (fd == -1) {                        
		perror("open device failed");
		return -1;            
	}
	
	flags = fcntl(fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl failed.\n");
		return -1;
	}
		
	if (isatty(fd) == 0)
	{
		printf("not tty device.\n");
		return -1;
	}
	else
		printf("tty device test ok.\n");
	
	return fd;
}
 
/**
 * libtty_close - close tty device
 * @fd: the device handle
 *
 */
int libtty_close(int fd)
{
	return close(fd);
}
 
//十六进制转换为十进制
int hexTodec(char high, char low)
{
    char a[4] = {low & 0x0f, (low >> 4) & 0x0f, high & 0x0f, (high  >> 4) & 0xf0}; //符号问题
    int b[4] = {0}; //16进制转换为int类型
    int i, j, sum = 0;
    int c = 0;
    while(sum < 4)
    {
		//Testing
		//printf("\n[%x]\n", a[sum]);
        if((a[sum] >= 'a') && (a[sum] <= 'f'))
        {
            b[sum] = a[sum] - 'a' + 10;
            sum++;
            continue;
        }
        if((a[sum] >= 'A') && (a[sum] <= 'F'))
        {
            b[sum] = a[sum] - 'A' + 10;
            sum++;
            continue;
        }
        b[sum] = a[sum] - '0';
        sum ++;
        
    }
    //每一位转换为十进制
    for(i = 0; i < sum; i++)
    {
        b[sum - 1 - i] = a[sum - 1 - i] * pow(16, 3 - i); 
		//Testing
		//printf("\n[%d]\n", b[sum - 1 - i]);
    }
    //直接累加
    for(j = 0; j < sum; j++)
    {

        c = c + b[j];
    }

//    printf("**********湿度计算结果：%d***********\n", c);
    return c;

}

//获取时间
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
void getTime (char *buffer)
{

    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );


    // printf ( "\007The current year is: %d\n", (timeinfo->tm_year)+1900);
    // printf ( "\007The current month is: %d\n", (timeinfo->tm_mon)+1);
    // printf ( "\007The current day is: %d\n", (timeinfo->tm_mday));
    // printf ( "\007The current hour is: %d\n", (timeinfo->tm_hour));
    // printf ( "\007The current minute is: %d\n", (timeinfo->tm_min));  
    // printf ( "\007The current second is: %d\n", (timeinfo->tm_sec));



//    char buffer[200];
    int j = 0;
    //格式化并打印各种数据到buffer中
    j = sprintf(buffer, "%d",(timeinfo->tm_year)+1900);
    if((timeinfo->tm_mon)+1 < 10){
        j += sprintf(buffer + j, "%02d",(timeinfo->tm_mon)+1);
    }else{
        j += sprintf(buffer + j, "%d",(timeinfo->tm_mon)+1); 
    }

    if((timeinfo->tm_mday) < 10){
        j += sprintf(buffer + j, "%02d",(timeinfo->tm_mday)); 
    }else{
        j += sprintf(buffer + j, "%d",(timeinfo->tm_mday)); 
    }

    if((timeinfo->tm_hour) < 10){
        j += sprintf(buffer + j, "%02d",(timeinfo->tm_hour)); 
    }else{
        j += sprintf(buffer + j, "%d",(timeinfo->tm_hour)); 
    }

    if((timeinfo->tm_min) < 10){
        j += sprintf(buffer + j, "%02d",(timeinfo->tm_min)); 
    }else{
        j += sprintf(buffer + j, "%d",(timeinfo->tm_min)); 
    }

    if((timeinfo->tm_sec) < 10){
        j += sprintf(buffer + j, "%02d",(timeinfo->tm_sec)); 
    }else{
        j += sprintf(buffer + j, "%d",(timeinfo->tm_sec)); 
    }

    printf("Output:\n%s\ncharacter count=%d\n",buffer,j);



    
} 
//=======================================================================================

 //mqtt发送 
 int sendtemp(char *buffer, double a, double b, int c, int d, int e, double f, int g)
{
    //声明一个MQTTClient
    MQTTClient client;
    char *username = "rudy";      //添加的用户名
    char *password = "p@ssw0rd";  //添加的密码
    char *windDirection;
    int rc;

    //初始化MQTT Client选项
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    //#define MQTTClient_message_initializer { {'M', 'Q', 'T', 'M'}, 0, 0, NULL, 0, 0, 0, 0 }
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //声明消息token
    MQTTClient_deliveryToken token;

    //使用参数创建一个client，并将其赋值给之前声明的client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username; //将用户名写入连接选项中
    conn_opts.password = password;//将密码写入连接选项中

    //使用MQTTClient_connect将client连接到服务器，使用指定的连接选项。成功则返回MQTTCLIENT_SUCCESS
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
//+===+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    struct json_object *obj = json_object_new_object();

//风向判断
    switch (g)
    {
        case 0:
            windDirection = "北风";
            break;
        case 1:
            windDirection = "东北风";
            break;
        case 2:
            windDirection = "东风";
            break;
        case 3:
            windDirection = "东南风";
            break;
        case 4:
            windDirection = "南风";
            break;
        case 5:
            windDirection = "西南风";
            break;
        case 6:
            windDirection = "西风";
            break;
        case 7:
            windDirection = "西北风";
            break;

        default:
            windDirection = "null";                                                                                
    }

//小数点调整测试
//----------------------------------------------------------------------
	
	char str_a[10];
	char str_b[10];
    char str_f[10];
	sprintf(str_a,"%.1f", a);
	sprintf(str_b,"%.1f", b);//设置字符串输出格式
    sprintf(str_f,"%.1f", f);
//===============================test mysql column err
	char str_m[10];
	char str_n[10];
    char str_q[10];

    char str_o[10];
	sprintf(str_m,"%d", c);
	sprintf(str_n,"%d", d);//设置字符串输出格式
    sprintf(str_q,"%d", e);
    sprintf(str_o,"%s", windDirection);


//=====================================================================

//----------------------------------------------------------------------
//往json对象添加键值对  json_object_new_string把字符串转换成json对象
//    json_object_object_add(obj, "name", json_object_new_string("jack"));
    json_object_object_add(obj, "datetime", json_object_new_string(buffer));

    json_object_object_add(obj, "temp", json_object_new_string(str_a));
    json_object_object_add(obj, "humid", json_object_new_string(str_b));
	// json_object_object_add(obj, "lux", json_object_new_int(c));
    // json_object_object_add(obj, "pm25", json_object_new_int(d));
    // json_object_object_add(obj, "pm10", json_object_new_int(e));


    json_object_object_add(obj, "lux", json_object_new_string(str_m));
    json_object_object_add(obj, "pm25", json_object_new_string(str_n));
    json_object_object_add(obj, "pm10", json_object_new_string(str_q));


    json_object_object_add(obj, "windD", json_object_new_string(str_o));
    json_object_object_add(obj, "windV", json_object_new_string(str_f));
//    json_object_object_add(obj, "photo", json_object_new_string(photo));
//    json_object_object_add(obj, "sex", json_object_new_string("male"));

    //打印json对象
    printf("%s\n", json_object_to_json_string(obj));

    const char *PAYLOADtest = json_object_to_json_string(obj);
    pubmsg.payload = PAYLOADtest;
    pubmsg.payloadlen = strlen(PAYLOADtest);

//++===++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    // pubmsg.payload = PAYLOAD;
    // pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic '%s' for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);

    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return rc;
}

void tty_testTemp(int fd, char *buf, char *buf1)
{

	int nwrite, nread;
	//unsigned int buf;
	






    //reading the number of temp and humid
    nwrite = write(fd, buf, sizeof(buf));
    printf("*********************\n");
    printf("写读取温湿度光照指令\n");
    printf("*********************\n");

    printf("%x ",buf[0]);
    printf("%x ",buf[1]);
    printf("%x ",buf[2]);
    printf("%x ",buf[3]);
    printf("%x ",buf[4]);
    printf("%x ",buf[5]);
    printf("%x ",buf[6]);
    printf("%x\n",buf[7]);
            
    printf("wrote %d bytes already.\n", nwrite);
    printf("*********************\n");
    printf("读取温湿度光照数据\n");
    printf("*********************\n");

		


    nread = read(fd, buf1, 19);
    printf("read %d bytes already.\n", nread);
    printf("%x ",buf1[0]);
    printf("%x ",buf1[1]);
    printf("%x ",buf1[2]);
    printf("%x ",buf1[3]);
    printf("%x ",buf1[4]);
    printf("%x ",buf1[5]);
    printf("%x ",buf1[6]);
    printf("%x ",buf1[7]);
    printf("%x ",buf1[8]);
    printf("%x ",buf1[9]);
    printf("%x ",buf1[10]);
    printf("%x ",buf1[11]);
    printf("%x ",buf1[12]);
    printf("%x ",buf1[13]);
    printf("%x ",buf1[14]);
    printf("%x ",buf1[15]);
    printf("%x ",buf1[16]);
    printf("%x ",buf1[17]);
    printf("%x\n",buf1[18]);
    printf("\n\n");
}	

void tty_testPm(int fd, char *buf2, char *buf3)
{

	int nwrite, nread;
	//unsigned int buf;
	

    nwrite = write(fd, buf2, sizeof(buf2));
    printf("============空气质量问询帧===========\n");
    printf("wrote %d bytes already.\n", nwrite);
    printf("%x ",buf2[0]);
    printf("%x ",buf2[1]);
    printf("%x ",buf2[2]);
    printf("%x ",buf2[3]);
    printf("%x ",buf2[4]);
    printf("%x ",buf2[5]);
    printf("%x ",buf2[6]);
    printf("%x\n",buf2[7]);
    printf("wrote %d bytes already.\n", nwrite);
    printf("============空气质量问询帧===========\n");
    

    nread = read(fd, buf3, 8);
    printf("============空气质量应答帧===========\n");
    printf("read %d bytes already.\n", nread);
    printf("%x ",buf3[0]);
    printf("%x ",buf3[1]);
    printf("%x ",buf3[2]);
    printf("%x ",buf3[3]);
    printf("%x ",buf3[4]);
    printf("%x ",buf3[5]);
    printf("%x ",buf3[6]);
    printf("%x ",buf3[7]);
    printf("%x \n",buf3[8]);

    printf("read %d bytes already.\n", nread);
    printf("============空气质量应答帧===========\n");
    printf("\n\n");
	
}	

void tty_testwindD(int fd, char *buf2, char *buf3)
{

	int nwrite, nread;
	//unsigned int buf;
	

    nwrite = write(fd, buf2, sizeof(buf2));
    printf("===========风向问询帧===========\n");
    printf("wrote %d bytes already.\n", nwrite);
    printf("%x ",buf2[0]);
    printf("%x ",buf2[1]);
    printf("%x ",buf2[2]);
    printf("%x ",buf2[3]);
    printf("%x ",buf2[4]);
    printf("%x ",buf2[5]);
    printf("%x ",buf2[6]);
    printf("%x\n",buf2[7]);
    printf("wrote %d bytes already.\n", nwrite);
    printf("============风向问询帧===========\n");
    

    nread = read(fd, buf3, 8);
    printf("============风向应答帧===========\n");
    printf("read %d bytes already.\n", nread);
    printf("%x ",buf3[0]);
    printf("%x ",buf3[1]);
    printf("%x ",buf3[2]);
    printf("%x ",buf3[3]);
    printf("%x ",buf3[4]);
    printf("%x ",buf3[5]);
    printf("%x ",buf3[6]);
    printf("%x ",buf3[7]);
    printf("%x \n",buf3[8]);

    printf("read %d bytes already.\n", nread);
    printf("============风向应答帧===========\n");
    printf("\n\n");
	
}	

void tty_testwindV(int fd, char *buf2, char *buf3)
{

	int nwrite, nread;
	//unsigned int buf;
	

    nwrite = write(fd, buf2, sizeof(buf2));
    printf("===========风速度问询帧===========\n");
    printf("wrote %d bytes already.\n", nwrite);
    printf("%x ",buf2[0]);
    printf("%x ",buf2[1]);
    printf("%x ",buf2[2]);
    printf("%x ",buf2[3]);
    printf("%x ",buf2[4]);
    printf("%x ",buf2[5]);
    printf("%x ",buf2[6]);
    printf("%x\n",buf2[7]);
    printf("wrote %d bytes already.\n", nwrite);
    printf("============风速度问询帧===========\n");
    

    nread = read(fd, buf3, 8);
    printf("============风速度应答帧===========\n");
    printf("read %d bytes already.\n", nread);
    printf("%x ",buf3[0]);
    printf("%x ",buf3[1]);
    printf("%x ",buf3[2]);
    printf("%x ",buf3[3]);
    printf("%x ",buf3[4]);
    printf("%x ",buf3[5]);
    printf("%x ",buf3[6]);

    printf("read %d bytes already.\n", nread);
    printf("============风速度应答帧===========\n");
    printf("\n\n");
	
}	

 
int main(int argc, char *argv[])
{
	int fd;
	int ret;
    
    char buffer[20];
    char buf[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08}; //temp & humid
    char buf1[19];
	//空气质量问询帧
	char buf2[8] = {0x03, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC5, 0xE9};//lux
	char buf3[9];
    //风向
	char buf4[8] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x5E};//°
	char buf5[9];
    //风速
    char buf6[8] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0x8E};//m per s
	char buf7[7];

	memset(buf1, 0x00, sizeof(buf1));

    memset(buf3, 0x00, sizeof(buf3));

    memset(buf3, 0x00, sizeof(buf5));

    memset(buf3, 0x00, sizeof(buf7));

    memset(buffer, 0x00, sizeof(buffer));

    getTime(buffer);


	fd = libtty_open("/dev/ttyUSB0");
	if (fd < 0) {
		printf("libtty_open error.\n");
		exit(0);
	}
	
	ret = libtty_setopt(fd, 4800, 8, 1, 'n');
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}

	tty_testTemp(fd, buf, buf1);
	ret = libtty_close(fd);
    sleep(1);
//    sendtemp(hexTodec(buf1[5], buf1[6])*0.1, hexTodec(buf1[3], buf1[4])*0.1, hexTodec(buf1[7], buf1[8]), hexTodec(buf3[3], buf3[4]), hexTodec(buf3[5], buf3[6]));


	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}


    //****************************************************************************//

    fd = libtty_open("/dev/ttyUSB0");
	if (fd < 0) {
		printf("libtty_open error.\n");
		exit(0);
	}
	
	ret = libtty_setopt(fd, 4800, 8, 1, 'n');
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}

	tty_testPm(fd, buf2, buf3);

	ret = libtty_close(fd);


	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}

    //===================================================================================//
    fd = libtty_open("/dev/ttyUSB0");
	if (fd < 0) {
		printf("libtty_open error.\n");
		exit(0);
	}
	
	ret = libtty_setopt(fd, 4800, 8, 1, 'n');
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}

	tty_testwindV(fd, buf6, buf7);

	ret = libtty_close(fd);


	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}
//===================================================================================//


    //=============================================================================//
    fd = libtty_open("/dev/ttyUSB0");
	if (fd < 0) {
		printf("libtty_open error.\n");
		exit(0);
	}
	
	ret = libtty_setopt(fd, 4800, 8, 1, 'n');
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}

	tty_testwindD(fd, buf4, buf5);

	ret = libtty_close(fd);


	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}

    //读取文件
    // unsigned int length;
    // FILE *fp;
    // char *box;
    // fp = fopen("demo.txt", "rb");
    // if(fp == NULL)
    // {
    //     return FALSE;
    // }
    // fseek(fp, 0, SEEK_END);
    // length = ftell(fp);

    // box = (char *)malloc((length + 1) * sizeof(char));
    // rewind(fp);
    // length = fread(box, 1, length, fp);
    // box[length] = '\0';
    // fclose(fp);
    
    sendtemp(buffer, 
    hexTodec(buf1[5], buf1[6])*0.1, 
    hexTodec(buf1[3], buf1[4])*0.1, 
    hexTodec(buf1[7], buf1[8]), 
    hexTodec(buf3[3], buf3[4]), 
    hexTodec(buf3[5], buf3[6]),
    hexTodec(buf7[3], buf7[4])*0.1,
    hexTodec(buf5[3], buf5[4])
    

    );
//    printf("%s", box);
    return TRUE;
    //******************************************************************************//
}
