#pragma comment(lib,"ws2_32.lib")
#include <cstdio>
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ctime>
#include <vector>

#define MaxClient 10
#define MaxBufSize 1024

using namespace std;

const int PORT = 5815;                         // 设置监听端口为5815
using client = pair<SOCKET*, int>;
vector<client> clients;                        // 已连接的用户列表
HANDLE hConsole;                               // 用于更改控制台输出颜色

void PrintSystemPrefix();                      // 即[System]
void PrintInitialPrefix();                     // 即[Initialize]
DWORD WINAPI ServerThread(LPVOID lpParameter); // 服务线程

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // 用于更改控制台输出颜色
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WSAData wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    PrintInitialPrefix();
	cout << "Socket initialized!" << endl;
	SOCKADDR_IN ListenAddr;
	ListenAddr.sin_family = AF_INET;
	ListenAddr.sin_addr.S_un.S_addr = INADDR_ANY; // 表示填入本机ip
	ListenAddr.sin_port = htons(PORT);
	int n;
	n = bind(ListenSocket, (LPSOCKADDR)&ListenAddr, sizeof(ListenAddr));
	if (n == SOCKET_ERROR) {
        PrintInitialPrefix();
		cout << "Bind failed!" << endl;
		return -1;
	}
	else {
        PrintInitialPrefix();
		cout << "Bind port ";
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        cout << PORT;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << " successful!" << endl;
	}
	int l = listen(ListenSocket, 20);
    PrintInitialPrefix();
	cout << "Waiting for clients ..." << endl;

	while (1) {
		// 循环接收客户端连接请求并创建服务线程
		SOCKET* ClientSocket = new SOCKET;
		ClientSocket = (SOCKET*)malloc(sizeof(SOCKET));
		// 接收客户端连接请求
		int SockAddrlen = sizeof(sockaddr);
		*ClientSocket = accept(ListenSocket, 0, 0);
		clients.push_back(make_pair(ClientSocket, int(*ClientSocket)));

        PrintSystemPrefix();
		cout << "Client(";
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        cout << *ClientSocket;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << ") has been connected" << endl;
		CreateThread(NULL, 0, &ServerThread, ClientSocket, 0, NULL);
	}
	closesocket(ListenSocket);
	WSACleanup();
}

void PrintSystemPrefix()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN); // 绿色
    cout << "[System] ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void PrintInitialPrefix()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN); // 黄色
    cout <<"[Initialize] ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

DWORD WINAPI ServerThread(LPVOID lpParameter) {
	SOCKET* ClientSocket = (SOCKET*)lpParameter;
	int receByt = 0;
	char RecvBuf[MaxBufSize];
	char SendBuf[MaxBufSize];
	while (1) {
		receByt = recv(*ClientSocket, RecvBuf, sizeof(RecvBuf), 0);
		if (receByt > 0) {
            PrintSystemPrefix();
			cout << "Receive message: ";
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
            cout << RecvBuf[0];
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << " -- from client ";
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
            cout << *ClientSocket << endl;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			switch (RecvBuf[0])
			{
			case 't': // 服务端所在机器的当前时间
			{
				time_t now = time(0); // 把 now 转换为字符串形式
				char* dt = ctime(&now);
                PrintSystemPrefix();
				cout << "Local date & time: ";
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                cout << dt;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				SendBuf[0] = 't';
				strcpy((SendBuf + 1), dt);

				int k = 0;
				k = send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				if (k < 0) {
                    PrintSystemPrefix();
					cout << "Send failed!" << endl;
				}

				memset(SendBuf, 0, sizeof(SendBuf));
				break;
			}
			case 'n': // 服务端所在机器的名称
			{
				DWORD ComputerNameLength = 100;
				strcpy(SendBuf, "n");

				TCHAR t_name[100];
				GetComputerNameW((LPWSTR)t_name, &ComputerNameLength); // 获取本机名称
				char name[100];
				wcstombs(name, (wchar_t *)t_name, 100);
				strcat(SendBuf, name);

				int k = 0;
				k = send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				if (k < 0) {
                    PrintSystemPrefix();
					cout << "Send failed!" << endl;
				}
                PrintSystemPrefix();
				cout << "Server's PC name: ";
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                cout << SendBuf + 1 << endl;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

				memset(SendBuf, 0, sizeof(SendBuf));
				break;
			}
			case 'l': // 当前连接的所有客户端信息
			{
                cout << "         User id list:";
				for (auto iter : clients)
				{
					cout << " " << iter.second;
				}
				cout << endl;
				SendBuf[0] = 'l';
				SendBuf[1] = clients.size();
				char Buf[4] = { 0 };
				for (auto iter : clients)
				{
					char Buf[4] = { 0 };
					itoa(iter.second, Buf, 10);
					strcat(SendBuf, Buf);
				}

				int k = 0;
				k = send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				if (k < 0) {
                    PrintSystemPrefix();
					cout << "Send failed!" << endl;
				}

				memset(SendBuf, 0, sizeof(SendBuf));
				break;
			}
			case 's':
			{
				char dst_port[10];
				// 提取接收方id
				int idx = 1;
				while (RecvBuf[idx] != '&') {
					dst_port[idx - 1] = RecvBuf[idx];
					idx++;
				}

				int dst_port_id = atoi(dst_port);
				SOCKET* dst_client = NULL;

				for (auto iter : clients)
				{
					if (iter.second == dst_port_id)
					{
						dst_client = iter.first;
						break;
					}
				}
				if (!dst_client)
				{
                    PrintSystemPrefix();
					cout << "Client(";
                    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
                    cout << dst_port_id;
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    cout << ") is not connected" << endl;
					// 发个反馈包
					strcpy(SendBuf, "eInvalid receiver ID!");
					send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					break;
				}

				memset(SendBuf, 0, sizeof(SendBuf));
				SendBuf[0] = 's';
				char Buf[4] = { 0 };
				for (auto iter : clients)
				{
					if (iter.first == ClientSocket)
					{
						itoa(iter.second, Buf, 10);
						strcat(SendBuf, Buf);
						break;
					}
				}
				strcat_s(SendBuf, RecvBuf + idx + 1);

				int k = 0;
				k = send(*dst_client, SendBuf, sizeof(SendBuf), 0);
				if (k < 0) {
                    PrintSystemPrefix();
					cout << "Send failed!" << endl;
				}
                PrintSystemPrefix();
				cout << "Sender(";
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
                cout << *ClientSocket;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                cout << ") -- Receiver(";
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
                cout << *dst_client << ")" << endl;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				cout << "         Message is: ";
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                cout << SendBuf + 4 << endl;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				// 发个反馈包
				strcpy(SendBuf, "eSuccessfully sent!");
				send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				memset(SendBuf, 0, sizeof(SendBuf));
				break;
			}
			case 'c': // 返回用户id
			{
				strcpy(SendBuf, "c");
				int id = *ClientSocket;
				char usr_id[10];
				strcat(SendBuf, itoa(id, usr_id, 10));
				int k = 0;
				k = send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				if (k < 0) {
                    PrintSystemPrefix();
					cout << "Send failed!" << endl;
				}

				memset(SendBuf, 0, sizeof(SendBuf));
				break;
			}
			default:
				cout << "Undefined Behavior." << endl;
				break;
			}
		}
		else
		{
            PrintSystemPrefix();
			cout << "Client(";
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
            cout << *ClientSocket;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << ") has been disconnected" << endl;

			for (auto iter = clients.begin(); iter != clients.end(); iter++) {
				if ((*iter).first == ClientSocket) {
					iter = clients.erase(iter);
					break;
				}
			}
			break;
		}

		memset(RecvBuf, 0, sizeof(RecvBuf));
	}
	closesocket(*ClientSocket);
	free(ClientSocket);
	return 0;
}