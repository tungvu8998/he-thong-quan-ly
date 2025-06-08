// otp.h
#ifndef OTP_H
#define OTP_H

#include <string>
#include <map>
#include <chrono> // De su dung std::chrono::time_point
#include <random> // De sinh so ngau nhien
#include <ctime> // De su dung time_t

class OTPManager {
public:
    // Thoi gian hieu luc cua OTP (vi du: 5 phut = 300 giay)
    static const int OTP_EXPIRATION_SECONDS = 300;

    // Sinh OTP moi cho mot hanh dong/nguoi dung cu the
    // userId: ID cua nguoi dung
    // action: Loai hanh dong can xac thuc (vi du: "update_profile", "transfer_points")
    std::string generateOTP(const std::string& userId, const std::string& action);

    // Xac thuc OTP
    // userId: ID cua nguoi dung
    // action: Loai hanh dong can xac thuc
    // otp: Ma OTP ma nguoi dung nhap
    // Tra ve true neu OTP hop le va con han, false neu khong
    bool verifyOTP(const std::string& userId, const std::string& action, const std::string& otp);

    // Xoa OTP sau khi su dung hoac het han
    // userId: ID cua nguoi dung
    // action: Loai hanh dong da duoc xac thuc
    void invalidateOTP(const std::string& userId, const std::string& action);

private:
    // Cau truc de luu du lieu OTP
    struct OTPData {
        std::string otpCode;
        std::chrono::time_point<std::chrono::system_clock> creationTime; // Thoi gian tao OTP
    };

    // Luu tru OTP duoi dang: map<key (userId_action), OTPData>
    // Key duoc tao bang cach noi userId va action de dam bao OTP duy nhat cho moi hanh dong cua moi nguoi dung.
    std::map<std::string, OTPData> activeOTPs;

    // Ham ho tro sinh ma OTP ngau nhien
    std::string generateRandomOTP(int length = 6);

    // Ham tao key cho map activeOTPs
    std::string generateKey(const std::string& userId, const std::string& action) const;
};

#endif // OTP_H

