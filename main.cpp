#include <print>
#include <string>
#include <vector>

struct Transaction {
    int id; // уникальный номер транзакции
    double
        amount; // сумма (положительная для доходов, отрицаткльная для расходов)
    std::string
        category;     // категория (например, "Еда", "Транспорт", "Стипендия")
    std::string date; // дата в формате "YYYY-MM-DD"
    enum Type { INCOME, EXPENSE }; // тип транзакции (доход/расход)
    std::string comment;           // краткое описание (опционально)
};

int main(int argc, char* argv[]) { std::println("Hello"); }
