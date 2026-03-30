#include <iostream>
#include <fstream>
#include <string>

void log_install(std::string name, std::string url) {
    std::ofstream file("save.json", std::ios::app);
    if (file.is_open()) {
        file << "{\"name\":\"" << name << "\", \"url\":\"" << url << "\"}" << std::endl;
        file.close();
    }
}

bool check_where(std::string name) {
    std::ifstream file("save.json");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(name) != std::string::npos) return true;
    }
    return false;
}