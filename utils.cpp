// utils.cpp
#include "utils.h"
#include <iostream>
#include <functional> // De su dung std::hash
#include <algorithm> // De su dung std::remove_if
#include <cctype> // De su dung std::isspace
#include <string.h> // De su dung strerror
#include <cstdio>   // De su dung sscanf

// Da them cac include can thiet trong utils.h
#ifdef _WIN32
#include <io.h> // Can thiet cho _stat tren mot so he thong Windows/MinGW
#endif

// Trien khai cac ham tien ich

namespace Utils {

    // Ham bam mat khau (gia dinh)
    // Day la mot ham bam don gian, KHONG AN TOAN cho moi truong thuc te.
    // Trong ung dung thuc te, hay su dung cac thu vien bam mat khau chuyen dung nhu bcrypt hoac Argon2.
    std::string hashPassword(const std::string& password) {
        std::hash<std::string> hasher;
        size_t hashed_val = hasher(password);
        return std::to_string(hashed_val);
    }

    // Ham xac thuc mat khau
    bool verifyPassword(const std::string& password, const std::string& hashedPassword) {
        return hashPassword(password) == hashedPassword;
    }

    // Ham sinh ID duy nhat (UUID gia dinh)
    std::string generateUniqueId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> distrib(0, 15);
        static const char* hex_chars = "0123456789abcdef";

        std::stringstream ss;
        for (int i = 0; i < 32; ++i) {
            ss << hex_chars[distrib(gen)];
            if (i == 7 || i == 11 || i == 15 || i == 19) {
                ss << "-";
            }
        }
        return ss.str();
    }

    // Ham sinh mat khau ngau nhien
    std::string generateRandomPassword(int length) {
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
        std::string password = "";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, charset.length() - 1);

        for (int i = 0; i < length; ++i) {
            password += charset[distribution(generator)];
        }
        return password;
    }

    // Ham kiem tra va tao thu muc neu no khong ton tai
    // (Da sua doi de khong dung <filesystem>, chi tao mot cap)
    // (Da sua doi de khong dung _stat/_stat64, chi dung _mkdir/mkdir va kiem tra errno)
    void createDirectoryIfNotExists(const std::string& path) {
        // Thu muc se duoc tao neu no khong ton tai.
        // Neu no da ton tai, _mkdir/mkdir se tra ve loi va errno se la EEXIST.
        // Chung ta chi quan tam den cac loi khac ngoai EEXIST.
#ifdef _WIN32
        if (_mkdir(path.c_str()) != 0) { // Tao thu muc tren Windows
            if (errno != EEXIST) { // Neu loi khong phai do thu muc da ton tai
                std::cerr << "Loi khi tao thu muc " << path << ": " << strerror(errno) << std::endl;
            }
        } else { // Tao thu muc thanh cong
            std::cout << "Da tao thu muc: " << path << std::endl;
        }
#else // Unix/Linux/macOS
        if (mkdir(path.c_str(), 0777) != 0) { // Tao thu muc tren Unix-like
            if (errno != EEXIST) { // Neu loi khong phai do thu muc da ton tai
                std::cerr << "Loi khi tao thu muc " << path << ": " << strerror(errno) << std::endl;
            }
        } else { // Tao thu muc thanh cong
            std::cout << "Da tao thu muc: " << path << std::endl;
        }
#endif
    }

    // Ham doc toan bo noi dung tu mot file
    std::string readFileContent(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Ham ghi noi dung vao mot file (ghi de)
    bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Loi: Khong the mo file " << filename << " de ghi." << std::endl;
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    // Ham them noi dung vao cuoi mot file
    bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app); // Mo o che do append
        if (!file.is_open()) {
            std::cerr << "Loi: Khong the mo file " << filename << " de them." << std::endl;
            return false;
        }
        file << content << std::endl; // Them noi dung va xuong dong
        file.close();
        return true;
    }

    // Ham doc tat ca cac dong tu mot file
    std::vector<std::string> readAllLines(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<std::string> lines;
        if (!file.is_open()) {
            return lines;
        }
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        return lines;
    }

    // Ham tach chuoi theo delimiter
    std::vector<std::string> splitString(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Ham cat bo khoang trang o dau va cuoi chuoi
    std::string trimString(const std::string& s) {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){return std::isspace(c);});
        auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){return std::isspace(c);}).base();
        return (wsback <= wsfront ? "" : std::string(wsfront, wsback));
    }

    // Ham chuyen doi thoi gian sang chuoi
    std::string timeToString(time_t time) {
        char buffer[80];
        struct tm* tm_info = localtime(&time);
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(buffer);
    }

    // Ham chuyen doi chuoi sang thoi gian (Da sua doi de khong dung std::get_time)
    time_t stringToTime(const std::string& timeStr) {
        struct tm tm_info = {};
        int year, month, day, hour, minute, second;

        // Su dung sscanf de phan tich chuoi
        if (sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d",
                   &year, &month, &day, &hour, &minute, &second) == 6) {
            tm_info.tm_year = year - 1900; // Nam tinh tu 1900
            tm_info.tm_mon = month - 1;    // Thang tinh tu 0 (0-11)
            tm_info.tm_mday = day;
            tm_info.tm_hour = hour;
            tm_info.tm_min = minute;
            tm_info.tm_sec = second;
            tm_info.tm_isdst = -1; // De mktime tu dong xac dinh DST

            return mktime(&tm_info);
        } else {
            return 0; // Tra ve 0 neu that bai
        }
    }
}

