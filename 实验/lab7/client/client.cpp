#pragma comment(lib,"ws2_32.lib")
#include <cstdio>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <conio.h>

#define MAXBUFFSIZE 10000

using namespace std;

SOCKET servSock;    //套接字
bool connect_status;    //连接状态
char user_name[10];    //用户名
HANDLE hConsole;    //用于更改控制台输出颜色
struct sockaddr_in ServerAddr;  //服务器地址信息    
HANDLE hThread; //监听线程句柄
int user_id;
int cnt=0;  //计数器，用来完成实验报告中的内容



void _init_();  //初始化
void PrintSystemPrefix();   //即[system]
void PrintUserPrefix();    //即[user]
void PrintServerPrefix();   //即[server]
void PrintMenu();   //Help菜单
void Handle();  //对用户输入进行处理
void UserList();    //向server端获取user_list
void Send();    //向server端发送数据
void GetName(); //从server端获取user_name
bool Connect(); //建立连接
void Disconnect();  //断开连接
void GetTime(); //获取时间
void Exit();    //退出
void CustomizeRead(char Message[], int maxSize);    //读空格，不读换行符
DWORD WINAPI RecvHandleThread(LPVOID lpParameter);  //接受数据子线程





int main()
{
    _init_();
    system("pause");
}

void _init_()
{
    connect_status=false;   //初始化成未连接
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  //用于更改控制台输出颜色
    FILE* fp=fopen("user", "r");
    
    if(!fp){
        //初始化运行，输入用户名
        PrintSystemPrefix();
        printf("Please enter your user name:");
        scanf("%s", user_name);
        fp=fopen("user", "w");
        fwrite(user_name, sizeof(user_name), 1, fp);
        fclose(fp);
    }
    else{
        //已经运行过，直接读取用户名
        fscanf(fp, "%s", user_name);
        fclose(fp);
    }
    PrintSystemPrefix();
    printf("Hello, ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE); printf("%s", user_name);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); printf("!\n");
    PrintMenu();
}

void PrintSystemPrefix()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN); 
    printf("[System]");
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void PrintUserPrefix()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE); //蓝色
    printf("[%s]", user_name);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
}

void PrintServerPrefix()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
    printf("[Server]");
}

void PrintMenu() {
    PrintSystemPrefix();
	printf("Supported functions are as follows:\n");
	printf("        +---------------------+-------------------------------+\n");
	printf("        |       Command       |            Function           |\n");
	printf("        +---------------------+-------------------------------+\n");
	if (connect_status) {
		printf("        |         -c          | Connect to the chosen server. |\n");
		printf("        |         -d          | Disconnect from the server.   |\n");
		printf("        |         -t          | Get current time.             |\n");
		printf("        |         -n          | Get your user name.           |\n");
		printf("        |         -l          | Show the user list.           |\n");
		printf("        |         -s          | Send message to chosen user.  |\n");
		printf("        |         -e          | Disconnect and Exit.          |\n");
        printf("        |         -h          | Get help.                     |\n");
	}
	else {
		printf("        |         -c          | Connect to the chosen server. |\n");
		printf("        |         -e          | Exit.                         |\n");
        printf("        |         -h          | Get help.                     |\n");
	}
	printf("        +---------------------+-------------------------------+\n");
    PrintUserPrefix();
    Handle();
}

void Handle()
{
    char str[100];
    while(1){
        scanf("%s", str);
        if(str[0]!='-'){
            PrintSystemPrefix();
            printf("Not a command.\n");
        }
        else{
            char command=str[1];
            switch(command){
                case 'c':
                    connect_status=Connect();
                    break;
                case 'd':
                    Disconnect();
                    PrintUserPrefix();
                    break;
                case 't':
                    GetTime();
                    break;
                case 'n':
                    GetName();
                    break;
                case 'l':
                    UserList();
                    break;
                case 's':
                    Send();
                    break;
                case 'e':
                    Exit();
                case 'h':
                    PrintMenu();
                    break;
                default:
                    PrintSystemPrefix();
                    printf("Not a command.\n");
                    PrintUserPrefix();
                    break;
            }
        }
    }
}

bool Connect(){
    if(connect_status){
        //已经连接了    
        PrintSystemPrefix();
        printf("You have already been connected.\n");
        PrintUserPrefix();
        return true;
    }   
    char ServerIp[20];
    int ServerPort;
    //初始化dll
    WSADATA wsaData;
    int StartUp=WSAStartup(MAKEWORD(2, 2), &wsaData);   
    if(StartUp){
        //初始化不成功
        PrintSystemPrefix();
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("WSAStartUp error.\n");
        system("pause");
        exit(-1);
    }   
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
        //WinSock DLL不支持2.2版本
		WSACleanup();
        PrintSystemPrefix();
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED); 
        printf("Invalid Winsock version");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf(".\n");
		system("pause");
		exit(-1);
	}
    //创建套接字 ipv4地址 基于流 TCP协议
    servSock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(servSock==INVALID_SOCKET){
        //非法socket
        WSACleanup();
        PrintSystemPrefix();
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED); printf("Socket error");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf(".\n");
		system("pause");
		exit(-1);
    }

    PrintSystemPrefix();
    printf("Please enter server's ip: ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
    scanf("%s", ServerIp);
    PrintSystemPrefix();
    printf("Please enter server's port: ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
    scanf("%d", &ServerPort);
    //绑定套接字
    memset(&ServerAddr, 0, sizeof(ServerAddr)); //用0填充
    ServerAddr.sin_family=AF_INET;  //使用ipv4地址
    ServerAddr.sin_addr.S_un.S_addr=inet_addr(ServerIp);    //服务器IP
    ServerAddr.sin_port=htons(ServerPort);  //端口 小端序转化成网络字节序（大端序）
    if(connect(servSock, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr))==SOCKET_ERROR){
        //连接失败
        PrintSystemPrefix();
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Connection failed");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf(".\n");
        return false;
    }
    else{
        //连接成功
        PrintSystemPrefix();
        printf("Success to connect to server ");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        printf("%s", ServerIp);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf(".");
        hThread=CreateThread(NULL, 0, &RecvHandleThread, &servSock, 0, NULL);  
        //发送请求包，拿到自己的id
        char SendBuff[2]="c";
        int status=send(servSock, SendBuff, sizeof(SendBuff), 0);
        connect_status=true;
        return true;
    }
}

DWORD WINAPI RecvHandleThread(LPVOID lpParameter)
{
    SOCKET* clntSock=(SOCKET*)lpParameter;
    char RecvBuff[MAXBUFFSIZE];
    while(1){
        int n=0;
        n=recv(* clntSock, RecvBuff, sizeof(RecvBuff), 0);
        if(n>0){
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
            char command=RecvBuff[0];
            switch(command){
                case 'e':
                    //服务器对转发包的反馈
                    printf("\n");
                    PrintServerPrefix();
                    printf("%s\n", (char*)(RecvBuff+1));
                    PrintUserPrefix();
                    break;
                case 't':
                    PrintServerPrefix();
                    printf("No.%d Current time: %s", cnt, (char*)(RecvBuff+1));
                    cnt++;
                    PrintUserPrefix();
                    break;
                case 'n':
                    PrintServerPrefix();
                    printf("Server's PC name: %s.\n", (char*)(RecvBuff+1));
                    PrintUserPrefix();
                    break;
                case 'l':{
                    PrintServerPrefix();
                    int numClients=RecvBuff[1];
                    printf("Number of clients coonnected to the server: %d\n", numClients);
                    printf("        +----------------------+\n");
                    printf("        |     User Id List     |\n");
                    printf("        +----------------------+\n");
                    int offset=2;
                    for(int i=0; i<numClients; i++){
                        char usr_id[4];
                        memcpy(usr_id, RecvBuff+offset, 3);
                        usr_id[3]='\0';
                        printf("        |         %d          |\n", atoi(usr_id));
                        offset+=3;
                    }
                    printf("        +----------------------+\n");
                    PrintUserPrefix();
                    break;
                }
                case 's':{
                    Sleep(20);
                    printf("\n");
                    PrintServerPrefix();
                    char src_id[4];
                    memcpy(src_id, RecvBuff+1, 3);
                    printf("Message from [%s]: %s\n", src_id, (char*)(RecvBuff+4));
                    PrintUserPrefix();
                    break;
                }
                case 'c':{
                    printf("\n");
                    PrintServerPrefix();
                    char id[10];
                    strcpy(id, RecvBuff+1);
                    printf("User ID: %s.\n", id);
                    user_id=stoi(id);
                    PrintUserPrefix();
                    break;
                }
                default:
                    break;
            }
        }
    }
}


void Exit()
{
    if(connect_status){
        //关闭套接字
        closesocket(servSock);
        //终止dll
        WSACleanup();
        //关闭句柄
        CloseHandle(hThread);
        PrintSystemPrefix();
        printf("Success to discnnect and exit. Bye~\n");
        connect_status=false;
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("Success to exit. Bye~\n");
    }
    system("pause");
    exit(0);
}

void Disconnect()
{
    if(connect_status){
        int res=shutdown(servSock, SD_SEND);
        if(res==SOCKET_ERROR){
            //动作失败
            PrintSystemPrefix();
            printf("Fail to disconnect\n.");
            WSAGetLastError();
            //关闭套接字
            closesocket(servSock);
            //终止dll使用
            WSACleanup();
        }
        PrintSystemPrefix();
        printf("Success to disconnect from server.\n");
        connect_status=false;
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("No server connected.\n");
    }
}

void GetTime()
{
    if(connect_status){
        char SendBuff[2]="t";
        int status=send(servSock, SendBuff, sizeof(SendBuff), 0);
        if(status<0){
            PrintSystemPrefix();
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Send failed");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            printf(".\n");
            return;
        }
        else{
            //成功发送
            return;
        }
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("No server connected.\n");
        PrintUserPrefix();
    }
}

void GetName()
{
    if(connect_status){
        char SendBuff[2]="n";
        int status=send(servSock, SendBuff, sizeof(SendBuff), 0);
        if(status<0){
            PrintSystemPrefix();
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Send failed");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            printf(".\n");
            return;
        }
        else{
            //成功发送
            return;
        }
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("No server connected.\n");
        PrintUserPrefix();
    }
}

void UserList()
{
    if(connect_status){
        char SendBuff[2]="l";
        int status=send(servSock, SendBuff, sizeof(SendBuff), 0);
        if(status<0){
            PrintSystemPrefix();
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Send failed");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            printf(".\n");
            return;
        }
        else{
            //成功发送
            return;
        }
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("No server connected.\n");
        PrintUserPrefix();
    }
}

void Send()
{
    if(connect_status){
        int _ReceiverId;
        char Message[MAXBUFFSIZE];
        char SendBuff[MAXBUFFSIZE+12]="s";
        char ReceiverId[10];
        //读取receiver id
        PrintSystemPrefix();
        printf("Please enter receiver's id: ");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
        scanf("%d", &_ReceiverId);
        getchar();
        _itoa(_ReceiverId, ReceiverId, 10);
        //拼接"&"来区分id和message
        strcat(ReceiverId, "&");
        //读取发送信息
        PrintSystemPrefix();
        printf("Please enter message to be sent: ");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);   //黄色
        //要忽略空格，否则读不全
        CustomizeRead(Message, MAXBUFFSIZE);
        strcat(SendBuff, ReceiverId);
        strcat(SendBuff, Message);
        int status=send(servSock, SendBuff, strlen(SendBuff), 0);
        if(status<0){
            PrintSystemPrefix();
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Send failed");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            printf(".\n");
            return;
        }
    }
    else{
        //尚未连接
        PrintSystemPrefix();
        printf("No server connected.\n");
        PrintUserPrefix();
    }
}

void CustomizeRead(char Message[], int maxSize) {
    int index=0;
    char ch;
    while (index<maxSize&&cin.get(ch)&&ch!='\n') {
        Message[index++]=ch;
    }
    Message[index]='\0'; 
}
