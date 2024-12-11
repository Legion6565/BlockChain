#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "sha256.h"

const std::string BLOCK_DIR = "blocks";
#define START_HASH "00000000000000000000000000000000000000000000000000000000000000000"
#define DIFFICULTY "0000"

struct Transaction {
    std::string sender;
    std::string receiver;
    int amount;
};

class Blockchain {
private:
    std::map<std::string, int> balances;
    std::vector<Transaction> transactionBuffer;

    std::string calculateHash(const std::string& data) {
        return sha256(data);
    }

    void writeGenesisBlock() {
        std::string filename = BLOCK_DIR + "/block0.txt";
        std::ofstream genesisFile(filename);
        if (genesisFile.is_open()) {
            genesisFile << START_HASH << "\n\n0";
            genesisFile.close();
            std::cout << "Genesis block written successfully." << std::endl;
        }
        else {
            std::cerr << "Failed to create genesis block." << std::endl;
        }
    }

    bool validateBlock(int blockNumber, std::vector<Transaction>& transactionsOut) {
        std::string filename = BLOCK_DIR + "/block" + std::to_string(blockNumber) + ".txt";
        if (!std::filesystem::exists(filename)) {
            std::cout << "Block file does not exist: " << filename << std::endl;
            return false;
        }
        std::ifstream blockFile(filename);
        if (!blockFile.is_open()) {
            std::cout << "Failed to open block file: " << filename << std::endl;
            return false;
        }

        std::string line, hash, nonce;
        std::vector<Transaction> transactions;
        std::string blockData;

        if (blockNumber == 0) {
            if (std::getline(blockFile, line)) {
                hash = line;
                if (std::getline(blockFile, line)) {
                    nonce = line;
                }
            }
            if (hash == START_HASH) {
                std::cout << "Genesis block hash validated." << std::endl;
                return true;
            }
        }
        else {
            while (std::getline(blockFile, line) && !line.empty()) {
                std::istringstream iss(line);
                std::string sender, receiver;
                int amount;
                if (iss >> sender >> receiver >> amount) {
                    transactions.push_back({ sender, receiver, amount });
                    blockData += line + "\n";
                }
            }

            if (std::getline(blockFile, line) && line.empty() && std::getline(blockFile, line)) {
                std::istringstream iss(line);
                iss >> hash >> nonce;
            }
            else {
                std::cout << "Failed to read hash and nonce." << std::endl;
                return false;
            }

            std::string calculatedHash = calculateHash(blockData + "\n" + nonce);
            std::cout << "Calculated hash: " << calculatedHash << std::endl;
            std::cout << "Stored hash: " << hash << std::endl;
            if (calculatedHash == hash && hash.substr(0, std::string(DIFFICULTY).length()) == DIFFICULTY) {
                transactionsOut = transactions;
                return true;
            }
        }

        std::cout << "Block validation failed." << std::endl;
        return false;
    }

    void updateBalances(const std::vector<Transaction>& transactions) {
        for (const auto& t : transactions) {
            if (t.sender != "System") {
                balances[t.sender] -= t.amount;
            }
            balances[t.receiver] += t.amount;
        }
    }

public:
    Blockchain() {
        if (!std::filesystem::exists(BLOCK_DIR)) {
            std::filesystem::create_directory(BLOCK_DIR);
            writeGenesisBlock();
        }
        readBlockchain();
    }

    void readBlockchain() {
        for (int i = 0;; i++) {
            std::string filename = BLOCK_DIR + "/block" + std::to_string(i) + ".txt";
            if (!std::filesystem::exists(filename)) {
                std::cout << "No more blocks to read. Stopping at block " << i << std::endl;
                break;
            }
            std::vector<Transaction> transactions;
            if (!validateBlock(i, transactions)) {
                if (i == 0) {
                    std::cerr << "Genesis block is invalid!" << std::endl;
                    exit(1);
                }
                std::cout << "Block " << i << " is invalid!" << std::endl;
                break;
            }
            updateBalances(transactions);
            std::cout << "Balances after reading block " << i << ":" << std::endl;
            for (const auto& p : balances) {
                std::cout << p.first << ": " << p.second << std::endl;
            }
        }
    }

    void addTransaction(const std::string& sender, const std::string& receiver, int amount) {
        if (sender != "System" && (balances.find(sender) == balances.end() || balances[sender] < amount)) {
            std::cout << "Insufficient funds for " << sender << std::endl;
            return;
        }
        transactionBuffer.push_back({ sender, receiver, amount });
    }

    void createBlock() {
        std::string blockData;
        for (const auto& t : transactionBuffer) {
            blockData += t.sender + " " + t.receiver + " " + std::to_string(t.amount) + "\n";
        }
        blockData += "\n";

        std::cout << "Block Data before hashing:\n" << blockData << std::endl;

        std::string hash, nonce;
        for (int i = 0;; i++) {
            std::string testData = blockData + std::to_string(i);
            hash = calculateHash(testData);
            if (hash.substr(0, std::string(DIFFICULTY).length()) == DIFFICULTY) {
                nonce = std::to_string(i);
                break;
            }
        }

        std::size_t nextBlockNum = std::distance(
            std::filesystem::directory_iterator(std::filesystem::path(BLOCK_DIR)),
            std::filesystem::directory_iterator()
        );
        std::string filename = BLOCK_DIR + "/block" + std::to_string(nextBlockNum) + ".txt";
        std::ofstream newBlock(filename);
        if (newBlock.is_open()) {
            newBlock << blockData << hash << " " << nonce;
            newBlock.close();
            updateBalances(transactionBuffer);
            std::cout << "Balances after block creation:" << std::endl;
            for (const auto& p : balances) {
                std::cout << p.first << ": " << p.second << std::endl;
            }
            transactionBuffer.clear();
        }
        else {
            std::cout << "Failed to write new block." << std::endl;
        }
    }

    void userInteraction() {
        while (true) {
            std::string command;
            std::cout << "Enter command (add/create/exit/balance/create_user): ";
            std::cin >> command;

            if (command == "add") {
                std::string sender, receiver;
                int amount;
                std::cin >> sender >> receiver >> amount;
                addTransaction(sender, receiver, amount);
            }
            else if (command == "create") {
                createBlock();
            }
            else if (command == "balance") {
                std::string user;
                std::cin >> user;
                auto it = balances.find(user);
                if (it != balances.end()) {
                    std::cout << "Balance of " << user << ": " << it->second << std::endl;
                }
                else {
                    std::cout << "User " << user << " does not exist in the system." << std::endl;
                }
            }
            else if (command == "create_user") {
                std::string newUser;
                std::cin >> newUser;
                if (balances.find(newUser) == balances.end()) {
                    balances[newUser] = 0;
                    std::cout << "New user " << newUser << " created with balance 0." << std::endl;
                }
                else {
                    std::cout << "User " << newUser << " already exists." << std::endl;
                }
            }
            else if (command == "exit") {
                break;
            }
            else {
                std::cout << "Unknown command" << std::endl;
            }
        }
    }
};

int main() {
    Blockchain blockchain;
    blockchain.addTransaction("System", "Alice", 1000);
    blockchain.createBlock();
    blockchain.userInteraction();
    return 0;
}