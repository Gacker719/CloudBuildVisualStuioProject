#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#pragma comment(lib,"ws2_32.lib")

#define SERVER_IP "xxx.xxx.84.218"
#define PORT 4567
std::string GetProgramDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string currentPath = buffer;
    std::size_t found = currentPath.find_last_of("\\");
    return currentPath.substr(0, found);
}
std::string GenerateRandomAlphaString(int length, bool onlyAlpha = false) {
    static const char alphaCharset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const char alphaNumericCharset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static const int charsetSize = onlyAlpha ? sizeof(alphaCharset) - 1 : sizeof(alphaNumericCharset) - 1;

    const char* charset = onlyAlpha ? alphaCharset : alphaNumericCharset;

    std::string randomString;
    for (int i = 0; i < length; ++i)
    {
        randomString += charset[std::rand() % charsetSize];
    }

    return randomString;
}
std::string ReadStrInFile(const std::string& FileName) {


    HANDLE hFile = CreateFileA(FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file." << std::endl;
        return std::string(); // Return an empty string on failure
    }

    char tempBuf[4096];
    DWORD bytesRead;
    if (!ReadFile(hFile, tempBuf, 4096, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return std::string(); // Return an empty string on failure
    }

    tempBuf[bytesRead] = '\0';
    CloseHandle(hFile);
    return std::string(tempBuf);
}
bool MoveFileToCurrentDirectory(const std::string& sourcePath, const std::string& newFileName) {
    std::string destinationPath = GetProgramDirectory() + "\\" + newFileName; // Target File Path
    std::cout << sourcePath << std::endl;
    std::cout << destinationPath << std::endl;

    if (MoveFileA(sourcePath.c_str(), destinationPath.c_str()))
    {
        std::cout << "File Move Success。" << std::endl;
        return true;
    }
    else {
        DWORD error = GetLastError();
        std::cout << "File Cannot Move. LastCode: " << error << std::endl;
        return false;
    }

}
int main(int argc, char** argv)
{



    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup Error。" << std::endl;
        return 1;
    }
    else {
        std::cerr << "WSAStartup Success。" << std::endl;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error：Cannot Create socket。" << std::endl;
        WSACleanup();
        return 1;
    }
    else {
        std::cerr << "Create socket Success。" << std::endl;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr));  // ServerIP Address
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error：Cannot connect to server。" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    else {
        std::cerr << "Server connect。" << std::endl;
    }

    std::string fileName;
	if (argc == 1)
    {
        std::cout << "CloudFileName：";
        std::cin >> fileName;
    }
    else
    {
        fileName = argv[1];
    }


    send(clientSocket, fileName.c_str(), fileName.size(), 0);
    fileName = GetProgramDirectory() + "\\" + fileName;
	if (_access(fileName.c_str(), 0) == 0)
    {
        DeleteFileA(fileName.c_str());
    }
    std::cout << "FileName:" << fileName << std::endl;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
	std::string tempFileName = fileName + GenerateRandomAlphaString((rand() % (6 - 3)) + 3 + 1);
    std::ofstream file(tempFileName, std::ios::out | std::ios::binary);


    char buffer[4096];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesRead);
    }
    size_t fileSize = file.tellp();
    file.close();

	if (fileSize == 13)
    {
		if (ReadStrInFile(fileName).compare("FileNotFound") == 0)
        {
            std::cout << "FileNotFounded" << std::endl;
            int returnCode = 2;
        }
    }
    else if (fileSize == 16)
    {
        if (ReadStrInFile(fileName).compare("CloudBuildFaild") == 0)
        {
            std::cout << "CloudBuildError" << std::endl;
            int returnCode = 13;
        }
    }
    else
    {
        std::cout << "FileAcceptSuccesful :" << fileName << std::endl;
        
        if (std::rename(tempFileName.c_str(), fileName.c_str()) == 0) {
            std::cout << "File renamed successfully." << std::endl;
        }
        else {
            std::cerr << "File rename failed." << std::endl;
            return 1;
        }
 
    }
    //std::cout << file.tellp() << std::endl;


    int returnCode = 0;


    closesocket(clientSocket);
    WSACleanup();
    //system("pause");
    return returnCode;
}
