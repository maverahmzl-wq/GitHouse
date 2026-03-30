#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <chrono>
#include <map>
#include <vector>

// Sürüm ve Yeni Klasör Yapısına Göre Repo Bilgisi
const std::string GITHOUSE_VERSION = "8.1.0-Stable";
const std::string REPO_EXE_URL = "https://github.com/maverahmzl-wq/GitHouse/raw/main/githouse/bin/githouse.exe";

// Market Linkleri
std::map<std::string, std::string> market = {
    {"python", "https://www.python.org/ftp/python/3.12.2/python-3.12.2-amd64.exe"},
    {"nodejs", "https://nodejs.org/dist/v20.11.1/node-v20.11.1-x64.msi"},
    {"git", "https://github.com/git-for-windows/git/releases/download/v2.44.0.windows.1/Git-2.44.0-64-bit.exe"},
    {"php", "https://windows.php.net/downloads/releases/php-8.3.4-nts-Win32-vs16-x64.zip"},
    {"go", "https://go.dev/dl/go1.22.1.windows-amd64.msi"},
    {"lua", "https://github.com/rjpcomputing/luaforwindows/releases/download/v5.1.5-52/LuaForWindows_v5.1.5-52.exe"},
    {"luajit", "https://github.com/openresty/luajit2/archive/refs/heads/v2.1-agentzh.zip"},
    {"cpp", "https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-msvcrt-rt_v11-rev1.7z"},
    {"c", "https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-msvcrt-rt_v11-rev1.7z"},
    {"cs", "https://download.visualstudio.microsoft.com/download/pr/8be4c95a-c603-4903-b09e-73c33215264b/81f8f742f1f31f9d6a36c4b9d033758f/dotnet-sdk-8.0.201-win-x64.exe"}
};

void play_loading_animation(bool* keepRunning, std::string action) {
    char chars[] = {'/', '-', '\\', '|'};
    int i = 0;
    while (*keepRunning) {
        std::cout << "\r[" << action << "] İşlem yapılıyor... " << chars[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        i++;
    }
    std::cout << "\r[" << action << "] TAMAMLANDI!          " << std::endl;
}

void show_help() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "      GITHOUSE CLI v8.1 - KÜTAHYA       " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "--force-install=\"URL\" : Manuel kurulum." << std::endl;
    std::cout << "--repair              : Githouse'u günceller." << std::endl;
    std::cout << "--version             : Sürüm bilgisi." << std::endl;
    std::cout << "\nPOPÜLER MARKET KOMUTLARI:" << std::endl;
    int count = 0;
    for (auto const& [name, url] : market) {
        std::cout << "--get-" << name << "\t";
        if (++count % 2 == 0) std::cout << std::endl;
    }
    std::cout << "\n========================================\n" << std::endl;
}

int run_process(std::string filePath, std::string mode) {
    std::string cmd;
    bool keepRunning = true;
    std::string actionName = "INSTALLING";

    // ZIP DOSYASI AYIKLAMA (LuaJIT ve PHP için)
    if (filePath.find(".zip") != std::string::npos || filePath.find(".7z") != std::string::npos) {
        actionName = "EXTRACTING";
        std::string dest = filePath.substr(0, filePath.find_last_of("\\")) + "\\GitHouse_Extracted";
        cmd = "powershell -Command \"Expand-Archive -Path '" + filePath + "' -DestinationPath '" + dest + "' -Force\"";
    } 
    // MSI KURULUM
    else if (filePath.find(".msi") != std::string::npos) {
        cmd = "msiexec /i \"" + filePath + "\" /passive /norestart";
    } 
    // EXE KURULUM
    else {
        cmd = "\"" + filePath + "\" /S /passive";
    }

    std::thread loader(play_loading_animation, &keepRunning, actionName);
    int result = system(cmd.c_str());
    keepRunning = false;
    loader.join();

    if (result == 0) {
        DeleteFileA(filePath.c_str());
        std::cout << "[CLEANUP] Geçici dosya imha edildi." << std::endl;
    }
    return result;
}

int download_and_execute(std::string url, std::string mode) {
    char* winDir = getenv("WINDIR");
    size_t lastSlash = url.find_last_of("/");
    std::string filename = url.substr(lastSlash + 1);

    // Dinamik GitHub linkleri için isim düzeltme
    if (filename.find("zip") != std::string::npos && filename.find(".zip") == std::string::npos) filename += ".zip";
    if (filename.find(".") == std::string::npos) filename += ".exe";

    std::string fullPath = std::string(winDir) + "\\" + filename;

    std::cout << "[GITHOUSE] İndiriliyor: " << filename << std::endl;
    
    // Güçlü indirme motoru (Invoke-WebRequest)
    std::string psCmd = "powershell -Command \"[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '" + url + "' -OutFile '" + fullPath + "'\"";
    
    bool keepRunning = true;
    std::thread loader(play_loading_animation, &keepRunning, "DOWNLOADING");
    int dlResult = system(psCmd.c_str());
    keepRunning = false;
    loader.join();

    if (dlResult == 0) return run_process(fullPath, mode);
    
    std::cout << "[!] İndirme hatası! Linki kontrol et." << std::endl;
    return -1;
}

int main(int argc, char* argv[]) {
    system("chcp 65001 > nul"); // Türkçe karakter desteği

    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string arg = argv[1];

    if (arg == "--version") {
        std::cout << "GitHouse Version: " << GITHOUSE_VERSION << " (Kütahya)" << std::endl;
    } 
    else if (arg == "--help") {
        show_help();
    }
    else if (arg == "--repair") {
        download_and_execute(REPO_EXE_URL, "repair");
    }
    else if (arg.find("--force-install=") == 0) {
        download_and_execute(arg.substr(16), "install");
    }
    else if (arg.find("--get-") == 0) {
        std::string target = arg.substr(6);
        if (market.count(target)) {
            download_and_execute(market[target], "install");
        } else {
            std::cout << "[!] Market'te '" << target << "' bulunamadı." << std::endl;
        }
    }
    else {
        std::cout << "[!] Bilinmeyen komut. Yardım: githouse --help" << std::endl;
    }

    return 0;
}