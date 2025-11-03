#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>

using namespace std;

// ==================== Utility Functions ====================

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

vector<string> split(const string& str, char delim) {
    vector<string> result;
    stringstream ss(str);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

bool isValidUserID(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidPassword(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidUsername(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool isValidISBN(const string& s) {
    if (s.empty() || s.length() > 20) return false;
    for (char c : s) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool isValidBookString(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    for (char c : s) {
        if (c < 33 || c > 126 || c == '"') return false;
    }
    return true;
}

bool isValidKeyword(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    vector<string> parts = split(s, '|');
    set<string> unique_check;
    for (const string& part : parts) {
        if (part.empty()) return false;
        if (unique_check.count(part)) return false;
        unique_check.insert(part);
        for (char c : part) {
            if (c < 33 || c > 126 || c == '"') return false;
        }
    }
    return true;
}

bool isValidQuantity(const string& s) {
    if (s.empty() || s.length() > 10) return false;
    if (s[0] == '0' && s.length() > 1) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool isValidPrice(const string& s) {
    if (s.empty() || s.length() > 13) return false;
    size_t dotPos = s.find('.');

    if (dotPos == string::npos) {
        // No decimal point - must be all digits
        for (char c : s) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    // Has decimal point
    if (dotPos == 0) return false; // Can't start with '.'
    if (dotPos == s.length() - 1) return false; // Can't end with '.'
    if (s.find('.', dotPos + 1) != string::npos) return false; // Multiple dots

    // Check all chars except dot are digits
    for (size_t i = 0; i < s.length(); i++) {
        if (i != dotPos && !isdigit(s[i])) return false;
    }

    return true;
}

long long parseQuantity(const string& s) {
    return stoll(s);
}

double parsePrice(const string& s) {
    return stod(s);
}

// ==================== Data Structures ====================

struct Account {
    char userID[31];
    char password[31];
    int privilege;
    char username[31];

    Account() {
        memset(userID, 0, sizeof(userID));
        memset(password, 0, sizeof(password));
        privilege = 0;
        memset(username, 0, sizeof(username));
    }
};

struct Book {
    char ISBN[21];
    char name[61];
    char author[61];
    char keyword[61];
    double price;
    long long quantity;

    Book() {
        memset(ISBN, 0, sizeof(ISBN));
        memset(name, 0, sizeof(name));
        memset(author, 0, sizeof(author));
        memset(keyword, 0, sizeof(keyword));
        price = 0.0;
        quantity = 0;
    }
};

struct Transaction {
    double amount;
    int type; // 1: income, -1: expenditure
};

// ==================== File-based Storage ====================

class AccountManager {
private:
    const string filename = "accounts.dat";

public:
    AccountManager() {
        // Initialize root account if needed
        ifstream test(filename);
        if (!test.good()) {
            Account root;
            strcpy(root.userID, "root");
            strcpy(root.password, "sjtu");
            root.privilege = 7;
            strcpy(root.username, "root");
            addAccount(root);
        }
    }

    void addAccount(const Account& acc) {
        ofstream file(filename, ios::binary | ios::app);
        file.write(reinterpret_cast<const char*>(&acc), sizeof(Account));
        file.close();
    }

    bool findAccount(const string& userID, Account& acc) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) return false;

        while (file.read(reinterpret_cast<char*>(&acc), sizeof(Account))) {
            if (string(acc.userID) == userID) {
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    bool deleteAccount(const string& userID) {
        vector<Account> accounts;
        ifstream inFile(filename, ios::binary);
        Account acc;
        bool found = false;

        while (inFile.read(reinterpret_cast<char*>(&acc), sizeof(Account))) {
            if (string(acc.userID) != userID) {
                accounts.push_back(acc);
            } else {
                found = true;
            }
        }
        inFile.close();

        if (!found) return false;

        ofstream outFile(filename, ios::binary | ios::trunc);
        for (const auto& a : accounts) {
            outFile.write(reinterpret_cast<const char*>(&a), sizeof(Account));
        }
        outFile.close();
        return true;
    }

    bool updatePassword(const string& userID, const string& newPassword) {
        vector<Account> accounts;
        ifstream inFile(filename, ios::binary);
        Account acc;
        bool found = false;

        while (inFile.read(reinterpret_cast<char*>(&acc), sizeof(Account))) {
            if (string(acc.userID) == userID) {
                strcpy(acc.password, newPassword.c_str());
                found = true;
            }
            accounts.push_back(acc);
        }
        inFile.close();

        if (!found) return false;

        ofstream outFile(filename, ios::binary | ios::trunc);
        for (const auto& a : accounts) {
            outFile.write(reinterpret_cast<const char*>(&a), sizeof(Account));
        }
        outFile.close();
        return true;
    }
};

class BookManager {
private:
    const string filename = "books.dat";

public:
    void addOrUpdateBook(const Book& book) {
        vector<Book> books;
        ifstream inFile(filename, ios::binary);
        Book b;
        bool found = false;

        while (inFile.read(reinterpret_cast<char*>(&b), sizeof(Book))) {
            if (string(b.ISBN) == string(book.ISBN)) {
                books.push_back(book);
                found = true;
            } else {
                books.push_back(b);
            }
        }
        inFile.close();

        if (!found) {
            books.push_back(book);
        }

        ofstream outFile(filename, ios::binary | ios::trunc);
        for (const auto& bk : books) {
            outFile.write(reinterpret_cast<const char*>(&bk), sizeof(Book));
        }
        outFile.close();
    }

    bool findBook(const string& ISBN, Book& book) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) return false;

        while (file.read(reinterpret_cast<char*>(&book), sizeof(Book))) {
            if (string(book.ISBN) == ISBN) {
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    vector<Book> getAllBooks() {
        vector<Book> books;
        ifstream file(filename, ios::binary);
        if (!file.is_open()) return books;

        Book book;
        while (file.read(reinterpret_cast<char*>(&book), sizeof(Book))) {
            books.push_back(book);
        }
        file.close();
        return books;
    }

    vector<Book> searchByISBN(const string& ISBN) {
        vector<Book> result;
        vector<Book> all = getAllBooks();
        for (const auto& book : all) {
            if (string(book.ISBN) == ISBN) {
                result.push_back(book);
            }
        }
        return result;
    }

    vector<Book> searchByName(const string& name) {
        vector<Book> result;
        vector<Book> all = getAllBooks();
        for (const auto& book : all) {
            if (string(book.name) == name) {
                result.push_back(book);
            }
        }
        return result;
    }

    vector<Book> searchByAuthor(const string& author) {
        vector<Book> result;
        vector<Book> all = getAllBooks();
        for (const auto& book : all) {
            if (string(book.author) == author) {
                result.push_back(book);
            }
        }
        return result;
    }

    vector<Book> searchByKeyword(const string& keyword) {
        vector<Book> result;
        vector<Book> all = getAllBooks();
        for (const auto& book : all) {
            string keywords = string(book.keyword);
            vector<string> parts = split(keywords, '|');
            for (const string& kw : parts) {
                if (kw == keyword) {
                    result.push_back(book);
                    break;
                }
            }
        }
        return result;
    }
};

class TransactionManager {
private:
    const string filename = "transactions.dat";

public:
    void addTransaction(double amount, int type) {
        Transaction trans;
        trans.amount = amount;
        trans.type = type;

        ofstream file(filename, ios::binary | ios::app);
        file.write(reinterpret_cast<const char*>(&trans), sizeof(Transaction));
        file.close();
    }

    vector<Transaction> getTransactions() {
        vector<Transaction> result;
        ifstream file(filename, ios::binary);
        if (!file.is_open()) return result;

        Transaction trans;
        while (file.read(reinterpret_cast<char*>(&trans), sizeof(Transaction))) {
            result.push_back(trans);
        }
        file.close();
        return result;
    }
};

class LogManager {
private:
    const string filename = "logs.txt";

public:
    void addLog(const string& log) {
        ofstream file(filename, ios::app);
        file << log << endl;
        file.close();
    }

    vector<string> getLogs() {
        vector<string> result;
        ifstream file(filename);
        if (!file.is_open()) return result;

        string line;
        while (getline(file, line)) {
            result.push_back(line);
        }
        file.close();
        return result;
    }
};

// ==================== Session Management ====================

struct Session {
    string userID;
    int privilege;
    string selectedISBN;
};

class BookstoreSystem {
private:
    AccountManager accountMgr;
    BookManager bookMgr;
    TransactionManager transMgr;
    LogManager logMgr;
    vector<Session> loginStack;

    int getCurrentPrivilege() {
        if (loginStack.empty()) return 0;
        return loginStack.back().privilege;
    }

    string& getCurrentSelectedISBN() {
        static string empty = "";
        if (loginStack.empty()) return empty;
        return loginStack.back().selectedISBN;
    }

public:
    void processCommand(const string& line) {
        string cmd = trim(line);
        if (cmd.empty()) return;

        vector<string> tokens;
        stringstream ss(cmd);
        string token;
        while (ss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) return;

        if (tokens[0] == "quit" || tokens[0] == "exit") {
            exit(0);
        } else if (tokens[0] == "su") {
            cmdSu(tokens);
        } else if (tokens[0] == "logout") {
            cmdLogout(tokens);
        } else if (tokens[0] == "register") {
            cmdRegister(tokens);
        } else if (tokens[0] == "passwd") {
            cmdPasswd(tokens);
        } else if (tokens[0] == "useradd") {
            cmdUseradd(tokens);
        } else if (tokens[0] == "delete") {
            cmdDelete(tokens);
        } else if (tokens[0] == "show") {
            cmdShow(tokens);
        } else if (tokens[0] == "buy") {
            cmdBuy(tokens);
        } else if (tokens[0] == "select") {
            cmdSelect(tokens);
        } else if (tokens[0] == "modify") {
            cmdModify(tokens);
        } else if (tokens[0] == "import") {
            cmdImport(tokens);
        } else if (tokens[0] == "log") {
            cmdLog(tokens);
        } else if (tokens[0] == "report") {
            cmdReport(tokens);
        } else {
            cout << "Invalid" << endl;
        }
    }

    void cmdSu(const vector<string>& tokens) {
        if (tokens.size() < 2 || tokens.size() > 3) {
            cout << "Invalid" << endl;
            return;
        }

        string userID = tokens[1];
        string password = tokens.size() == 3 ? tokens[2] : "";

        if (!isValidUserID(userID) || (tokens.size() == 3 && !isValidPassword(password))) {
            cout << "Invalid" << endl;
            return;
        }

        Account acc;
        if (!accountMgr.findAccount(userID, acc)) {
            cout << "Invalid" << endl;
            return;
        }

        int currentPriv = getCurrentPrivilege();

        if (currentPriv > acc.privilege) {
            // Can login without password
            Session sess;
            sess.userID = userID;
            sess.privilege = acc.privilege;
            loginStack.push_back(sess);
        } else {
            if (tokens.size() != 3) {
                cout << "Invalid" << endl;
                return;
            }
            if (string(acc.password) != password) {
                cout << "Invalid" << endl;
                return;
            }
            Session sess;
            sess.userID = userID;
            sess.privilege = acc.privilege;
            loginStack.push_back(sess);
        }
    }

    void cmdLogout(const vector<string>& tokens) {
        if (tokens.size() != 1) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 1) {
            cout << "Invalid" << endl;
            return;
        }

        if (loginStack.empty()) {
            cout << "Invalid" << endl;
            return;
        }

        loginStack.pop_back();
    }

    void cmdRegister(const vector<string>& tokens) {
        if (tokens.size() != 4) {
            cout << "Invalid" << endl;
            return;
        }

        string userID = tokens[1];
        string password = tokens[2];
        string username = tokens[3];

        if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
            cout << "Invalid" << endl;
            return;
        }

        Account existing;
        if (accountMgr.findAccount(userID, existing)) {
            cout << "Invalid" << endl;
            return;
        }

        Account acc;
        strcpy(acc.userID, userID.c_str());
        strcpy(acc.password, password.c_str());
        acc.privilege = 1;
        strcpy(acc.username, username.c_str());

        accountMgr.addAccount(acc);
    }

    void cmdPasswd(const vector<string>& tokens) {
        if (tokens.size() < 3 || tokens.size() > 4) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 1) {
            cout << "Invalid" << endl;
            return;
        }

        string userID = tokens[1];
        string currentPassword = tokens.size() == 4 ? tokens[2] : "";
        string newPassword = tokens.size() == 4 ? tokens[3] : tokens[2];

        if (!isValidUserID(userID) || !isValidPassword(newPassword)) {
            cout << "Invalid" << endl;
            return;
        }

        if (tokens.size() == 4 && !isValidPassword(currentPassword)) {
            cout << "Invalid" << endl;
            return;
        }

        Account acc;
        if (!accountMgr.findAccount(userID, acc)) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() == 7) {
            // Can change without current password
            accountMgr.updatePassword(userID, newPassword);
        } else {
            if (tokens.size() != 4) {
                cout << "Invalid" << endl;
                return;
            }
            if (string(acc.password) != currentPassword) {
                cout << "Invalid" << endl;
                return;
            }
            accountMgr.updatePassword(userID, newPassword);
        }
    }

    void cmdUseradd(const vector<string>& tokens) {
        if (tokens.size() != 5) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 3) {
            cout << "Invalid" << endl;
            return;
        }

        string userID = tokens[1];
        string password = tokens[2];
        string privilegeStr = tokens[3];
        string username = tokens[4];

        if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
            cout << "Invalid" << endl;
            return;
        }

        if (privilegeStr.length() != 1 || !isdigit(privilegeStr[0])) {
            cout << "Invalid" << endl;
            return;
        }

        int privilege = privilegeStr[0] - '0';
        if (privilege != 1 && privilege != 3 && privilege != 7) {
            cout << "Invalid" << endl;
            return;
        }

        if (privilege >= getCurrentPrivilege()) {
            cout << "Invalid" << endl;
            return;
        }

        Account existing;
        if (accountMgr.findAccount(userID, existing)) {
            cout << "Invalid" << endl;
            return;
        }

        Account acc;
        strcpy(acc.userID, userID.c_str());
        strcpy(acc.password, password.c_str());
        acc.privilege = privilege;
        strcpy(acc.username, username.c_str());

        accountMgr.addAccount(acc);
    }

    void cmdDelete(const vector<string>& tokens) {
        if (tokens.size() != 2) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 7) {
            cout << "Invalid" << endl;
            return;
        }

        string userID = tokens[1];

        if (!isValidUserID(userID)) {
            cout << "Invalid" << endl;
            return;
        }

        // Check if account is logged in
        for (const auto& sess : loginStack) {
            if (sess.userID == userID) {
                cout << "Invalid" << endl;
                return;
            }
        }

        if (!accountMgr.deleteAccount(userID)) {
            cout << "Invalid" << endl;
        }
    }

    void cmdShow(const vector<string>& tokens) {
        if (tokens.size() == 1) {
            // show all books
            if (getCurrentPrivilege() < 1) {
                cout << "Invalid" << endl;
                return;
            }

            vector<Book> books = bookMgr.getAllBooks();
            sort(books.begin(), books.end(), [](const Book& a, const Book& b) {
                return string(a.ISBN) < string(b.ISBN);
            });

            if (books.empty()) {
                cout << endl;
            } else {
                for (const auto& book : books) {
                    cout << book.ISBN << "\t" << book.name << "\t" << book.author << "\t"
                         << book.keyword << "\t" << fixed << setprecision(2) << book.price << "\t"
                         << book.quantity << endl;
                }
            }
        } else if (tokens.size() == 2 && tokens[1] == "finance") {
            // show finance
            if (getCurrentPrivilege() < 7) {
                cout << "Invalid" << endl;
                return;
            }

            vector<Transaction> transactions = transMgr.getTransactions();
            double income = 0.0, expenditure = 0.0;

            for (const auto& trans : transactions) {
                if (trans.type == 1) {
                    income += trans.amount;
                } else {
                    expenditure += trans.amount;
                }
            }

            cout << "+ " << fixed << setprecision(2) << income << " - " << expenditure << endl;
        } else if (tokens.size() == 3 && tokens[1] == "finance") {
            // show finance [count]
            if (getCurrentPrivilege() < 7) {
                cout << "Invalid" << endl;
                return;
            }

            if (!isValidQuantity(tokens[2])) {
                cout << "Invalid" << endl;
                return;
            }

            long long count = parseQuantity(tokens[2]);
            vector<Transaction> transactions = transMgr.getTransactions();

            if (count > (long long)transactions.size()) {
                cout << "Invalid" << endl;
                return;
            }

            if (count == 0) {
                cout << endl;
                return;
            }

            double income = 0.0, expenditure = 0.0;
            long long start = transactions.size() - count;
            for (long long i = start; i < (long long)transactions.size(); i++) {
                if (transactions[i].type == 1) {
                    income += transactions[i].amount;
                } else {
                    expenditure += transactions[i].amount;
                }
            }

            cout << "+ " << fixed << setprecision(2) << income << " - " << expenditure << endl;
        } else if (tokens.size() == 2) {
            // show with filter
            if (getCurrentPrivilege() < 1) {
                cout << "Invalid" << endl;
                return;
            }

            string filter = tokens[1];
            vector<Book> books;

            if (filter.find("-ISBN=") == 0) {
                string isbn = filter.substr(6);
                if (!isValidISBN(isbn)) {
                    cout << "Invalid" << endl;
                    return;
                }
                books = bookMgr.searchByISBN(isbn);
            } else if (filter.find("-name=\"") == 0 && filter.back() == '"') {
                string name = filter.substr(7, filter.length() - 8);
                if (!isValidBookString(name)) {
                    cout << "Invalid" << endl;
                    return;
                }
                books = bookMgr.searchByName(name);
            } else if (filter.find("-author=\"") == 0 && filter.back() == '"') {
                string author = filter.substr(9, filter.length() - 10);
                if (!isValidBookString(author)) {
                    cout << "Invalid" << endl;
                    return;
                }
                books = bookMgr.searchByAuthor(author);
            } else if (filter.find("-keyword=\"") == 0 && filter.back() == '"') {
                string keyword = filter.substr(10, filter.length() - 11);
                if (!isValidBookString(keyword)) {
                    cout << "Invalid" << endl;
                    return;
                }
                if (keyword.find('|') != string::npos) {
                    cout << "Invalid" << endl;
                    return;
                }
                books = bookMgr.searchByKeyword(keyword);
            } else {
                cout << "Invalid" << endl;
                return;
            }

            sort(books.begin(), books.end(), [](const Book& a, const Book& b) {
                return string(a.ISBN) < string(b.ISBN);
            });

            if (books.empty()) {
                cout << endl;
            } else {
                for (const auto& book : books) {
                    cout << book.ISBN << "\t" << book.name << "\t" << book.author << "\t"
                         << book.keyword << "\t" << fixed << setprecision(2) << book.price << "\t"
                         << book.quantity << endl;
                }
            }
        } else {
            cout << "Invalid" << endl;
        }
    }

    void cmdBuy(const vector<string>& tokens) {
        if (tokens.size() != 3) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 1) {
            cout << "Invalid" << endl;
            return;
        }

        string isbn = tokens[1];
        string quantityStr = tokens[2];

        if (!isValidISBN(isbn) || !isValidQuantity(quantityStr)) {
            cout << "Invalid" << endl;
            return;
        }

        long long quantity = parseQuantity(quantityStr);
        if (quantity <= 0) {
            cout << "Invalid" << endl;
            return;
        }

        Book book;
        if (!bookMgr.findBook(isbn, book)) {
            cout << "Invalid" << endl;
            return;
        }

        if (book.quantity < quantity) {
            cout << "Invalid" << endl;
            return;
        }

        double totalCost = book.price * quantity;
        book.quantity -= quantity;
        bookMgr.addOrUpdateBook(book);

        transMgr.addTransaction(totalCost, 1);

        cout << fixed << setprecision(2) << totalCost << endl;
    }

    void cmdSelect(const vector<string>& tokens) {
        if (tokens.size() != 2) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 3) {
            cout << "Invalid" << endl;
            return;
        }

        string isbn = tokens[1];

        if (!isValidISBN(isbn)) {
            cout << "Invalid" << endl;
            return;
        }

        Book book;
        if (!bookMgr.findBook(isbn, book)) {
            // Create new book
            Book newBook;
            strcpy(newBook.ISBN, isbn.c_str());
            bookMgr.addOrUpdateBook(newBook);
        }

        getCurrentSelectedISBN() = isbn;
    }

    void cmdModify(const vector<string>& tokens) {
        if (tokens.size() < 2) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 3) {
            cout << "Invalid" << endl;
            return;
        }

        string& selectedISBN = getCurrentSelectedISBN();
        if (selectedISBN.empty()) {
            cout << "Invalid" << endl;
            return;
        }

        Book book;
        if (!bookMgr.findBook(selectedISBN, book)) {
            cout << "Invalid" << endl;
            return;
        }

        set<string> usedParams;
        string newISBN = "";

        for (size_t i = 1; i < tokens.size(); i++) {
            string param = tokens[i];

            if (param.find("-ISBN=") == 0) {
                if (usedParams.count("ISBN")) {
                    cout << "Invalid" << endl;
                    return;
                }
                usedParams.insert("ISBN");

                newISBN = param.substr(6);
                if (!isValidISBN(newISBN) || newISBN.empty()) {
                    cout << "Invalid" << endl;
                    return;
                }

                if (newISBN == string(book.ISBN)) {
                    cout << "Invalid" << endl;
                    return;
                }

                Book existing;
                if (bookMgr.findBook(newISBN, existing)) {
                    cout << "Invalid" << endl;
                    return;
                }
            } else if (param.find("-name=\"") == 0 && param.back() == '"') {
                if (usedParams.count("name")) {
                    cout << "Invalid" << endl;
                    return;
                }
                usedParams.insert("name");

                string name = param.substr(7, param.length() - 8);
                if (!isValidBookString(name) || name.empty()) {
                    cout << "Invalid" << endl;
                    return;
                }
                strcpy(book.name, name.c_str());
            } else if (param.find("-author=\"") == 0 && param.back() == '"') {
                if (usedParams.count("author")) {
                    cout << "Invalid" << endl;
                    return;
                }
                usedParams.insert("author");

                string author = param.substr(9, param.length() - 10);
                if (!isValidBookString(author) || author.empty()) {
                    cout << "Invalid" << endl;
                    return;
                }
                strcpy(book.author, author.c_str());
            } else if (param.find("-keyword=\"") == 0 && param.back() == '"') {
                if (usedParams.count("keyword")) {
                    cout << "Invalid" << endl;
                    return;
                }
                usedParams.insert("keyword");

                string keyword = param.substr(10, param.length() - 11);
                if (!isValidKeyword(keyword) || keyword.empty()) {
                    cout << "Invalid" << endl;
                    return;
                }
                strcpy(book.keyword, keyword.c_str());
            } else if (param.find("-price=") == 0) {
                if (usedParams.count("price")) {
                    cout << "Invalid" << endl;
                    return;
                }
                usedParams.insert("price");

                string priceStr = param.substr(7);
                if (!isValidPrice(priceStr) || priceStr.empty()) {
                    cout << "Invalid" << endl;
                    return;
                }
                book.price = parsePrice(priceStr);
            } else {
                cout << "Invalid" << endl;
                return;
            }
        }

        if (!newISBN.empty()) {
            // Delete old book and create new one
            vector<Book> books = bookMgr.getAllBooks();
            vector<Book> updatedBooks;

            for (const auto& b : books) {
                if (string(b.ISBN) != string(book.ISBN)) {
                    updatedBooks.push_back(b);
                }
            }

            strcpy(book.ISBN, newISBN.c_str());
            updatedBooks.push_back(book);

            ofstream outFile("books.dat", ios::binary | ios::trunc);
            for (const auto& bk : updatedBooks) {
                outFile.write(reinterpret_cast<const char*>(&bk), sizeof(Book));
            }
            outFile.close();

            selectedISBN = newISBN;
        } else {
            bookMgr.addOrUpdateBook(book);
        }
    }

    void cmdImport(const vector<string>& tokens) {
        if (tokens.size() != 3) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 3) {
            cout << "Invalid" << endl;
            return;
        }

        string& selectedISBN = getCurrentSelectedISBN();
        if (selectedISBN.empty()) {
            cout << "Invalid" << endl;
            return;
        }

        string quantityStr = tokens[1];
        string costStr = tokens[2];

        if (!isValidQuantity(quantityStr) || !isValidPrice(costStr)) {
            cout << "Invalid" << endl;
            return;
        }

        long long quantity = parseQuantity(quantityStr);
        double cost = parsePrice(costStr);

        if (quantity <= 0 || cost <= 0) {
            cout << "Invalid" << endl;
            return;
        }

        Book book;
        if (!bookMgr.findBook(selectedISBN, book)) {
            cout << "Invalid" << endl;
            return;
        }

        book.quantity += quantity;
        bookMgr.addOrUpdateBook(book);

        transMgr.addTransaction(cost, -1);
    }

    void cmdLog(const vector<string>& tokens) {
        if (tokens.size() != 1) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 7) {
            cout << "Invalid" << endl;
            return;
        }

        vector<string> logs = logMgr.getLogs();
        for (const auto& log : logs) {
            cout << log << endl;
        }
    }

    void cmdReport(const vector<string>& tokens) {
        if (tokens.size() != 2) {
            cout << "Invalid" << endl;
            return;
        }

        if (getCurrentPrivilege() < 7) {
            cout << "Invalid" << endl;
            return;
        }

        if (tokens[1] == "finance") {
            cout << "Financial Report:" << endl;
            vector<Transaction> transactions = transMgr.getTransactions();
            for (size_t i = 0; i < transactions.size(); i++) {
                if (transactions[i].type == 1) {
                    cout << "Income: " << fixed << setprecision(2) << transactions[i].amount << endl;
                } else {
                    cout << "Expenditure: " << fixed << setprecision(2) << transactions[i].amount << endl;
                }
            }
        } else if (tokens[1] == "employee") {
            cout << "Employee Report:" << endl;
            vector<string> logs = logMgr.getLogs();
            for (const auto& log : logs) {
                cout << log << endl;
            }
        } else {
            cout << "Invalid" << endl;
        }
    }
};

int main() {
    BookstoreSystem system;
    string line;

    while (getline(cin, line)) {
        system.processCommand(line);
    }

    return 0;
}
