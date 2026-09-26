#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <print>
#include <sstream>
#include <string>
#include <vector>

// хранит информацию об одном платеже или пополнении
struct Transaction {
    long long id; // уникальный номер транзакции
    double
        amount; // сумма (положительная для доходов, отрицаткльная для расходов)
    std::string
        category;     // категория (например, "Еда", "Транспорт", "Стипендия")
    std::string date; // дата в формате "YYYY-MM-DD"
    std::string type; // тип транзакции (доход/расход)
    std::string comment; // краткое описание (опционально)
};

long long generateUniqueId();

std::string truncateUTF8(const std::string& str, std::size_t max_chars);

bool saveToFile(const std::string& filename,
                const std::vector<Transaction>& transactions);

std::vector<Transaction> loadFromFile(const std::string& filename);

void handleAdd(int argc, char* argv[], std::vector<Transaction>& transactions,
               const std::string& filename);

void handleList(const std::vector<Transaction>& transactions);
void handleBalance(const std::vector<Transaction>& transactions);
void handleStats(const std::vector<Transaction>& transactions);
void printHelp();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string filename{"data.csv"};
    std::vector<Transaction> transactions = loadFromFile(filename);
    std::string command{argv[1]};

    if (command == "add") {
        handleAdd(argc, argv, transactions, filename);
    } else if (command == "list") {
        handleList(transactions);
    } else if (command == "balance") {
        handleBalance(transactions);
    } else if (command == "stats") {
        handleStats(transactions);
    } else if (command == "help") {
        printHelp();
    } else {
        std::println("Неизвестная команда. Введите 'help' для справки.");
    }

    return 0;
}

long long generateUniqueId() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string truncateUTF8(const std::string& str, std::size_t max_chars) {
    std::size_t char_count = 0;
    std::size_t byte_index = 0;

    while (byte_index < str.size() && char_count < max_chars) {
        unsigned char c = static_cast<unsigned char>(str[byte_index]);

        // Определяем размер UTF-8 символа по первому байту
        if ((c & 0x80) == 0) {
            byte_index += 1; // 1-байтовый ASCII (A-Z, 0-9, пробел)
        } else if ((c & 0xE0) == 0xC0) {
            byte_index += 2; // 2-байтовый символ (Кириллица)
        } else if ((c & 0xF0) == 0xE0) {
            byte_index += 3; // 3-байтовый символ
        } else if ((c & 0xF8) == 0xF0) {
            byte_index += 4; // 4-байтовый символ (Emoji и др.)
        } else {
            byte_index += 1; // защита от битых байтов
        }

        char_count++;
    }

    if (byte_index < str.size()) {
        return str.substr(0, byte_index) + "...";
    }

    return str;
}

bool saveToFile(const std::string& filename,
                const std::vector<Transaction>& transactions) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    file << "ID;Тип транзакции;Сумма;Категория;Дата;Комментарий\n";
    for (const auto& transaction : transactions) {
        file << transaction.id << ";" << transaction.type << ";"
             << transaction.amount << ";" << transaction.category << ";"
             << transaction.date << ";" << transaction.comment << "\n";
    }

    // проверяем не возникло ли ошибок при записи
    if (!file) {
        return false;
    }

    file.close();
    return true; // всё прошло успешно
}

std::vector<Transaction> loadFromFile(const std::string& filename) {
    std::vector<Transaction> transactions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        // если файл не открылся, возвращаем пустой вектор
        return transactions;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string id_str, type, amount_str, category, date, comment;

        if (std::getline(ss, id_str, ';') && std::getline(ss, type, ';') &&
            std::getline(ss, amount_str, ';') &&
            std::getline(ss, category, ';') && std::getline(ss, date, ';')) {

            std::getline(ss, comment);

            try {
                Transaction t;
                t.id = std::stoll(id_str);
                t.type = type;
                t.amount = std::stod(amount_str);
                t.category = category;
                t.date = date;
                t.comment = comment;

                transactions.push_back(t);
            } catch (const std::exception& e) {
                // игнорируем поврежденные строки
                continue;
            }
        }
    }
    file.close();
    return transactions;
}

void handleAdd(int argc, char* argv[], std::vector<Transaction>& transactions,
               const std::string& filename) {

    auto now = std::chrono::system_clock::now();
    auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    auto local_date =
        std::chrono::floor<std::chrono::days>(zoned.get_local_time());
    std::chrono::year_month_day ymd{local_date};

    long long id{generateUniqueId()};
    double amount{0.0};
    std::string category, date, type, comment;
    date = std::format("{:%Y-%m-%d}", ymd);

    // парсинг аргументов
    for (int i{2}; i < argc; ++i) {
        std::string arg{argv[i]};

        if (i + 1 < argc) {
            if (arg == "--amount") {
                amount = std::stod(argv[++i]);
            } else if (arg == "--category") {
                category = argv[++i];
            } else if (arg == "--type") {
                type = argv[++i];
            } else if (arg == "--comment") {
                comment = argv[++i];
            }
        } else {
            std::cerr << "Ошибка: Для флага " << arg
                      << " не указано значение.\n";
        }
    }

    const std::size_t MAX_CHARS{40};

    std::string truncated = truncateUTF8(comment, MAX_CHARS - 3);
    if (truncated != comment) {
        comment = truncated;
        std::println(
            "Предупреждение: Комментарий слишком длинный и был обрезан.");
    }
    Transaction transaction{id, amount, category, date, type, comment};
    transactions.push_back(transaction);
    saveToFile(filename, transactions);
}

void handleList(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        std::println("Список транзакций пуст");
        return;
    }
    std::println(
        "\n-------------------------------------------------------------"
        "------------------------------------------------------------------");
    std::println("| {:<16} | {:<8} | {:<16} | {:<16} | {:<12} | {:<40} |", "ID",
                 "ТИП", "СУММА", "КАТЕГОРИЯ", "ДАТА", "КОММЕНТАРИЙ");
    std::println(
        "-------------------------------------------------------------"
        "------------------------------------------------------------------");
    for (const auto& transaction : transactions) {
        std::println(
            "| {:<16} | {:<8} | {:<16.2f} | {:<16} | {:<12} | {:<40} |",
            transaction.id, transaction.type == "+" ? "Доход" : "Расход",
            transaction.amount, transaction.category, transaction.date,
            transaction.comment);
    }
    std::println(
        "-------------------------------------------------------------"
        "------------------------------------------------------------------");
}
void handleBalance(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        std::println("Данные отсутствуют.");
        return;
    }

    std::println("\n=== ФИНАНСОВЫЙ БАЛАНС ===");
    double all_incomes{0.0};
    double all_expenses{0.0};
    for (const auto& transaction : transactions) {
        if (transaction.type == "+") {
            all_incomes += transaction.amount;
        } else {
            all_expenses += transaction.amount;
        }
    }
    std::println("Всего доходов: {:<16.2f}", all_incomes);
    std::println("Всего расходов: {:<16.2f}", all_expenses);
    std::println("--------------------------");
    double balance{all_incomes - all_expenses};
    std::println("Текущий баланс: {:<16.2f} ({})", balance,
                 balance > 0 ? "Профицит" : "Перерасход");
}
void handleStats(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        std::println("Нет данных для расчета статистики");
        return;
    }

    std::println("\n=== СТАТИСТИКА РАСХОДОВ ===");
    std::println("---------------------------------------");
    std::println("| {:<16} | {:<16} |", "КАТЕГОРИЯ", "СУММА");
    std::println("---------------------------------------");

    std::map<std::string, double> incomes{};
    std::map<std::string, double> expenses{};
    double all_incomes{0.0};
    double all_expenses{0.0};
    for (const auto& transaction : transactions) {
        if (transaction.type == "+") {
            incomes[transaction.category] += transaction.amount;
            all_incomes += transaction.amount;
        } else {
            expenses[transaction.category] += transaction.amount;
            all_expenses += transaction.amount;
        }
    }

    for (const auto& [category, amount] : expenses) {
        std::println("| {:<16} | {:<16.2f} |", category, amount);
    }
    std::println("---------------------------------------");
    std::println("Всего расходов: {:.2f}", all_expenses);

    std::println("\n=== СТАТИСТИКА ДОХОДОВ ===");
    std::println("---------------------------------------");
    std::println("| {:<16} | {:<16} |", "КАТЕГОРИЯ", "СУММА");
    std::println("---------------------------------------");
    for (const auto& [category, amount] : incomes) {
        std::println("| {:<16} | {:<16.2f} |", category, amount);
    }
    std::println("---------------------------------------");
    std::println("Всего доходов: {:.2f}", all_incomes);
}
void printHelp() {
    std::println("\nИспользование: ./CLI_finance_tracker <команда> [опции]\n");
    std::println("Команды:");
    std::println("  add     --type <\"+\"|\"-\"> --amount <число> --category "
                 "<название> [--comment <текст>]");
    std::println("  balance Показать текущий баланс");
    std::println("  list    Вывести список всех транзакций");
    std::println("  stats   Показать статистику по категориям");
    std::println("  help    Показать эту справку\n");
}
