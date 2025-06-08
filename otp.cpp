// otp.cpp
#include "otp.h"
#include <iostream>
#include <random>
#include <string>
#include <chrono>

// Ham ho tro sinh ma OTP ngau nhien
std::string OTPManager::generateRandomOTP(int length) {
    const std::string digits = "0123456789";
    std::string otp = "";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, digits.length() - 1);

    for (int i = 0; i < length; ++i) {
        otp += digits[distribution(generator)];
    }
    return otp;
}

// Ham tao key cho map activeOTPs
std::string OTPManager::generateKey(const std::string& userId, const std::string& action) const {
    return userId + "_" + action;
}

// Sinh OTP moi cho mot hanh dong/nguoi dung cu the
std::string OTPManager::generateOTP(const std::string& userId, const std::string& action) {
    std::string key = generateKey(userId, action);
    std::string otpCode = generateRandomOTP();
    activeOTPs[key] = {otpCode, std::chrono::system_clock::now()};
    std::cout << "OTP da duoc tao cho " << userId << " (" << action << "): " << otpCode << std::endl; // De muc dich kiem tra
    return otpCode;
}

// Xac thuc OTP
bool OTPManager::verifyOTP(const std::string& userId, const std::string& action, const std::string& otp) {
    std::string key = generateKey(userId, action);
    auto it = activeOTPs.find(key);

    if (it == activeOTPs.end()) {
        std::cout << "Loi: Khong tim thay OTP cho hanh dong nay hoac da het han." << std::endl;
        return false; // Khong tim thay OTP hoac da bi huy
    }

    // Kiem tra thoi gian het han
    auto now = std::chrono::system_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.creationTime).count();

    if (elapsed_seconds > OTP_EXPIRATION_SECONDS) {
        std::cout << "Loi: OTP da het han." << std::endl;
        activeOTPs.erase(it); // Xoa OTP da het han
        return false;
    }

    // Kiem tra ma OTP
    if (it->second.otpCode == otp) {
        std::cout << "Xac thuc OTP thanh cong." << std::endl;
        // activeOTPs.erase(it); // Xoa OTP sau khi su dung thanh cong (co the xoa o ham invalidateOTP)
        return true;
    } else {
        std::cout << "Loi: Ma OTP khong chinh xac." << std::endl;
        return false;
    }
}

// Xoa OTP sau khi su dung hoac het han
void OTPManager::invalidateOTP(const std::string& userId, const std::string& action) {
    std::string key = generateKey(userId, action);
    activeOTPs.erase(key);
    std::cout << "OTP cho " << userId << " (" << action << ") da bi huy." << std::endl;
}

