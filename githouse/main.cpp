#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <chrono>

// Sürüm Bilgisi
const std::string GITHOUSE_VERSION = "7.0.2-Stable";

// Animasyon Fonksiyonu
void play_loading_animation(bool* keepRunning, std::string action) {
    char chars[] = {'/', '-', '\\', '|'};
    int i = 0;
    while (*keepRunning) {
        std::cout << "\r[" << action << "] Islem yapiliyor... " << chars[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        i++;
    }
    std::cout << "\r[" << action << "] TAMAMLANDI!          " << std::endl;
}

// Yardım Menüsü
void show_help() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "        GITHOUSE CLI - YARDIM           " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "--force-install=\"URL\" : Programi indirir ve kurar." << std::endl;
    std::cout << "--repair=\"URL\"        : Mevcut programi onarir." << std::endl;
    std::cout << "--remove=\"URL/AD\"     : Programi sistemden kaldirir." << std::endl;
    std::cout << "--version             : Mevcut surumu gosterir." << std::endl;
    std::cout << "--help                : Bu menuyu acar." << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Kurulum ve Onarım Motoru
int run_process(std::string filePath, std::string mode) {
    std::string cmd;
    bool isMsi = (filePath.find(".msi") != std::string::npos);
    std::string actionName = (mode == "install") ? "INSTALLING" : (mode == "repair" ? "REPAIRING" : "REMOVING");

    if (isMsi) {
        if (mode == "install") cmd = "msiexec /i \"" + filePath + "\" /passive /norestart";
        else if (mode == "repair") cmd = "msiexec /f p \"" + filePath + "\" /passive";
        else if (mode == "remove") cmd = "msiexec /x \"" + filePath + "\" /passive";
    } else {
        // EXE için standart sessiz parametreler
        if (mode == "remove") cmd = "\"" + filePath + "\" /uninstall /S";
        else cmd = "\"" + filePath + "\" /S";
    }

    bool keepRunning = true;
    std::thread loader(play_loading_animation, &keepRunning, actionName);
    
    int result = system(cmd.c_str());
    
    keepRunning = false;
    loader.join();

    // Temizlik: Kurulum bittiyse dosyayı sil (Remove işleminde dosya zaten sistemdedir, silme)
    if (result == 0 && mode != "remove") {
        DeleteFileA(filePath.c_str());
        std::cout << "[CLEANUP] Gecici dosyalar imha edildi." << std::endl;
    }

    return result;
}

// İndirme Motoru (BITS)
int download_and_execute(std::string url, std::string mode) {
    char* winDir = getenv("WINDIR");
    size_t lastSlash = url.find_last_of("/");
    std::string filename = url.substr(lastSlash + 1);
    std::string fullPath = std::string(winDir) + "\\" + filename;

    std::cout << "[GITHOUSE] Hedef: " << filename << " (" << mode << ")" << std::endl;

    std::string psCmd = "powershell -Command \"Start-BitsTransfer -Source '" + url + "' -Destination '" + fullPath + "'\"";
    int dlResult = system(psCmd.c_str());

    if (dlResult == 0) return run_process(fullPath, mode);
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string arg = argv[1];

    if (arg == "--version") {
        std::cout << "GitHouse Version: " << GITHOUSE_VERSION << " (Kütahya/Turkey)" << std::endl;
    } 
    else if (arg == "--help") {
        show_help();
    }
    else if (arg.find("--force-install=") == 0) {
        std::string url = arg.substr(16);
        download_and_execute(url, "install");
    }
    else if (arg.find("--repair=") == 0) {
        std::string url = arg.substr(9);
        download_and_execute(url, "repair");
    }
    else if (arg.find("--remove=") == 0) {
        // Remove için eğer URL verilirse önce indirip sonra /x yapar
        std::string url = arg.substr(9);
        download_and_execute(url, "remove");
    }
    else {
        std::cout << "[!] Bilinmeyen komut. Yardim icin: githouse --help" << std::endl;
    }

    return 0;
}