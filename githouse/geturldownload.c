#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "urlmon.lib")

int download_file(const char* url, const char* destination) {
    printf("[Githouse] Baglanti kuruluyor: %s\n", url);
    // URLDownloadToFileA Windows'un yerleşik indirme motorudur
    HRESULT hr = URLDownloadToFileA(NULL, url, destination, 0, NULL);
    if (hr == S_OK) {
        return 0;
    }
    return -1;
}