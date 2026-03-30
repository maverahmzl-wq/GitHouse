#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include <thread>
#include <chrono>

// Havali dönen imlec fonksiyonu
void play_loading_animation(bool* keepRunning, std::string fileName) {
    char chars[] = {'/', '-', '\\', '|'};
    int i = 0;
    while (*keepRunning) {
        std::cout << "\r[SETUPPING] " << fileName << " kuruluyor... " << chars[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        i++;
    }
    std::cout << "\r[SETUPPING] " << fileName << " kuruluyor... TAMAM!   " << std::endl;
}

int execute_setup(std::string filePath) {
    std::string cmd;
    bool isMsi = (filePath.find(".msi") != std::string::npos);

    if (isMsi) {
        // /i: Kur, /passive: Sadece ilerleme çubuğu göster (kullanıcı onayı gerektirmez), /norestart: Yeniden başlatma
        cmd = "msiexec /i \"" + filePath + "\" /passive /norestart";
    } else {
        // Normal EXE ise doğrudan çalıştır
        cmd = "\"" + filePath + "\" /S"; // /S genellikle sessiz kurulum parametresidir
    }

    bool keepRunning = true;
    size_t lastSlash = filePath.find_last_of("\\");
    std::string fileName = filePath.substr(lastSlash + 1);

    // Animasyonu yan bir kanalda (thread) başlat
    std::thread loader(play_loading_animation, &keepRunning, fileName);

    int result = system(cmd.c_str());
    
    keepRunning = false; // Animasyonu durdur
    loader.join();

    return result;
}

int download_to_windows(std::string url, std::string filename) {
    char* winDir = getenv("WINDIR");
    if (!winDir) return -2;

    std::string fullPath = std::string(winDir) + "\\" + filename;
    std::cout << "[Githouse] Hedef: " << fullPath << std::endl;

    std::string psCmd = "powershell -Command \"Start-BitsTransfer -Source '" + url + "' -Destination '" + fullPath + "'\"";
    
    int result = system(psCmd.c_str());
    if (result == 0) {
        std::cout << "[SUCCESSFULLY] INSTALLED \"" << filename << "\"" << std::endl;
        return execute_setup(fullPath);
    }
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 0;
    std::string arg = argv[1];

    if (arg.find("--force-install") != std::string::npos) {
        size_t pos = arg.find("=");
        if (pos == std::string::npos) return 1;
        std::string url = arg.substr(pos + 1);
        
        if (!url.empty() && (url.front() == '\"' || url.front() == '\'')) url.erase(0, 1);
        if (!url.empty() && (url.back() == '\"' || url.back() == '\'')) url.pop_back();

        size_t lastSlash = url.find_last_of("/");
        std::string filename = url.substr(lastSlash + 1);

        if (download_to_windows(url, filename) == 0) {
            std::cout << "\n[DONE] Sisteme basariyla entegre edildi." << std::endl;
        } else {
            std::cout << "\n[FAIL] Bir seyler ters gitti." << std::endl;
        }
    }
    return 0;
}