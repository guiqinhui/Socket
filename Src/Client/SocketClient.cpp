#include "XTcp.h"

int main(int argc, char * argv[])
{
	char SendBuf[] = { "Client connect\r\n" };
	char RecvBuf[1024] = { 0 };
	XTcp tcpClient;
	tcpClient.Socket_Create();
	tcpClient.Socket_Connect("10.0.2.15",8080);
	tcpClient.Socket_Send(SendBuf,sizeof(SendBuf));
	tcpClient.Socket_Recv(RecvBuf,sizeof(RecvBuf));
	printf("RecvSever:%s\r\n", RecvBuf);
	getchar();
}