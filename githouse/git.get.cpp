#include <string>

std::string get_raw_url(std::string url) {
    size_t pos = url.find("github.com");
    if (pos != std::string::npos) {
        // GitHub linkini raw formatına çevirir
        url.replace(pos, 10, "raw.githubusercontent.com");
        size_t blob_pos = url.find("/blob/");
        if (blob_pos != std::string::npos) {
            url.erase(blob_pos, 5);
        }
    }
    return url;
}