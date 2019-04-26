#include "XTcp.h"

int main(int argc, char * argv[])
{
	XTcp tcpClient;
	tcpClient.Socket_Create();
	tcpClient.Socket_Connect("10.0.2.15",8080);
	getchar();
}