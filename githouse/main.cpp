#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

// Sürüm ve Repo Bilgisi
const std::string GITHOUSE_VERSION = "7.5.0-SelfRepair";
const std::string REPO_EXE_URL = "https://github.com/maverahmzl-wq/GitHouse/raw/main/githouse/githouse.exe";

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
    std::cout << "        GITHOUSE CLI v7.5 - YARDIM      " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "--force-install=\"URL\" : Programi indirir ve kurar." << std::endl;
    std::cout << "--repair              : Githouse'u internetten onarir/gunceller." << std::endl;
    std::cout << "--remove=\"URL\"        : .msi paketiyle programi kaldirir." << std::endl;
    std::cout << "--version             : Mevcut surumu gosterir." << std::endl;
    std::cout << "--help                : Bu menuyu acar." << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Kendi Kendini Onarma Fonksiyonu (Self-Repair)
int self_repair() {
    char* winDir = getenv("WINDIR");
    if (!winDir) return -1;

    std::string targetPath = std::string(winDir) + "\\githouse.exe";
    std::string tempPath = std::string(winDir) + "\\githouse_update.exe";

    std::cout << "[SELF-REPAIR] Ana cekirdek GitHub'dan cekiliyor..." << std::endl;

    // 1. Yeni sürümü geçici isimle indir
    std::string psDownload = "powershell -Command \"Start-BitsTransfer -Source '" + REPO_EXE_URL + "' -Destination '" + tempPath + "'\"";
    
    bool keepRunning = true;
    std::thread loader(play_loading_animation, &keepRunning, "REPAIRING");
    int dlResult = system(psDownload.c_str());
    keepRunning = false;
    loader.join();

    if (dlResult == 0) {
        // 2. Çalışan dosyayı değiştirmek için Batch script numarası kullanıyoruz
        // Bu komut mevcut exe kapandığı an yenisini üzerine yazar.
        std::string moveCmd = "cmd /c move /y \"" + tempPath + "\" \"" + targetPath + "\"";
        std::cout << "[SUCCESS] Onarim dosyasi hazirlandi. Bir sonraki calistirmada aktif olacak." << std::endl;
        system(moveCmd.c_str());
        return 0;
    } else {
        std::cout << "[FAIL] GitHub baglantisi kurulamadi!" << std::endl;
        return -1;
    }
}

// Genel Kurulum/Kaldırma Motoru
int run_process(std::string filePath, std::string mode) {
    std::string cmd;
    bool isMsi = (filePath.find(".msi") != std::string::npos);
    std::string actionName = (mode == "install") ? "INSTALLING" : "REMOVING";

    if (isMsi) {
        if (mode == "install") cmd = "msiexec /i \"" + filePath + "\" /passive /norestart";
        else cmd = "msiexec /x \"" + filePath + "\" /passive";
    } else {
        cmd = "\"" + filePath + "\" /S";
    }

    bool keepRunning = true;
    std::thread loader(play_loading_animation, &keepRunning, actionName);
    int result = system(cmd.c_str());
    keepRunning = false;
    loader.join();

    if (result == 0) {
        DeleteFileA(filePath.c_str());
        std::cout << "[CLEANUP] Kurulum dosyasi temizlendi." << std::endl;
    }
    return result;
}

// İndirme ve Yürütme
int download_and_execute(std::string url, std::string mode) {
    char* winDir = getenv("WINDIR");
    size_t lastSlash = url.find_last_of("/");
    std::string filename = url.substr(lastSlash + 1);
    std::string fullPath = std::string(winDir) + "\\" + filename;

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
        std::cout << "GitHouse Version: " << GITHOUSE_VERSION << " (Kütahya)" << std::endl;
    } 
    else if (arg == "--help") {
        show_help();
    }
    else if (arg == "--repair") {
        self_repair();
    }
    else if (arg.find("--force-install=") == 0) {
        std::string url = arg.substr(16);
        download_and_execute(url, "install");
    }
    else if (arg.find("--remove=") == 0) {
        std::string url = arg.substr(9);
        download_and_execute(url, "remove");
    }
    else {
        std::cout << "[!] Bilinmeyen komut. Yardim: githouse --help" << std::endl;
    }

    return 0;
}