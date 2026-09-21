#include <chrono>
#include <format>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
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

class FileManager {
public:
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

    std::vector<Transaction> loadFromFile(const std::string& filaname) {}
};

int main(int argc, char* argv[]) {
    auto now = std::chrono::system_clock::now();
    auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    auto local_date =
        std::chrono::floor<std::chrono::days>(zoned.get_local_time());
    std::chrono::year_month_day ymd{local_date};
    Transaction transaction1{1,
                             1000,
                             "Food",
                             std::format("{:%F}", ymd),
                             Type::INCOME,
                             "My food at lunch"};
    std::vector<Transaction> transactions;
    transactions.emplace_back(transaction1);
    FileManager fileMgr;
    fileMgr.saveToFile("tranactions.csv", transactions);
    return 0;
};
