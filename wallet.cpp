// wallet.cpp
#include "wallet.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip> // De dinh dang so thuc (std::fixed, std::setprecision)
#include <stdexcept> // De su dung std::runtime_error

// --- Trien khai cho cau truc Transaction ---

std::string Transaction::toString() const {
    std::stringstream ss;
    ss << "transactionId:" << transactionId << "|"
       << "senderWalletId:" << senderWalletId << "|"
       << "receiverWalletId:" << receiverWalletId << "|"
       << "amount:" << std::fixed << std::setprecision(2) << amount << "|"
       << "timestamp:" << timestamp << "|" // Luu timestamp
       << "status:" << status << "|"
       << "description:" << description;
    return ss.str();
}

Transaction* Transaction::fromString(const std::string& data) {
    std::vector<std::string> parts = Utils::splitString(data, '|');
    if (parts.size() != 7) { // 7 truong du lieu
        return nullptr;
    }

    Transaction* transaction = new Transaction();
    for (const std::string& part : parts) {
        std::vector<std::string> keyValue = Utils::splitString(part, ':');
        if (keyValue.size() < 2) continue;
        std::string key = Utils::trimString(keyValue[0]);
        std::string value = Utils::trimString(keyValue[1]);

        if (key == "transactionId") transaction->transactionId = value;
        else if (key == "senderWalletId") transaction->senderWalletId = value;
        else if (key == "receiverWalletId") transaction->receiverWalletId = value;
        else if (key == "amount") transaction->amount = std::stod(value);
        else if (key == "timestamp") transaction->timestamp = std::stoll(value);
        else if (key == "status") transaction->status = value;
        else if (key == "description") transaction->description = value;
    }
    return transaction;
}

// --- Trien khai cho lop Wallet ---

// Constructor cho vi moi
Wallet::Wallet(std::string ownerUserId)
    : ownerUserId(ownerUserId), balance(0.0) {
    this->walletId = Utils::generateUniqueId(); // Sinh ID duy nhat cho vi
}

// Constructor khi doc tu file
Wallet::Wallet(std::string walletId, std::string ownerUserId, double balance)
    : walletId(walletId), ownerUserId(ownerUserId), balance(balance) {}

// Hien thi thong tin vi
void Wallet::displayWalletInfo() const {
    std::cout << "--- Thong tin vi ---" << std::endl;
    std::cout << "ID Vi: " << walletId << std::endl;
    std::cout << "ID Nguoi so huu: " << ownerUserId << std::endl;
    std::cout << "So du: " << std::fixed << std::setprecision(2) << balance << " diem" << std::endl;
    std::cout << "--------------------------" << std::endl;
}

// Chuyen doi doi tuong Wallet thanh chuoi de luu vao file
std::string Wallet::toString() const {
    std::stringstream ss;
    ss << "walletId:" << walletId << "\n"
       << "ownerUserId:" << ownerUserId << "\n"
       << "balance:" << std::fixed << std::setprecision(2) << balance;
    return ss.str();
}

// Tao doi tuong Wallet tu chuoi doc tu file
Wallet* Wallet::fromString(const std::string& data) {
    std::string line;
    std::istringstream iss(data);

    std::string walletId, ownerUserId;
    double balance = 0.0;

    while (std::getline(iss, line)) {
        std::vector<std::string> parts = Utils::splitString(line, ':');
        if (parts.size() < 2) continue;
        std::string key = Utils::trimString(parts[0]);
        std::string value = Utils::trimString(parts[1]);

        if (key == "walletId") walletId = value;
        else if (key == "ownerUserId") ownerUserId = value;
        else if (key == "balance") balance = std::stod(value);
    }

    if (walletId.empty() || ownerUserId.empty()) {
        return nullptr; // Du lieu khong hop le
    }

    return new Wallet(walletId, ownerUserId, balance);
}

// Luu doi tuong Wallet vao file
// (Da them cap nhat file index)
bool Wallet::saveToFile() const {
    std::string walletDir = "data/wallets/";
    std::string walletIndexFile = "data/wallet_index.txt"; // File index vi
    
    // Dam bao thu muc 'data' va 'data/wallets' ton tai
    Utils::createDirectoryIfNotExists("data");
    Utils::createDirectoryIfNotExists(walletDir);

    std::string filename = walletDir + walletId + ".txt"; // Luu theo walletId
    bool success = Utils::writeToFile(filename, toString());

    if (success) {
        // Kiem tra xem walletId da co trong index chua truoc khi them de tranh trung lap
        std::vector<std::string> existingWalletIds = Utils::readAllLines(walletIndexFile);
        bool foundInIndex = false;
        for(const std::string& wid : existingWalletIds) {
            if(wid == walletId) {
                foundInIndex = true;
                break;
            }
        }
        if(!foundInIndex) {
            Utils::appendToFile(walletIndexFile, walletId); // Them walletId vao index file
        }
    }
    return success;
}

// Tai doi tuong Wallet tu file dua tren walletId
// Thay doi kieu tra ve tu Wallet* sang std::unique_ptr<Wallet>
std::unique_ptr<Wallet> Wallet::loadFromFile(const std::string& walletId) {
    std::string walletDir = "data/wallets/";
    std::string filename = walletDir + walletId + ".txt";
    std::string content = Utils::readFileContent(filename);
    if (content.empty()) {
        return nullptr; // Khong tim thay file hoac file trong
    }
    // Tra ve unique_ptr tu con tro duoc cap phat dong
    return std::unique_ptr<Wallet>(fromString(content));
}

// Tai doi tuong Wallet tu file dua tren ownerUserId
// Thay doi kieu tra ve tu Wallet* sang std::unique_ptr<Wallet>
std::unique_ptr<Wallet> Wallet::loadWalletByUserId(const std::string& userId) {
    std::string walletIndexFile = "data/wallet_index.txt"; // File index vi
    std::vector<std::string> walletIds = Utils::readAllLines(walletIndexFile); // Doc tat ca walletIds tu file index

    for (const std::string& walletId : walletIds) {
        // Su dung auto de unique_ptr tu dong quan ly bo nho
        if (auto wallet = loadFromFile(walletId)) { // loadFromFile tra ve unique_ptr
            if (wallet->ownerUserId == userId) {
                return wallet; // Tra ve unique_ptr tim duoc
            }
        }
    }
    return nullptr; // Khong tim thay vi cho userId nay
}

// Phuong thuc thuc hien giao dich chuyen diem (atomic)
bool Wallet::transferPoints(const std::string& senderUserId, const std::string& receiverWalletId, double amount) {
    // Su dung unique_ptr de tu dong giai phong bo nho
    std::unique_ptr<Wallet> senderWallet = Wallet::loadWalletByUserId(senderUserId);
    if (!senderWallet) {
        std::cout << "Loi: Khong tim thay vi cua nguoi gui." << std::endl;
        return false;
    }

    std::unique_ptr<Wallet> receiverWallet = Wallet::loadFromFile(receiverWalletId);
    if (!receiverWallet) {
        std::cout << "Loi: Khong tim thay vi cua nguoi nhan." << std::endl;
        return false; // unique_ptr se tu dong giai phong senderWallet khi ra khoi scope
    }

    // Khong cho phep chuyen diem cho chinh minh
    if (senderWallet->walletId == receiverWallet->walletId) {
        std::cout << "Loi: Khong the chuyen diem cho chinh vi cua ban." << std::endl;
        return false;
    }

    Transaction newTransaction;
    newTransaction.transactionId = Utils::generateUniqueId();
    newTransaction.senderWalletId = senderWallet->walletId;
    newTransaction.receiverWalletId = receiverWallet->walletId;
    newTransaction.amount = amount;
    newTransaction.timestamp = time(0);
    newTransaction.description = "Chuyen diem";

    bool transactionSuccess = false; // Bien de luu ket qua giao dich

    try {
        if (senderWallet->balance < amount) {
            newTransaction.status = "failed";
            newTransaction.description = "So du khong du. Khong the tien hanh.";
            std::cout << "So du khong du. Khong the tien hanh giao dich." << std::endl;
            // Khong throw ngoai le o day, chi dat status va return false
            transactionSuccess = false; // Cap nhat ket qua
        } else {
            // Tac vu 3_1: Tru diem tu vi A
            senderWallet->balance -= amount;
            // Tac vu 3_2: Cong diem vao vi B
            receiverWallet->balance += amount;

            // Luu thay doi vao file
            bool senderSaveSuccess = senderWallet->saveToFile();
            bool receiverSaveSuccess = receiverWallet->saveToFile();

            if (!senderSaveSuccess || !receiverSaveSuccess) {
                // Neu co loi khi luu, phuc hoi trang thai ban dau
                senderWallet->balance += amount; // Hoan lai diem cho nguoi gui
                receiverWallet->balance -= amount; // Tru diem da cong cho nguoi nhan
                senderWallet->saveToFile(); // Co gang luu lai trang thai cu
                receiverWallet->saveToFile(); // Co gang luu lai trang thai cu
                throw std::runtime_error("Loi khi luu du lieu vi. Giao dich da duoc hoan tac.");
            }

            newTransaction.status = "completed";
            std::cout << "Chuyen diem thanh cong!" << std::endl;
            transactionSuccess = true; // Cap nhat ket qua
        }

    } catch (const std::exception& e) {
        newTransaction.status = "failed";
        newTransaction.description = "Giao dich that bai do loi he thong: " + std::string(e.what());
        std::cerr << "Giao dich that bai: " << e.what() << ". Dang hoan tac cac thay doi." << std::endl;
        transactionSuccess = false; // Cap nhat ket qua
    }

    // Ghi log giao dich vao transactions.log bat ke thanh cong hay that bai
    // Phan nay se luon duoc thuc thi sau try-catch block
    Utils::appendToFile("data/transactions.log", newTransaction.toString());
    
    // unique_ptr senderWallet va receiverWallet se tu dong giai phong bo nho khi ra khoi ham

    return transactionSuccess; // Tra ve ket qua giao dich
}

