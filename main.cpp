#include <chrono>
#include <format>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

enum Type { INCOME, EXPENSE };

// хранит информацию об одном платеже или пополнении
struct Transaction {
    int id; // уникальный номер транзакции
    double
        amount; // сумма (положительная для доходов, отрицаткльная для расходов)
    std::string
        category;     // категория (например, "Еда", "Транспорт", "Стипендия")
    std::string date; // дата в формате "YYYY-MM-DD"
    Type type;        // тип транзакции (доход/расход)
    std::string comment; // краткое описание (опционально)
};

bool saveToFile(const std::string& filename,
                const std::vector<Transaction>& transactions) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    file << "ID;Сумма;Категория;Дата;Тип транзакции (0 = Доход, 1 = "
            "Расход);Комметарий\n";
    for (const auto& transaction : transactions) {
        file << transaction.id << ";" << transaction.amount << ";"
             << transaction.category << ";" << transaction.date << ";"
             << transaction.type << ";" << transaction.comment << "\n";
    }

    // проверяем не возникло ли ошибок при записи
    if (!file) {
        return false;
    }

    file.close();
    return true; // всё прошло успешно
}

std::vector<Transaction> loadFromFile(const std::string& filename) {
    std::vector<Transaction> tranactions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        // если файл не открылся, возвращаем пустой вектор
        return tranactions;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string id_str, amount_str, category, date, type_str, comment;

        if (std::getline(ss, id_str, ';') &&
            std::getline(ss, amount_str, ';') &&
            std::getline(ss, category, ';') && std::getline(ss, date, ';') &&
            std::getline(ss, type_str, ';')) {

            std::getline(ss, comment);

            Transaction t;
            t.id = std::stoi(id_str);
            t.amount = std::stod(amount_str);
            t.category = category;
            t.date = date;

            int type_int = std::stoi(type_str);
            t.type = static_cast<Type>(type_int);

            t.comment = comment;

            tranactions.push_back(t);
        }
    }
    file.close();
    return tranactions;
}

void handleAdd(int argc, char* argv[], std::vector<Transaction>& list,
               const std::string& filename) {}
void handleList(const std::vector<Transaction>& transactions);
void handleBalance(const std::vector<Transaction>& transactions);
void handleStats(const std::vector<Transaction>& transactions);
void printHelp();

int main(int argc, char* argv[]) {
    // auto now = std::chrono::system_clock::now();
    // auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    // auto local_date =
    //     std::chrono::floor<std::chrono::days>(zoned.get_local_time());
    // std::chrono::year_month_day ymd{local_date};
    if (argc < 2) {
        printHelp();
    }

    std::string filename{"data.csv"};
    std::vector<Transaction> transactions{loadFromFile(filename)};

    std::string command{argv[1]};

    if (command == "add") {
        handleAdd(argc, argv, transactions, filename);
    } else if (command == "list") {
        handleList(transactions);
    } else if (command == "balance") {
        handleBalance(transactions);
    } else if (command == "stats") {
        handleStats(transactions);
    } else {
        std::println("Неизвестная команда. Введите 'help' для справки.");
    }

    return 0;
};
