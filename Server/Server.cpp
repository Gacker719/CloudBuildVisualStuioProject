#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"BuildProject.lib")
#define PORT 1234
int BuildProject(std::string buildName);//From BuildProject
std::string GetProgramDirectory();//From BuildProject
bool CheckFileName(std::string filename)
{
    bool result = true;
    for (size_t i = 0; i < filename.size(); i++)
    {
        if (isalnum(filename[i]) == 0)
        {
            //std::cout << "i:" << i << " NotNumerOrAlapha" << std::endl;
            result = false;
            break;
        }
    }
    return result;
}
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup error。" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "error：cannot create socket。" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "xxx.xxx.84.218", &(serverAddr.sin_addr));  // Server IP

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "error：blind error。" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "error: cannot listen。" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "server is listen port: " << PORT << "..." << std::endl;

    while (true) {
        SOCKET newSocket = accept(serverSocket, NULL, NULL);
        if (newSocket == INVALID_SOCKET) {
            std::cerr << "error：cannot accpt client。" << std::endl;
        }
        else {
            // get client IP 
            sockaddr_in clientAddress;
            int addrSize = sizeof(clientAddress);
            if (getpeername(newSocket, (sockaddr*)&clientAddress, &addrSize) == 0) {
                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
                //std::cout << "client IP: " << clientIP << std::endl;
				if (strcmp(clientIP, "xxx.xxx.9.228") != 0)//check ip is white
                {
                    printf( "[*]Hack IP in:%s\n", clientIP);// 

                    closesocket(newSocket);
                    continue;
                }
            }
            else {
                //std::cerr << "cannot get client IP." << std::endl;
                closesocket(newSocket);
                continue;
            }
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            int bytesRead = recv(newSocket, buffer, sizeof(buffer), 0);
            std::string fileName(buffer);
            if (!CheckFileName(fileName))
            {
                printf("[*]Hack File Path:%s\n", fileName.c_str());// 
                closesocket(newSocket);
                continue;
            }

			if (BuildProject(fileName) != 0)
			{
				const char* notFoundMessage = "CloudBuildFaild";
				send(newSocket, notFoundMessage, strlen(notFoundMessage), 0);
			}
			else
            {
                //根目录文件
                fileName = GetProgramDirectory() + "\\" + fileName;
				std::cout << "file is sending：" << fileName << " file access:" << _access(fileName.c_str(), 0) << std::endl;

                //发送文件
                std::ifstream file(fileName, std::ios::in | std::ios::binary);
                if (file.is_open()) {
                    file.seekg(0, std::ios::end);
                    int fileSize = file.tellg();
                    file.seekg(0, std::ios::beg);

                    char* fileData = new char[fileSize];
                    file.read(fileData, fileSize);

                    send(newSocket, fileData, fileSize, 0);
                    delete[] fileData;
                    file.close();
                    std::cout << "file sent：" << fileName << std::endl;
                }
                else {
                    const char* notFoundMessage = "FileNotFound";
                    send(newSocket, notFoundMessage, strlen(notFoundMessage), 0);
                }
            }



            closesocket(newSocket);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
