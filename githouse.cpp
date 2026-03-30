#include <iostream>
#include <filesystem>

// Modern C++17 Dosya Sistemi
namespace fs = std::filesystem;

void prepare_bin() {
    if (!fs::exists("bin")) {
        fs::create_directory("bin");
    }
}