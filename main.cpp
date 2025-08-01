// main.cpp
#include <iostream>
#include <string>
#include <limits> // De su dung numeric_limits
#include <vector>
#include <algorithm> // De su dung std::remove_if
#include <memory> // Them dong nay de su dung std::unique_ptr

#include "user.h"
#include "wallet.h"
#include "otp.h"
#include "utils.h"

// Bien toan cuc de quan ly OTP (co the truyen qua ham neu muon)
OTPManager otpManager;

// Bien toan cuc cho nguoi dung hien tai dang dang nhap
// Su dung unique_ptr de tu dong quan ly bo nho
std::unique_ptr<User> currentUser = nullptr;

// Ham khoi tao he thong (tao thu muc, vi tong)
void initializeSystem() {
    // Chu y: createDirectoryIfNotExists chi tao duoc mot cap thu muc.
    // Dam bao "data" duoc tao truoc khi tao "data/users" va "data/wallets"
    Utils::createDirectoryIfNotExists("data");
    Utils::createDirectoryIfNotExists("data/users");
    Utils::createDirectoryIfNotExists("data/wallets");

    // Kiem tra va tao vi tong neu chua co
    // Su dung unique_ptr de tu dong quan ly bo nho
    std::unique_ptr<Wallet> masterWallet = Wallet::loadFromFile("MASTER_WALLET");
    if (!masterWallet) {
        // Neu chua co, tao moi va cap phat tren heap
        masterWallet = std::make_unique<Wallet>("SYSTEM"); // Owner ID la "SYSTEM"
        masterWallet->walletId = "MASTER_WALLET"; // Dat ID co dinh
        masterWallet->balance = 1000000.0; // Khoi tao so diem lon cho vi tong
        masterWallet->saveToFile();
        std::cout << "Da khoi tao vi tong voi ID: MASTER_WALLET va so du " << masterWallet->balance << " diem." << std::endl;
    } else {
        std::cout << "Vi tong da ton tai voi ID: MASTER_WALLET va so du " << masterWallet->balance << " diem." << std::endl;
    }
    // unique_ptr masterWallet se tu dong giai phong bo nho khi ra khoi scope
}

// Ham xoa bo dem ban phim
void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Ham dang ky tai khoan
void registerAccount() {
    std::string username, password, confirmPassword, fullName, email, phoneNumber;
    std::cout << "\n--- Dang ky tai khoan ---" << std::endl;
    std::cout << "Nhap ten dang nhap: ";
    std::cin >> username;
    clearInputBuffer();

    // Kiem tra trung ten dang nhap
    // Su dung unique_ptr de tu dong quan ly bo nho cho doi tuong user doc tu file
    std::unique_ptr<User> existingUser = std::unique_ptr<User>(User::loadFromFile(username));
    if (existingUser != nullptr) {
        std::cout << "Ten dang nhap da ton tai. Vui long chon ten khac." << std::endl;
        return;
    }

    std::cout << "Nhap mat khau: ";
    std::cin >> password;
    clearInputBuffer();
    std::cout << "Xac nhan mat khau: ";
    std::cin >> confirmPassword;
    clearInputBuffer();

    if (password != confirmPassword) {
        std::cout << "Mat khau xac nhan khong khop. Dang ky that bai." << std::endl;
        return;
    }

    std::cout << "Nhap ho va ten: ";
    std::getline(std::cin, fullName);

    std::cout << "Nhap email: ";
    std::cin >> email;
    clearInputBuffer();

    std::cout << "Nhap so dien thoai: ";
    std::cin >> phoneNumber;
    clearInputBuffer();

    // Mac dinh la nguoi dung thong thuong khi tu dang ky
    User newUser(username, Utils::hashPassword(password), fullName, email, phoneNumber, "normal");
    if (newUser.saveToFile()) { // Ham saveToFile da tu dong cap nhat file index
        std::cout << "Dang ky tai khoan thanh cong!" << std::endl;
        // Tao vi cho nguoi dung moi
        Wallet newWallet(newUser.getUserId());
        if (newWallet.saveToFile()) { // Ham saveToFile da tu dong cap nhat file index
            std::cout << "Da tao vi diem thuong cho tai khoan moi voi ID: " << newWallet.walletId << std::endl;
        } else {
            std::cerr << "Loi: Khong the tao vi diem thuong cho tai khoan moi." << std::endl;
        }
    } else {
        std::cout << "Dang ky tai khoan that bai. Vui long thu lai." << std::endl;
    }
}

// Ham dang nhap
bool login() {
    std::string username, password;
    std::cout << "\n--- Dang nhap ---" << std::endl;
    std::cout << "Ten dang nhap: ";
    std::cin >> username;
    clearInputBuffer();
    std::cout << "Mat khau: ";
    std::cin >> password;
    clearInputBuffer();

    // Su dung unique_ptr de tu dong quan ly bo nho cho doi tuong user doc tu file
    std::unique_ptr<User> user = std::unique_ptr<User>(User::loadFromFile(username));
    if (!user) {
        std::cout << "Ten dang nhap khong ton tai." << std::endl;
        return false;
    }

    if (Utils::verifyPassword(password, user->getHashedPassword())) {
        std::cout << "Dang nhap thanh cong!" << std::endl;
        // Chuyen quyen so huu unique_ptr cho currentUser
        currentUser = std::move(user); 

        if (currentUser->getIsAutoGeneratedPassword()) {
            std::cout << "LUU Y: Mat khau cua ban la mat khau tu dong sinh. Vui long doi mat khau ngay!" << std::endl;
            // Yeu cau doi mat khau ngay lap tuc
            std::string newPassword, confirmNewPassword;
            do {
                std::cout << "Nhap mat khau moi: ";
                std::cin >> newPassword;
                clearInputBuffer();
                std::cout << "Xac nhan mat khau moi: ";
                std::cin >> confirmNewPassword;
                clearInputBuffer();

                if (newPassword != confirmNewPassword) {
                    std::cout << "Mat khau xac nhan khong khop. Vui long nhap lai." << std::endl;
                } else if (newPassword.length() < 6) { // Kiem tra do dai mat khau toi thieu
                    std::cout << "Mat khau moi phai co it nhat 6 ky tu. Vui long nhap lai." << std::endl;
                }
            } while (newPassword != confirmNewPassword || newPassword.length() < 6);

            currentUser->changePassword(Utils::hashPassword(newPassword));
            if (currentUser->saveToFile()) {
                std::cout << "Mat khau da duoc cap nhat thanh cong." << std::endl;
            } else {
                std::cerr << "Loi: Khong the luu mat khau moi." << std::endl;
            }
        }
        return true;
    } else {
        std::cout << "Mat khau khong chinh xac." << std::endl;
        // unique_ptr user se tu dong giai phong bo nho khi ra khoi scope
        return false;
    }
}

// Ham thay doi thong tin ca nhan (dung cho ca user va admin tu sua)
// Nhan vao User* vi currentUser.get() tra ve raw pointer
void updateProfile(User* userToUpdate, bool isAdminUpdating = false) {
    if (!userToUpdate) {
        std::cout << "Loi: Khong co thong tin nguoi dung de cap nhat." << std::endl;
        return;
    }

    std::string newFullName, newEmail, newPhoneNumber;
    std::cout << "\n--- Cap nhat thong tin ca nhan ---" << std::endl;
    std::cout << "Ho va ten hien tai: " << userToUpdate->getFullName() << std::endl;
    std::cout << "Nhap ho va ten moi (de trong de giu nguyen): ";
    std::getline(std::cin, newFullName);

    std::cout << "Email hien tai: " << userToUpdate->getEmail() << std::endl;
    std::cout << "Nhap email moi (de trong de giu nguyen): ";
    std::cin >> newEmail;
    clearInputBuffer();

    std::cout << "So dien thoai hien tai: " << userToUpdate->getPhoneNumber() << std::endl;
    std::cout << "Nhap so dien thoai moi (de trong de giu nguyen): ";
    std::cin >> newPhoneNumber;
    clearInputBuffer();

    // Tao thong bao ve cac thay doi du kien
    std::string changesSummary = "Ban dang yeu cau thay doi thong tin:\n";
    bool hasChanges = false;
    if (!newFullName.empty()) {
        changesSummary += "  - Ho va ten: " + userToUpdate->getFullName() + " -> " + newFullName + "\n";
        hasChanges = true;
    }
    if (!newEmail.empty()) {
        changesSummary += "  - Email: " + userToUpdate->getEmail() + " -> " + newEmail + "\n";
        hasChanges = true;
    }
    if (!newPhoneNumber.empty()) {
        changesSummary += "  - So dien thoai: " + userToUpdate->getPhoneNumber() + " -> " + newPhoneNumber + "\n";
        hasChanges = true;
    }

    if (!hasChanges) {
        std::cout << "Khong co thay doi nao duoc nhap. Huy bo cap nhat." << std::endl;
        return;
    }

    std::cout << changesSummary << std::endl;

    // Sinh va xac thuc OTP
    std::string otpCode = otpManager.generateOTP(userToUpdate->getUserId(), "update_profile");
    std::string enteredOTP;
    std::cout << "Ma OTP da duoc gui den ban (hoac nguoi quan ly). Vui long nhap OTP de xac nhan: ";
    std::cin >> enteredOTP;
    clearInputBuffer();

    if (otpManager.verifyOTP(userToUpdate->getUserId(), "update_profile", enteredOTP)) {
        if (!newFullName.empty()) userToUpdate->updateFullName(newFullName);
        if (!newEmail.empty()) userToUpdate->updateEmail(newEmail);
        if (!newPhoneNumber.empty()) userToUpdate->updatePhoneNumber(newPhoneNumber);

        if (userToUpdate->saveToFile()) {
            std::cout << "Cap nhat thong tin thanh cong!" << std::endl;
        } else {
            std::cerr << "Loi: Khong the luu thong tin cap nhat." << std::endl;
        }
    } else {
        std::cout << "Xac thuc OTP that bai. Cap nhat thong tin bi huy bo." << std::endl;
    }
    otpManager.invalidateOTP(userToUpdate->getUserId(), "update_profile"); // Huy OTP sau khi su dung
}

// Ham thay doi mat khau (dung cho user tu sua)
void changePassword() {
    if (!currentUser) {
        std::cout << "Ban chua dang nhap." << std::endl;
        return;
    }

    std::string oldPassword, newPassword, confirmNewPassword;
    std::cout << "\n--- Thay doi mat khau ---" << std::endl;
    std::cout << "Nhap mat khau cu: ";
    std::cin >> oldPassword;
    clearInputBuffer();

    if (!Utils::verifyPassword(oldPassword, currentUser->getHashedPassword())) {
        std::cout << "Mat khau cu khong chinh xac." << std::endl;
        return;
    }

    do {
        std::cout << "Nhap mat khau moi: ";
        std::cin >> newPassword;
        clearInputBuffer();
        std::cout << "Xac nhan mat khau moi: ";
        std::cin >> confirmNewPassword;
        clearInputBuffer();

        if (newPassword != confirmNewPassword) {
            std::cout << "Mat khau xac nhan khong khop. Vui long nhap lai." << std::endl;
        } else if (newPassword.length() < 6) { // Kiem tra do dai mat khau toi thieu
            std::cout << "Mat khau moi phai co it nhat 6 ky tu. Vui long nhap lai." << std::endl;
        }
    } while (newPassword != confirmNewPassword || newPassword.length() < 6);

    currentUser->changePassword(Utils::hashPassword(newPassword));
    if (currentUser->saveToFile()) {
        std::cout << "Mat khau da duoc cap nhat thanh cong." << std::endl;
    } else {
        std::cerr << "Loi: Khong the luu mat khau moi." << std::endl;
    }
}

// Ham xem thong tin vi va so du
void viewWalletInfo() {
    if (!currentUser) {
        std::cout << "Ban chua dang nhap." << std::endl;
        return;
    }
    // Su dung unique_ptr de tu dong quan ly bo nho
    std::unique_ptr<Wallet> userWallet = Wallet::loadWalletByUserId(currentUser->getUserId());
    if (userWallet) {
        userWallet->displayWalletInfo();
    } else {
        std::cout << "Loi: Khong tim thay vi cua ban." << std::endl;
    }
}

// Ham chuyen diem
void transferPoints() {
    if (!currentUser) {
        std::cout << "Ban chua dang nhap." << std::endl;
        return;
    }

    std::string receiverWalletId;
    double amount;

    std::cout << "\n--- Chuyen diem ---" << std::endl;
    std::cout << "Nhap ID vi nguoi nhan: ";
    std::cin >> receiverWalletId;
    clearInputBuffer();

    std::cout << "Nhap so diem muon chuyen: ";
    std::cin >> amount;
    clearInputBuffer();

    if (std::cin.fail() || amount <= 0) {
        std::cout << "So diem khong hop le. Vui long nhap so duong." << std::endl;
        std::cin.clear();
        clearInputBuffer();
        return;
    }

    // Sinh va xac thuc OTP truoc khi chuyen diem
    std::string otpCode = otpManager.generateOTP(currentUser->getUserId(), "transfer_points");
    std::string enteredOTP;
    std::cout << "Ma OTP da duoc gui den ban. Vui long nhap OTP de xac nhan giao dich: ";
    std::cin >> enteredOTP;
    clearInputBuffer();

    if (otpManager.verifyOTP(currentUser->getUserId(), "transfer_points", enteredOTP)) {
        if (Wallet::transferPoints(currentUser->getUserId(), receiverWalletId, amount)) {
            std::cout << "Giao dich chuyen diem da hoan tat." << std::endl;
        } else {
            std::cout << "Giao dich chuyen diem that bai." << std::endl;
        }
    } else {
        std::cout << "Xac thuc OTP that bai. Giao dich chuyen diem bi huy bo." << std::endl;
    }
    otpManager.invalidateOTP(currentUser->getUserId(), "transfer_points"); // Huy OTP
}

// Ham xem lich su giao dich cua nguoi dung hien tai
void viewTransactionHistory() {
    if (!currentUser) {
        std::cout << "Ban chua dang nhap." << std::endl;
        return;
    }

    // Su dung unique_ptr de tu dong quan ly bo nho
    std::unique_ptr<Wallet> userWallet = Wallet::loadWalletByUserId(currentUser->getUserId());
    if (!userWallet) {
        std::cout << "Loi: Khong tim thay vi cua ban de xem lich su giao dich." << std::endl;
        return;
    }

    std::cout << "\n--- Lich su giao dich cua ban (" << userWallet->walletId << ") ---" << std::endl;
    std::vector<std::string> lines = Utils::readAllLines("data/transactions.log");
    bool foundTransactions = false;

    for (const std::string& line : lines) {
        Transaction* transaction = Transaction::fromString(line);
        if (transaction) {
            if (transaction->senderWalletId == userWallet->walletId || transaction->receiverWalletId == userWallet->walletId) {
                std::cout << "------------------------------------" << std::endl;
                std::cout << "ID Giao dich: " << transaction->transactionId << std::endl;
                std::cout << "Tu vi: " << transaction->senderWalletId << std::endl;
                std::cout << "Den vi: " << transaction->receiverWalletId << std::endl;
                std::cout << "So diem: " << std::fixed << std::setprecision(2) << transaction->amount << std::endl;
                std::cout << "Thoi gian: " << Utils::timeToString(transaction->timestamp) << std::endl;
                std::cout << "Trang thai: " << transaction->status << std::endl;
                std::cout << "Mo ta: " << transaction->description << std::endl;
                foundTransactions = true;
            }
            delete transaction; // Giai phong bo nho cho Transaction*
        }
    }

    if (!foundTransactions) {
        std::cout << "Chua co giao dich nao." << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
    // unique_ptr userWallet se tu dong giai phong bo nho khi ra khoi scope
}

// Menu cho nguoi dung thong thuong
void normalUserMenu() {
    int choice;
    do {
        std::cout << "\n--- Menu nguoi dung thong thuong (" << currentUser->getUsername() << ") ---" << std::endl;
        std::cout << "1. Xem thong tin ca nhan" << std::endl;
        std::cout << "2. Cap nhat thong tin ca nhan" << std::endl;
        std::cout << "3. Thay doi mat khau" << std::endl;
        std::cout << "4. Xem thong tin vi diem thuong" << std::endl;
        std::cout << "5. Chuyen diem" << std::endl;
        std::cout << "6. Xem lich su giao dich" << std::endl;
        std::cout << "0. Dang xuat" << std::endl;
        std::cout << "Nhap lua chon cua ban: ";
        std::cin >> choice;
        clearInputBuffer();

        switch (choice) {
            case 1: currentUser->displayUserInfo(); break;
            case 2: updateProfile(currentUser.get()); break; // Truyen raw pointer
            case 3: changePassword(); break;
            case 4: viewWalletInfo(); break;
            case 5: transferPoints(); break;
            case 6: viewTransactionHistory(); break;
            case 0:
                std::cout << "Dang xuat thanh cong." << std::endl;
                currentUser.reset(); // Giai phong unique_ptr
                break;
            default: std::cout << "Lua chon khong hop le. Vui long thu lai." << std::endl; break;
        }
    } while (choice != 0);
}

// Ham tao tai khoan moi boi admin
void adminCreateNewAccount() {
    std::string username, fullName, email, phoneNumber, userTypeChoice;
    bool isAutoGeneratedPassword = false;
    std::string password = "";

    std::cout << "\n--- Tao tai khoan moi (Admin) ---" << std::endl;
    std::cout << "Nhap ten dang nhap: ";
    std::cin >> username;
    clearInputBuffer();

    // Su dung unique_ptr de tu dong quan ly bo nho cho doi tuong user doc tu file
    std::unique_ptr<User> existingUser = std::unique_ptr<User>(User::loadFromFile(username));
    if (existingUser != nullptr) {
        std::cout << "Ten dang nhap da ton tai. Vui long chon ten khac." << std::endl;
        return;
    }

    std::cout << "Co muon tu dong sinh mat khau? (y/n): ";
    char autoGenChoice;
    std::cin >> autoGenChoice;
    clearInputBuffer();
    if (tolower(autoGenChoice) == 'y') {
        isAutoGeneratedPassword = true;
        password = Utils::generateRandomPassword();
        std::cout << "Mat khau tu dong sinh: " << password << std::endl;
    } else {
        std::string confirmPassword;
        do {
            std::cout << "Nhap mat khau: ";
            std::cin >> password;
            clearInputBuffer();
            std::cout << "Xac nhan mat khau: ";
            std::cin >> confirmPassword;
            clearInputBuffer();
            if (password != confirmPassword) {
                std::cout << "Mat khau xac nhan khong khop. Vui long nhap lai." << std::endl;
            }
        } while (password != confirmPassword);
    }

    std::cout << "Nhap ho va ten: ";
    std::getline(std::cin, fullName);

    std::cout << "Nhap email: ";
    std::cin >> email;
    clearInputBuffer();

    std::cout << "Nhap so dien thoai: ";
    std::cin >> phoneNumber;
    clearInputBuffer();

    std::cout << "Chon loai tai khoan (normal/admin): ";
    std::cin >> userTypeChoice;
    clearInputBuffer();
    if (userTypeChoice != "normal" && userTypeChoice != "admin") {
        userTypeChoice = "normal"; // Mac dinh la normal neu nhap sai
        std::cout << "Loai tai khoan khong hop le. Mac dinh la 'normal'." << std::endl;
    }

    User newUser(username, Utils::hashPassword(password), fullName, email, phoneNumber, userTypeChoice, isAutoGeneratedPassword);
    if (newUser.saveToFile()) { // Ham saveToFile da tu dong cap nhat file index
        std::cout << "Tao tai khoan thanh cong!" << std::endl;
        // Tao vi cho nguoi dung moi
        Wallet newWallet(newUser.getUserId());
        if (newWallet.saveToFile()) { // Ham saveToFile da tu dong cap nhat file index
            std::cout << "Da tao vi diem thuong cho tai khoan moi voi ID: " << newWallet.walletId << std::endl;
        } else {
            std::cerr << "Loi: Khong the tao vi diem thuong cho tai khoan moi." << std::endl;
        }
    } else {
        std::cout << "Tao tai khoan that bai. Vui long thu lai." << std::endl;
    }
}

// Ham theo doi danh sach nguoi dung (admin)
// (Da sua doi de doc tu file index)
void adminViewAllUsers() {
    std::cout << "\n--- Danh sach tat ca tai khoan ---" << std::endl;
    std::string userIndexFile = "data/user_index.txt"; // File index nguoi dung
    std::vector<std::string> usernames = Utils::readAllLines(userIndexFile); // Doc tat ca usernames tu file index

    if (usernames.empty()) {
        std::cout << "Chua co tai khoan nao trong he thong." << std::endl;
        return;
    }

    bool foundUsers = false;
    for (const std::string& username : usernames) {
        // Su dung unique_ptr de tu dong quan ly bo nho
        std::unique_ptr<User> user = std::unique_ptr<User>(User::loadFromFile(username));
        if (user) {
            std::cout << "------------------------------------" << std::endl;
            std::cout << "User ID: " << user->getUserId() << std::endl;
            std::cout << "Ten dang nhap: " << user->getUsername() << std::endl;
            std::cout << "Ho va ten: " << user->getFullName() << std::endl;
            std::cout << "Email: " << user->getEmail() << std::endl;
            std::cout << "Loai nguoi dung: " << user->getUserType() << std::endl;
            foundUsers = true;
        }
    }
    if (!foundUsers) { // Truong hop cac username trong index bi loi file hoac file bi xoa
        std::cout << "Khong the tai duoc thong tin tai khoan nao." << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
}

// Ham admin dieu chinh thong tin cua tai khoan khac
void adminUpdateOtherAccount() {
    std::string targetUsername;
    std::cout << "\n--- Cap nhat thong tin tai khoan khac (Admin) ---" << std::endl;
    std::cout << "Nhap ten dang nhap cua tai khoan muon cap nhat: ";
    std::cin >> targetUsername;
    clearInputBuffer();

    // Su dung unique_ptr de tu dong quan ly bo nho
    std::unique_ptr<User> targetUser = std::unique_ptr<User>(User::loadFromFile(targetUsername));
    if (!targetUser) {
        std::cout << "Tai khoan " << targetUsername << " khong ton tai." << std::endl;
        return;
    }

    std::cout << "Thong tin hien tai cua " << targetUser->getUsername() << ":" << std::endl;
    targetUser->displayUserInfo();

    // Goi ham updateProfile, truyen raw pointer
    updateProfile(targetUser.get(), true);

    // unique_ptr targetUser se tu dong giai phong bo nho khi ra khoi scope
}

// Ham xem tat ca lich su giao dich (admin)
void adminViewAllTransactions() {
    std::cout << "\n--- Tat ca lich su giao dich ---" << std::endl;
    std::vector<std::string> lines = Utils::readAllLines("data/transactions.log");
    bool foundTransactions = false;

    if (lines.empty()) {
        std::cout << "Chua co giao dich nao trong he thong." << std::endl;
        return;
    }

    for (const std::string& line : lines) {
        Transaction* transaction = Transaction::fromString(line);
        if (transaction) {
            std::cout << "------------------------------------" << std::endl;
            std::cout << "ID Giao dich: " << transaction->transactionId << std::endl;
            std::cout << "Tu vi: " << transaction->senderWalletId << std::endl;
            std::cout << "Den vi: " << transaction->receiverWalletId << std::endl;
            std::cout << "So diem: " << std::fixed << std::setprecision(2) << transaction->amount << std::endl;
            std::cout << "Thoi gian: " << Utils::timeToString(transaction->timestamp) << std::endl;
            std::cout << "Trang thai: " << transaction->status << std::endl;
            std::cout << "Mo ta: " << transaction->description << std::endl;
            foundTransactions = true;
            delete transaction; // Giai phong bo nho cho Transaction*
        }
    }

    if (!foundTransactions) {
        std::cout << "Khong co giao dich nao." << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
}


// Menu cho nguoi dung quan ly
void adminUserMenu() {
    int choice;
    do {
        std::cout << "\n--- Menu nguoi dung quan ly (" << currentUser->getUsername() << ") ---" << std::endl;
        std::cout << "1. Xem thong tin ca nhan" << std::endl;
        std::cout << "2. Cap nhat thong tin ca nhan (cua chinh minh)" << std::endl;
        std::cout << "3. Thay doi mat khau (cua chinh minh)" << std::endl;
        std::cout << "4. Theo doi danh sach tat ca tai khoan" << std::endl;
        std::cout << "5. Tao tai khoan moi" << std::endl;
        std::cout << "6. Dieu chinh thong tin tai khoan khac" << std::endl;
        std::cout << "7. Xem tat ca lich su giao dich" << std::endl;
        std::cout << "0. Dang xuat" << std::endl;
        std::cout << "Nhap lua chon cua ban: ";
        std::cin >> choice;
        clearInputBuffer();

        switch (choice) {
            case 1: currentUser->displayUserInfo(); break;
            case 2: updateProfile(currentUser.get()); break; // Truyen raw pointer
            case 3: changePassword(); break;
            case 4: adminViewAllUsers(); break;
            case 5: adminCreateNewAccount(); break;
            case 6: adminUpdateOtherAccount(); break;
            case 7: adminViewAllTransactions(); break;
            case 0:
                std::cout << "Dang xuat thanh cong." << std::endl;
                currentUser.reset(); // Giai phong unique_ptr
                break;
            default: std::cout << "Lua chon khong hop le. Vui long thu lai." << std::endl; break;
        }
    } while (choice != 0);
}

int main() {
    initializeSystem(); // Khoi tao he thong

    int choice;
    do {
        std::cout << "\n--- He thong quan ly diem thuong ---" << std::endl;
        std::cout << "1. Dang ky" << std::endl;
        std::cout << "2. Dang nhap" << std::endl;
        std::cout << "0. Thoat" << std::endl;
        std::cout << "Nhap lua chon cua ban: ";
        std::cin >> choice;
        clearInputBuffer();

        switch (choice) {
            case 1:
                registerAccount();
                break;
            case 2:
                if (login()) {
                    if (currentUser->getUserType() == "normal") {
                        normalUserMenu();
                    } else if (currentUser->getUserType() == "admin") {
                        adminUserMenu();
                    }
                }
                break;
            case 0:
                std::cout << "Cam on ban da su dung he thong!" << std::endl;
                break;
            default:
                std::cout << "Lua chon khong hop le. Vui long thu lai." << std::endl;
                break;
        }
    } while (choice != 0);

    return 0;
}

