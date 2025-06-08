// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <chrono> // De su dung cac thao tac lien quan den thoi gian trong OTP
#include <random> // De sinh so ngau nhien
#include <sstream> // De thao tac voi chuoi du lieu
#include <iomanip> // De dinh dang dau ra (vi du: std::hex, std::setw, std::setfill)
#include <fstream> // De thao tac voi file


// Them cac include can thiet cho viec tao thu muc cu hon
#ifdef _WIN32 // Neu la he dieu hanh Windows
#include <direct.h> // Cho _mkdir
#else // Neu la he dieu hanh Unix/Linux/macOS
#include <sys/stat.h> // Cho mkdir
#include <errno.h> // Cho errno
#endif


namespace Utils {

    // Ham bam mat khau (gia dinh, khong an toan cho moi truong thuc te)
    // Trong thuc te, hay su dung thu vien nhu bcrypt hoac Argon2.
    std::string hashPassword(const std::string& password);

    // Ham xac thuc mat khau
    bool verifyPassword(const std::string& password, const std::string& hashedPassword);

    // Ham sinh ID duy nhat (su dung UUID gia dinh)
    std::string generateUniqueId();

    // Ham sinh mat khau ngau nhien
    std::string generateRandomPassword(int length = 10);

    // Ham kiem tra va tao thu muc neu no khong ton tai
    // (Da sua doi de khong dung <filesystem>)
    void createDirectoryIfNotExists(const std::string& path);

    // Ham doc toan bo noi dung tu mot file
    std::string readFileContent(const std::string& filename);

    // Ham ghi noi dung vao mot file (ghi de neu file ton tai)
    bool writeToFile(const std::string& filename, const std::string& content);

    // Ham them noi dung vao cuoi mot file
    bool appendToFile(const std::string& filename, const std::string& content);

    // Ham doc tat ca cac dong tu mot file
    std::vector<std::string> readAllLines(const std::string& filename);

    // Ham tach chuoi theo delimiter
    std::vector<std::string> splitString(const std::string& s, char delimiter);

    // Ham cat bo khoang trang o dau va cuoi chuoi
    std::string trimString(const std::string& s);

    // Ham chuyen doi thoi gian sang chuoi
    std::string timeToString(time_t time);

    // Ham chuyen doi chuoi sang thoi gian
    time_t stringToTime(const std::string& timeStr);
}

#endif // UTILS_H

