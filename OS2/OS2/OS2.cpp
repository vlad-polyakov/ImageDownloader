



#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS  
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define FD_CLR
#pragma comment(lib, "ws2_32.lib")
#include <Ws2tcpip.h>
#include "winsock2.h"  
#include <string>  
#include <iostream>  
#include <fstream>  
#include <vector>  
#include<tchar.h>
#include<locale>
#include<codecvt>
#include <time.h>  
#include <queue>  
#include <unordered_set>  
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <limits>
#include <vector>
#include <thread>
using namespace std;

char logFileName[50];
int port = 80;
int ii = 0;

string fullURL = "";

CRITICAL_SECTION csArray;

int makeSocket(string host, int port,string name)
{
	WSADATA inet_WsaData;
	WSAStartup(MAKEWORD(2, 0), &inet_WsaData);
	if (LOBYTE(inet_WsaData.wVersion) != 2 || HIBYTE(inet_WsaData.wVersion) != 0)
	{
		WSACleanup();
		return -1;
	}
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == INVALID_SOCKET)
	{
		cout << "Socket error : " << WSAGetLastError() << endl;
		return tcp_socket;
	}
	struct hostent * hp = ::gethostbyname(host.c_str());
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	memcpy(&saddr.sin_addr, hp->h_addr, 4);
	if (connect(tcp_socket, (const struct sockaddr *)&saddr, sizeof(saddr)) == -1)
	{
		cerr << "error in connect" << endl;
		return tcp_socket;
	}
	string request = "GET " + name + " HTTP/1.1\r\nHost:" + host + "\r\nConnection:Close\r\n\r\n";

	if (send(tcp_socket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
	{
		cout << "send error" << endl;
	}
	return tcp_socket;
}
void WriteLogFile(string file, int bites, string host)
{
	char answer_char[200];
	string answer;
	FILE* pFile = fopen(logFileName, "a");
	if (bites == 0) {
		answer = "<<------------------------------ file " + file + ": 100% ------------------------------>>";
		cout << "<<------------------------------- file " + file + ": 100% ------------------------------>>\n";
	}
	else answer = "File: " + file + ",  Host: " + host + ", Bites: " + to_string(bites);
	for (int j = 0; j < 200; j++) {

		if (j < answer.length()) answer_char[j] = answer[j];
		else answer_char[j] = '\0';
	}

	fprintf(pFile, "%s\n", answer_char);
	fclose(pFile);
	
}

string * NormalURL(string url1) {
	int k = 0;
	int j = 0;
	string normUrl;
	string *urlAndName = new string[2];
	if (url1.substr(0, 7) == "http://") url1.erase(0, 7);
	else if (url1.substr(0, 8) == "https://") url1.erase(0, 8);
	int pos = url1.find_first_of("/");
	urlAndName[0] = url1.substr(0, pos);
	urlAndName[1] = url1.substr(pos);
	return urlAndName;
}
string FileName(string dir)
{
	string search = "/";
	int pos = 0;
	while ((pos = dir.find(search, pos)) != string::npos) {
		dir.replace(pos, search.size(), "_");
		pos++;
	}
	return dir;
}
int download(int client_socket,string name) {
	fstream file;
	string fileName = FileName(name);
	int size = 0;
	file.open(fileName, ios::out | ios::binary);
	char buf[1024];
	::memset(buf, 0, sizeof(buf));
	int n = 0;
	n = recv(client_socket, buf, sizeof(buf) - sizeof(char), 0);
	size += n;
	char* cpos = strstr(buf, "\r\n\r\n");
	file.write(cpos + strlen("\r\n\r\n"), n - (cpos - buf) - strlen("\r\n\r\n"));
	memset(buf, 0, sizeof(buf));
	while ((n = recv(client_socket, buf, sizeof(buf) - 1, 0)) > 0)
	{
		try
		{
			file.write(buf, n);
			size += n;
		}
		catch (...)
		{
			cout << "ERROR" << endl;
		}
	
	}
	file.close();
	closesocket(client_socket);
	return size;
}
void ThreadRun(string name) {
	string *urlAndName = NormalURL(name);
	int size = download(makeSocket(urlAndName[0], port, urlAndName[1]), urlAndName[1]);
	EnterCriticalSection(&csArray);
	WriteLogFile(urlAndName[1], size, urlAndName[0]);
	LeaveCriticalSection(&csArray);
	
}

int main()
{
	try {
		vector<thread> threads;
		time_t seconds = time(NULL);
		tm* timeinfo = localtime(&seconds);
		char* format = "./LogFiles/LogFile %H.%M.%S %d %b %Y.txt";
		strftime(logFileName, 80, format, timeinfo);
		InitializeCriticalSection(&csArray);
		string str;
		ifstream link("text.txt", ios::in);

		while (!link.eof()) {
			getline(link, str);
			threads.push_back(thread(ThreadRun, str));
		}
		for (int index = 0; index < threads.size(); index++) {
			if (threads[index].joinable()) {
				threads[index].join();
			}
		}

		DeleteCriticalSection(&csArray);
		link.close();
	}
	catch (exception ex) { cout << "ddfddf"; }
		system("pause");
		return 0;
	
}

