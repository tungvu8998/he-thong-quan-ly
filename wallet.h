// wallet.h
#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <memory> // Them dong nay de su dung std::unique_ptr
#include "utils.h"

// Cau truc de luu thong tin giao dich
struct Transaction {
    std::string transactionId;
    std::string senderWalletId;
    std::string receiverWalletId;
    double amount;
    time_t timestamp;
    std::string status; // "completed" (hoan thanh), "pending" (cho xu ly), "failed" (that bai)
    std::string description;

    // Chuyen doi Transaction thanh chuoi de luu
    std::string toString() const;
    // Tao Transaction tu chuoi doc tu file
    static Transaction* fromString(const std::string& data);
};

class Wallet {
public:
    std::string walletId;    // ID duy nhat cho vi
    std::string ownerUserId; // ID nguoi dung so huu vi
    double balance;          // So du diem

    // Constructor cho vi moi
    Wallet(std::string ownerUserId);
    // Constructor khi doc tu file
    Wallet(std::string walletId, std::string ownerUserId, double balance);

    // Cac ham thanh vien
    void displayWalletInfo() const;

    // Phuong thuc de chuyen doi doi tuong Wallet thanh chuoi de luu vao file
    std::string toString() const;
    // Phuong thuc tinh de tao doi tuong Wallet tu chuoi doc tu file
    static Wallet* fromString(const std::string& data);

    // Phuong thuc de luu doi tuong Wallet vao file
    bool saveToFile() const;

    // Phuong thuc tinh de tai doi tuong Wallet tu file dua tren walletId
    // Thay doi kieu tra ve tu Wallet* sang std::unique_ptr<Wallet>
    static std::unique_ptr<Wallet> loadFromFile(const std::string& walletId);
    
    // Phuong thuc tinh de tai doi tuong Wallet tu file dua tren ownerUserId
    // Thay doi kieu tra ve tu Wallet* sang std::unique_ptr<Wallet>
    static std::unique_ptr<Wallet> loadWalletByUserId(const std::string& userId);

    // Phuong thuc thuc hien giao dich chuyen diem (atomic)
    // Tra ve true neu thanh cong, false neu that bai
    static bool transferPoints(const std::string& senderUserId, const std::string& receiverWalletId, double amount);
};

#endif // WALLET_H

