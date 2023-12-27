#include <iostream>
#include <windows.h>
#include <wtsapi32.h>



#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <io.h>

#pragma comment(lib, "Wtsapi32.lib")
//#pragma comment(lib, "GenerateRandomFunction.lib")
//int RandomFunction(std::string cppPath, std::string start_marker, std::string end_marker);//来自GenerateRandomFunction
std::vector<std::string> StrSplit(std::string s, char ch)
{
    int start = 0;
    int len = 0;
    std::vector<std::string> ret;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ch) {
            ret.push_back(s.substr(start, len));
            start = i + 1;
            len = 0;
        }
        else {
            len++;
        }
    }
    if (start < s.length())
        ret.push_back(s.substr(start, len));
    return ret;
}
std::string GetProgramDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string currentPath = buffer;
    std::size_t found = currentPath.find_last_of("\\");
    return currentPath.substr(0, found);
}

std::string ReadStrInFile(const std::string& FileName) {
    char filePath[MAX_PATH];
    DWORD pathLength = GetModuleFileNameA(NULL, filePath, MAX_PATH);

    if (pathLength == 0 || pathLength == MAX_PATH) {
        std::cerr << "Failed to get module file path." << std::endl;
        return std::string(); // Return an empty string on failure
    }

    std::string directoryPath(filePath, pathLength);
    size_t lastBackslash = directoryPath.find_last_of("\\");
    if (lastBackslash != std::string::npos) {
        directoryPath = directoryPath.substr(0, lastBackslash + 1); // Include the trailing backslash
    }

    std::string fullPath = directoryPath + FileName;

    HANDLE hFile = CreateFileA(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

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


std::wstring ConvertToWstring(const std::string& narrowString) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(narrowString);


}
bool MoveFileToCurrentDirectory(const std::string& sourcePath, const std::string& newFileName) {
	std::string destinationPath = GetProgramDirectory() + "\\" + newFileName; // TargetFilePath
    std::cout << sourcePath << std::endl;
    std::cout << destinationPath << std::endl;

    if (MoveFileA(sourcePath.c_str(), destinationPath.c_str()))
    {
        //std::cout << "文件移动成功。" << std::endl;
        return true;
    }
    else {
        DWORD error = GetLastError();
        std::cout << "File Move Error. LastError: " << error << std::endl;
        return false;
    }
    
}

int BuildProject(std::string buildName) {

    std::string TempFilePath = GetProgramDirectory() + "\\" + buildName;
	if (_access(TempFilePath.c_str(), 0) == 0)
    {
        DeleteFileA(TempFilePath.c_str());
    }
    std::string fileContents = ReadStrInFile("BuildConfig");
    /*
    * BuildConfig is used to fill in the compilation parameters. Parameter 1 is the compilation command. Parameter 2 is the output file path of successful compilation.
    * same as
    * "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv" "C:\Users\Lazarus\Desktop\Test\Test.sln" /Rebuild "Release"
    *  C:\Users\Lazarus\Desktop\Test\x64\Release\Test.dll
    */
    if (fileContents.empty()) {
        //std::cout << "File contents: " << fileContents << std::endl;
        std::cerr << "Failed to read file." << std::endl;
        return 1;
    }


    std::vector<std::string> BuildParames = StrSplit(fileContents, '\r\n');
    
    std::string commandLine = BuildParames[0];
	std::string BuildFilePath = BuildParames[1].erase(0, 1);
    //std::string RandomFunctionCppPath = BuildParames[2].erase(0, 1);
    //RandomFunction(RandomFunctionCppPath, "//-*-*-*-*-*-*-", "//+/+/+/+/+/+/+/+/+");


 
    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFOA));
    startupInfo.cb = sizeof(STARTUPINFOA);

  
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    // Use CreateProcessA to start devenv
    if (!CreateProcessA(NULL, (char*)commandLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
        std::cerr << "failed to create process." << std::endl;
        return 1;
    }
    
    // wait devenv process finish
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    // get devenv process exit Code
    DWORD exitCode;
    if (GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
        if (exitCode == 0) {
            std::cout << "build success." << std::endl;
        }
        else {
            std::cout << "build faild，exit code: " << exitCode << std::endl;
        }
    }
    else {
        std::cerr << "can not get process exit code." << std::endl;
    }

    // Clean Handle
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    std::cout << "devenv process completed." << std::endl;
    //system("pause");
    return !MoveFileToCurrentDirectory(BuildFilePath, buildName.c_str());

    //return 0;
}

//
//int main()
//{
//    BuildProject("bbbbb.dll");
//}