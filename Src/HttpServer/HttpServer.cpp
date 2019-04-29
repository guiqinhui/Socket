
#include <stdlib.h>
#include <string.h>
#include <thread>
#include "XTcp.h"
#include <regex>
#define	EPOLL_CTL	1
#define DEFAULT_HTTP_HTML	"/index.html"
#define DEFAULT_FILE_PATH	"www"
using namespace std;
class Tcpthread
{
public:
	XTcp Client;
	void ThreadMain()
	{
		int RecvLen = 0, SendLen = 0;
		char RecvBuf[10240] = { 0 };
		//6.接收客户端数据
		//接收http客户端请求
		printf("Client[%d]\n", Client.SockPort);
		RecvLen = Client.Socket_Recv(RecvBuf, sizeof(RecvBuf) - 1);
		if (RecvLen <= 0)
		{
			Client.Socket_Close();
			printf("Close Client[%d]!\n", Client.SockPort);
			delete this;
			return;
		}
		RecvBuf[RecvLen] = '\0';
		printf("================Recv[%d]================\n", RecvLen);
		printf("%s\n", RecvBuf);
		printf("==================End================\n");
		/*http请求报文，详情参阅https://blog.csdn.net/a19881029/article/details/14002273
			主要由3部分组成：
			1.请求行：也由3部分组成，由空格隔开
				请求方法[空格]URL[空格]协议版本
				请求方法包括：GET、HEAD、PUT、POST、TRACE、OPTIONS、DELETE以及扩展方法
				协议版本的格式：HTTP/主版本号.次版本号，常用的有HTTP/1.0和HTTP/1.1
			2.请求头部
				请求头部为请求报文添加了一些附加信息，由“名/值”对组成，每行一对，名和值之间使用冒号分隔
				请求头部的最后会有一个空行，表示请求头部结束，接下来为请求正文，这一行非常重要，必不可少
			3.请求正文
				此部分可选，例如GET就无此选项
		*/
		//GET /index.html HTTP/1.1		
		//Host: 192.168.3.69
		//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:51.0) Gecko/20100101 Fi
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
		//Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3
		//Accept-Encoding: gzip, deflate
		//DNT: 1
		//Connection: keep-alive
		//Upgrade-Insecure-Requests: 1

		//使用正则表达式解析GET数据
		string src = RecvBuf;
		string pattern = "^([A-Z]+) (.+) HTTP/1";
		regex reg(pattern);
		smatch mas;
		regex_search(src, mas, reg);
		if (mas.size() == 0)
		{
			printf("%s failed!\n", pattern.c_str());
			Close();
			return;
		}
		string CmdType = mas[1];
		string FilePath = mas[2];
		string FileName = FilePath;
		if (CmdType != "GET")
		{
			Close();
			return;
		}
		if (FileName == "/")
		{
			FileName = DEFAULT_HTTP_HTML;//如为根目录，则设置为默认页面路径
		}
		FilePath = DEFAULT_FILE_PATH;
		FilePath += FileName;
		//打开对应页面请求的文件
		FILE *pFile = fopen(FilePath.c_str(),"rb");//以二进制只读方式读取文件
		if (pFile == NULL)
		{
			printf("Open file: %s failed!\n", FilePath.c_str());
			Close();
			return;
		}
		//计算文件大小,有两种方法
		int FileSize = 0;
#if 1
		//1.移位计算法
		fseek(pFile,0,SEEK_END);
		FileSize = ftell(pFile);
		fseek(pFile,0,0);
#else
		//2.获取文件状态信息法
		struct stat FileStat;
		fstat(pFile,&FileStat);
		FileSize = FileStat.st_size;
#endif
		printf("File Size: %d \n", FileSize);
		/*
			HTTP响应报文：由3部分组成，由空格分隔
			1.状态行，由3部分组成
				协议版本号[空格]状态码[空格]状态码描述
				协议版本号：HTTP/主版本号.次版本号，常用的有HTTP/1.0和HTTP/1.1
				状态码：200-299表示成功，300-399表示资源重定向，400-400
			2.响应头部
				与请求报文头部类似，为响应报文添加了一些附加信息
				例如server：服务器应用程序软件的名字和版本号
				Content-Type：响应正文的类型
				Content-Length：响应正文的长度
			3.响应正文
		*/
		//回应HTTP GET 请求
		string RespMsg = "";
		RespMsg = "HTTP/1.1 200 OK\r\n";//状态行 200表示状态码
		RespMsg += "Server:Xhttp\r\n";//服务器端应用程序名称和版本号
		RespMsg += "Content-Type：text/html\r\n";
		RespMsg += "Content-Length：\r\n";
		char bsize[128] = { 0 };
		sprintf(bsize,"%d", FileSize);
		RespMsg += bsize;
		RespMsg += "\r\n\r\n";//最后一行空行表示消息报头结束
		//发送消息报头
		SendLen = Client.Socket_Send(RespMsg.c_str(), RespMsg.size());
		printf("==============sendsize = %d==============\n", SendLen);
		printf("%s\n", RespMsg.c_str());
		printf("===================End===================\n");
		//发送正文
		while (1)
		{
			int len = fread(RecvBuf, 1, sizeof(RecvBuf),pFile);
			if (len <= 0)break;
			int re = Client.Socket_Send(RecvBuf, len);
			if (re <= 0)break;
		}
		Close();
		return;
	}
	void Close(void)
	{
		Client.Socket_Close();
		printf("Close Client[%d]!\n", Client.SockPort);
		delete this;
	}
};
int main(int argc, char* argv[])
{
	unsigned short Sock_Port = 8080;
	XTcp ServerTcp;
	XTcp ClientTcp;
	if (argc > 1)
	{
		Sock_Port = atoi(argv[1]);
	}
	ServerTcp.Sockfd = ServerTcp.Socket_Create();
	ServerTcp.Socket_Bind(Sock_Port);
	for (;;)
	{
		ClientTcp = ServerTcp.Socket_Accetpt();
		Tcpthread *pthread = new Tcpthread();
		pthread->Client = ClientTcp;
		//创建线程
		thread sth(&Tcpthread::ThreadMain,pthread);
		sth.detach();//释放父线程拥有的子线程资源，子线程脱离主线程，不受父线程控制
	}
	ServerTcp.Socket_SetBlock(true);
	ServerTcp.Socket_Close();
	printf("Close Socket!\n");
	getchar();
}
