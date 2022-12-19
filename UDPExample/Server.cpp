#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include<deque>
#include<fstream>
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") // Winsock library
#pragma warning(disable:4996) // ��������� �������������� _WINSOCK_DEPRECATED_NO_WARNINGS

SOCKET server_socket;

vector<string> history;

struct Client {
	SOCKET socket;
	string nickname;
	int color;
};

deque<Client> ClientQ;

int main() {
	history.push_back("Chat was started");

	system("title Server");

	puts("Start server... DONE.");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}
	// puts("Create socket... DONE.");

	// prepare the sockaddr_in structure
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind socket
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	// puts("Bind socket... DONE.");

	// ������� �������� ����������
	listen(server_socket, MAX_CLIENTS);

	// ������� � �������� ����������
	puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

	// ������ ������ ��������� ������, ��� ����� ������
	// ����� ������������ �������
	// fd means "file descriptors"
	fd_set readfds; // https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set

	while (true) {
		// �������� ����� fdset
		FD_ZERO(&readfds);

		// �������� ������� ����� � fdset
		FD_SET(server_socket, &readfds);

		// �������� �������� ������ � fdset
		for (int i = 0; i < ClientQ.size(); i++)
		{
			SOCKET s = ClientQ[i].socket;
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// ��������� ���������� �� ����� �� �������, ����-��� ����� NULL, ������� ����� ����������
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// ���� ���-�� ��������� �� ������-������, �� ��� �������� ����������
		Client obj; // ����� ���������� �����
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((obj.socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}

			send(obj.socket, "Enter your nickname", 20, 0);
			char client_message[DEFAULT_BUFLEN];

			int client_message_length = recv(obj.socket, client_message, DEFAULT_BUFLEN, 0);
			client_message[client_message_length] = '\0';

			obj.nickname = client_message;


			send(obj.socket, "Enter your nickname color code", 20, 0);
			char client_message[DEFAULT_BUFLEN];

			int client_message_length = recv(obj.socket, client_message, DEFAULT_BUFLEN, 0);
			client_message[client_message_length] = '\0';

			obj.color = atoi(client_message);


			string NewClientMessage = "\v";
			for (int i = 0; i < obj.nickname.size(); i++) {
				NewClientMessage.push_back(obj.nickname[i]);
			}
			NewClientMessage.push_back(' ');
			char join[] = "joined";
			for (int i = 0; i < strlen(join); i++) {
				NewClientMessage.push_back(join[i]);
			}
			NewClientMessage.push_back('\n');
			
			for (int i = 0; i < ClientQ.size(); i++) 
			{
				if (ClientQ[i].socket != 0) 
				{
					send(ClientQ[i].socket, NewClientMessage.c_str(), NewClientMessage.size(), NULL);
				}
			}
			

			for (int i = 0; i < history.size(); i++)
			{
				cout << history[i] << "\n";
				send(obj.socket, history[i].c_str(), history[i].size(), 0);
			}
			history.push_back(NewClientMessage);
			// ������������� ��������� ������� � ������ ������ - ������������ � �������� �������� � ���������
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", obj.socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			// �������� ����� ����� � ������ �������
			ClientQ.push_back(obj);
		}

		// ���� �����-�� �� ���������� ������� ���������� ���-��
		for (int i = 0; i < ClientQ.size(); i++)
		{
			SOCKET s = ClientQ[i].socket;
			// ���� ������ ������������ � ������� ������
			if (FD_ISSET(s, &readfds))
			{
				// �������� ��������� �������
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);

				// ���������, ���� �� ��� �� ��������, � ����� ���������� �������� ���������
				// recv �� �������� ������� ���������� � ����� ������ (� �� ����� ��� printf %s ������������, ��� �� ����)

				char client_message[DEFAULT_BUFLEN];

				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';

				string check_exit = client_message;
				if (check_exit == "off")
				{
					cout << "Client #" << i << " is off\n";
					deque<Client>::iterator iter = ClientQ.begin();
					for ( int j = 0; j < i; j++) iter++;
					ClientQ.erase(iter);
				}
				string temp= ClientQ[i].nickname;
				temp.push_back(':');
				temp.push_back(' ');
				for (int i = 0; i < strlen(client_message); i++)
				{
					temp.push_back(client_message[i]);
				}
				
				history.push_back(temp);

				for (int i = 0; i < ClientQ.size(); i++) {
					if (ClientQ[i].socket != 0) {
						send(ClientQ[i].socket, history.back().c_str(), history.back().size(), 0);
					}
				}

			}
		}
	}

	WSACleanup();
}