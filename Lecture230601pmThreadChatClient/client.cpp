#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32")

using namespace std;

SOCKET ServerSocket;

UINT WINAPI ReceiveThread(void* Arg)
{
	while (1)
	{
		char Message[1024] = { 0, };
		int RecvByte = recv(ServerSocket, Message, sizeof(Message), 0);
		cout << "Server Send : " << Message << endl;
	}
	return 0;
}


int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	ZeroMemory(&ServerSockAddr, sizeof(ServerSockAddr));
	//memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = PF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("192.168.3.190");
	ServerSockAddr.sin_port = htons(40000);

	connect(ServerSocket, (SOCKADDR*)(&ServerSockAddr), sizeof(ServerSockAddr));

	HANDLE ThreadHandle = (HANDLE)_beginthreadex(0, 0, ReceiveThread, 0, 0, 0);

	while (1)
	{
		char Buffer[1024] = { 0, };
		cin.getline(Buffer, sizeof(Buffer));
		int SentByte = send(ServerSocket, Buffer, (int)strlen(Buffer), 0);
	}



	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}