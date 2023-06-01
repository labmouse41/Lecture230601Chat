#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <iostream>

#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <map>

using namespace std;

#pragma comment(lib, "ws2_32")

//임계영역
map<SOCKET, SOCKADDR_IN> PlayerList;

CRITICAL_SECTION PlayerListCS;

UINT WINAPI ClientThread(void* Arg)
{
	SOCKET ClientSocket = *(SOCKET*)Arg;

	while (1)
	{
		//recv
		char Buffer[1024] = { 0, };
		int RecvByte = recv(ClientSocket, Buffer, sizeof(Buffer), 0);

		if (RecvByte == 0)
		{
			//disconnect
			cout << "Disconnect : " << ClientSocket << endl;
			EnterCriticalSection(&PlayerListCS);
			PlayerList.erase(ClientSocket);
			LeaveCriticalSection(&PlayerListCS);
			break;
		}
		else if (RecvByte < 0)
		{
			cout << "Error Disconnect : " << ClientSocket << endl;
			EnterCriticalSection(&PlayerListCS);
			PlayerList.erase(ClientSocket);
			LeaveCriticalSection(&PlayerListCS);
			break;
		}
		else
		{
			for (auto& Player : PlayerList)
			{
				EnterCriticalSection(&PlayerListCS);
				int SentByte = send(Player.first, Buffer, sizeof(Buffer), 0);
				LeaveCriticalSection(&PlayerListCS);
			}

			cout << "Client Send : " << Buffer << endl;
		}
		//send
	}

	return 0;
}

int main()
{
	WSAData Data;
	WSAStartup(MAKEWORD(2, 2), &Data);

	InitializeCriticalSection(&PlayerListCS);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(40000);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 0);

	while (1)
	{
		SOCKADDR_IN ClientSockAddr;
		memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
		int ClientSockAddrLength = sizeof(ClientSockAddr);
		SOCKET NewClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

		//Client Thread
		//Thread Per Client
		EnterCriticalSection(&PlayerListCS);
		PlayerList[NewClientSocket] = ClientSockAddr;
		LeaveCriticalSection(&PlayerListCS);

		HANDLE ThreadHandle = (HANDLE)_beginthreadex(0, 0, ClientThread, (void*)&NewClientSocket, 0, 0);
	}

	closesocket(ListenSocket);

	DeleteCriticalSection(&PlayerListCS);

	WSACleanup();
	return 0;
}
