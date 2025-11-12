#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <cmath>

// Абстрактный базовый класс для анализа
class AnalysisStrategy {
public:
    virtual ~AnalysisStrategy() = default;
    virtual void analyze() = 0;
    virtual std::string get_description() const = 0;
};

// Базовый класс для студента с расширенной функциональностью
class AdvancedStudent {
protected:
    std::string name;
    std::string surname;
    int age;
    double average_score;
    std::string gender;
    std::string performance_level;
    std::vector<double> subject_scores;
    double gpa;
    bool is_risk_student;

public:
    AdvancedStudent(const std::string& n, const std::string& s, int a, double score, 
                   const std::string& g = "", const std::vector<double>& scores = {})
        : name(n), surname(s), age(a), average_score(score), gender(g), subject_scores(scores) {
        performance_level = determine_performance_level(score);
        gpa = score;
        is_risk_student = (score < 60.0);
    }

    virtual ~AdvancedStudent() = default;

    // Геттеры
    std::string get_name() const { return name; }
    std::string get_surname() const { return surname; }
    int get_age() const { return age; }
    double get_average_score() const { return average_score; }
    std::string get_gender() const { return gender; }
    std::string get_performance_level() const { return performance_level; }
    double get_gpa() const { return gpa; }
    bool is_at_risk() const { return is_risk_student; }

    // Виртуальные методы
    virtual std::string get_detailed_info() const {
        return name + " " + surname + " (возраст: " + std::to_string(age) + 
               ", балл: " + std::to_string(average_score) + ", уровень: " + performance_level + 
               ", риск: " + (is_risk_student ? "ДА" : "НЕТ") + ")";
    }

    virtual double calculate_risk_score() const {
        double risk = 0.0;
        if (average_score < 60) risk += 50.0;
        if (age > 25 && average_score < 70) risk += 20.0;
        if (average_score < 50) risk += 30.0;
        return std::min(risk, 100.0);
    }

protected:
    std::string determine_performance_level(double score) const {
        if (score >= 90) return "Отлично";
        else if (score >= 80) return "Хорошо";
        else if (score >= 70) return "Удовлетворительно";
        else if (score >= 60) return "Зачет";
        else return "Неудовлетворительно";
    }
};

// Класс для отличника с дополнительными возможностями
class ExcellentStudentAdvanced : public AdvancedStudent {
public:
    ExcellentStudentAdvanced(const std::string& n, const std::string& s, int a, double score, 
                           const std::string& g = "", const std::vector<double>& scores = {})
        : AdvancedStudent(n, s, a, score, g, scores) {}

    std::string get_detailed_info() const override {
        return "[ОТЛИЧНИК] " + AdvancedStudent::get_detailed_info();
    }

    double calculate_risk_score() const override {
        return 0.0;  // Отличники не в группе риска
    }
};

// Класс для проблемного студента с анализом рисков
class ProblematicStudentAdvanced : public AdvancedStudent {
public:
    ProblematicStudentAdvanced(const std::string& n, const std::string& s, int a, double score, 
                             const std::string& g = "", const std::vector<double>& scores = {})
        : AdvancedStudent(n, s, a, score, g, scores) {}

    std::string get_detailed_info() const override {
        return "[ПРОБЛЕМНЫЙ] " + AdvancedStudent::get_detailed_info();
    }

    double calculate_risk_score() const override {
        double risk = 70.0;  // Базовый риск для проблемных студентов
        if (average_score < 50) risk += 20.0;
        if (age > 25) risk += 10.0;
        return std::min(risk, 100.0);
    }
};

// Расширенный класс для работы с MongoDB
class AdvancedMongoDBHandler {
private:
    mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;
    bsoncxx::builder::basic::document filter_;

public:
    AdvancedMongoDBHandler(const std::string& uri, const std::string& db_name, const std::string& coll_name)
        : client{mongocxx::uri{uri}}, db{client[db_name]}, collection{db[coll_name]} {}

    void clear_filter() {
        filter_ = bsoncxx::builder::basic::document{};
    }

    void add_filter_condition(const std::string& field, const std::string& op, const bsoncxx::types::bson_value::value& value) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp(op, value)
            )
        ));
    }

    void add_regex_filter(const std::string& field, const std::string& pattern) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$regex", pattern)
            )
        ));
    }

    void add_range_filter(const std::string& field, double min_val, double max_val) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$gte", min_val),
                bsoncxx::builder::basic::kvp("$lte", max_val)
            )
        ));
    }

    mongocxx::collection get_collection() const { return collection; }
    bsoncxx::builder::basic::document get_filter() const { return filter_; }
};

// Класс для расширенного анализа статистики
class AdvancedStatisticsAnalyzer {
private:
    std::vector<std::unique_ptr<AdvancedStudent>> students;
    AdvancedMongoDBHandler& db_handler;

public:
    AdvancedStatisticsAnalyzer(AdvancedMongoDBHandler& handler) : db_handler(handler) {}

    void load_students() {
        students.clear();
        auto cursor = db_handler.get_collection().find(db_handler.get_filter());

        for (auto& doc : cursor) {
            std::string name = "";
            std::string surname = "";
            int age = 0;
            double score = 0.0;
            std::string gender = "";

            if (doc["Имя"]) {
                name = doc["Имя"].get_string().value.to_string();
            }
            if (doc["Фамилия"]) {
                surname = doc["Фамилия"].get_string().value.to_string();
            }
            if (doc["Возраст"]) {
                age = doc["Возраст"].get_int32().value;
            }
            if (doc["Пол"]) {
                gender = doc["Пол"].get_string().value.to_string();
            }
            if (doc["Средний_балл"]) {
                if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                    score = doc["Средний_балл"].get_double().value;
                } else {
                    score = static_cast<double>(doc["Средний_балл"].get_int32().value);
                }
            }

            // Создаем соответствующий тип студента
            if (score >= 85.0) {
                students.push_back(std::make_unique<ExcellentStudentAdvanced>(name, surname, age, score, gender));
            } else if (score < 60.0) {
                students.push_back(std::make_unique<ProblematicStudentAdvanced>(name, surname, age, score, gender));
            } else {
                students.push_back(std::make_unique<AdvancedStudent>(name, surname, age, score, gender));
            }
        }
    }

    struct AdvancedStats {
        int count;
        double total_average;
        double max_score;
        double min_score;
        double standard_deviation;
        int excellent_count;
        int problematic_count;
        int risk_students;
        double average_risk_score;
        std::map<std::string, int> performance_distribution;
        std::map<std::string, int> gender_distribution;
    };

    AdvancedStats get_advanced_statistics() const {
        AdvancedStats stats = {0, 0.0, 0.0, 100.0, 0.0, 0, 0, 0, 0.0};

        for (const auto& student : students) {
            stats.count++;
            stats.total_average += student->get_average_score();
            
            if (student->get_average_score() > stats.max_score) {
                stats.max_score = student->get_average_score();
            }
            if (student->get_average_score() < stats.min_score) {
                stats.min_score = student->get_average_score();
            }
            
            if (student->get_average_score() >= 85.0) stats.excellent_count++;
            if (student->get_average_score() < 60.0) stats.problematic_count++;
            if (student->is_at_risk()) stats.risk_students++;
            
            stats.average_risk_score += student->calculate_risk_score();
            
            stats.performance_distribution[student->get_performance_level()]++;
            stats.gender_distribution[student->get_gender()]++;
        }

        if (stats.count > 0) {
            stats.total_average /= stats.count;
            stats.average_risk_score /= stats.count;
            
            // Вычисляем стандартное отклонение
            double variance = 0.0;
            for (const auto& student : students) {
                variance += pow(student->get_average_score() - stats.total_average, 2);
            }
            stats.standard_deviation = sqrt(variance / stats.count);
        }

        return stats;
    }

    void print_advanced_statistics(const std::string& description) const {
        AdvancedStats stats = get_advanced_statistics();
        
        std::cout << description << std::endl;
        std::cout << "   Количество студентов: " << stats.count << std::endl;
        
        if (stats.count > 0) {
            std::cout << "   Средний балл: " << std::fixed << std::setprecision(2) << stats.total_average << std::endl;
            std::cout << "   Максимальный балл: " << std::fixed << std::setprecision(2) << stats.max_score << std::endl;
            std::cout << "   Минимальный балл: " << std::fixed << std::setprecision(2) << stats.min_score << std::endl;
            std::cout << "   Стандартное отклонение: " << std::fixed << std::setprecision(2) << stats.standard_deviation << std::endl;
            std::cout << "   Отличников: " << stats.excellent_count << std::endl;
            std::cout << "   Проблемных: " << stats.problematic_count << std::endl;
            std::cout << "   Студентов в группе риска: " << stats.risk_students << std::endl;
            std::cout << "   Средний риск: " << std::fixed << std::setprecision(1) << stats.average_risk_score << "%" << std::endl;
            
            std::cout << "   Распределение по уровням успеваемости:" << std::endl;
            for (const auto& level : stats.performance_distribution) {
                double percentage = (double)level.second / stats.count * 100;
                std::cout << "     " << level.first << ": " << level.second << " (" 
                          << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
            }
            
            std::cout << "   Распределение по полу:" << std::endl;
            for (const auto& gender : stats.gender_distribution) {
                if (!gender.first.empty()) {
                    double percentage = (double)gender.second / stats.count * 100;
                    std::cout << "     " << gender.first << ": " << gender.second << " (" 
                              << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
                }
            }
        } else {
            std::cout << "   Студентов не найдено." << std::endl;
        }
        std::cout << std::endl;
    }

    const std::vector<std::unique_ptr<AdvancedStudent>>& get_students() const {
        return students;
    }
};

// Конкретные стратегии анализа
class AgeGroupAnalysis : public AnalysisStrategy {
private:
    AdvancedMongoDBHandler& db_handler;
    std::vector<std::pair<int, int>> age_groups;

public:
    AgeGroupAnalysis(AdvancedMongoDBHandler& handler) 
        : db_handler(handler), age_groups({{17, 18}, {19, 20}, {21, 22}, {23, 25}, {26, 30}}) {}

    void analyze() override {
        std::cout << "=== АНАЛИЗ ПО ВОЗРАСТНЫМ ГРУППАМ ===" << std::endl;
        
        for (auto& group : age_groups) {
            db_handler.clear_filter();
            db_handler.add_range_filter("Возраст", group.first, group.second);
            
            AdvancedStatisticsAnalyzer analyzer(db_handler);
            analyzer.load_students();
            analyzer.print_advanced_statistics("Возрастная группа " + std::to_string(group.first) + "-" + std::to_string(group.second) + " лет:");
        }
    }

    std::string get_description() const override {
        return "Анализ успеваемости по возрастным группам";
    }
};

class GenderAnalysis : public AnalysisStrategy {
private:
    AdvancedMongoDBHandler& db_handler;
    std::vector<std::string> genders;

public:
    GenderAnalysis(AdvancedMongoDBHandler& handler) 
        : db_handler(handler), genders({"М", "Ж", "Мужской", "Женский", "м", "ж"}) {}

    void analyze() override {
        std::cout << "=== АНАЛИЗ ПО ПОЛУ ===" << std::endl;
        
        for (const std::string& gender : genders) {
            db_handler.clear_filter();
            db_handler.add_filter_condition("Пол", "$eq", gender);
            
            AdvancedStatisticsAnalyzer analyzer(db_handler);
            analyzer.load_students();
            if (analyzer.get_students().size() > 0) {
                analyzer.print_advanced_statistics("Пол '" + gender + "':");
            }
        }
    }

    std::string get_description() const override {
        return "Анализ успеваемости по полу";
    }
};

class RiskAnalysis : public AnalysisStrategy {
private:
    AdvancedMongoDBHandler& db_handler;

public:
    RiskAnalysis(AdvancedMongoDBHandler& handler) : db_handler(handler) {}

    void analyze() override {
        std::cout << "=== АНАЛИЗ АКАДЕМИЧЕСКИХ РИСКОВ ===" << std::endl;
        
        // Студенты с низкой успеваемостью
        db_handler.clear_filter();
        db_handler.add_filter_condition("Средний_балл", "$lt", 60.0);
        AdvancedStatisticsAnalyzer low_performance(db_handler);
        low_performance.load_students();
        low_performance.print_advanced_statistics("СТУДЕНТЫ С НИЗКОЙ УСПЕВАЕМОСТЬЮ (< 60):");
        
        // Студенты на грани отчисления
        db_handler.clear_filter();
        db_handler.add_range_filter("Средний_балл", 50.0, 60.0);
        AdvancedStatisticsAnalyzer at_risk(db_handler);
        at_risk.load_students();
        at_risk.print_advanced_statistics("СТУДЕНТЫ НА ГРАНИ ОТЧИСЛЕНИЯ (50-60):");
        
        // Анализ рисков для всех студентов
        db_handler.clear_filter();
        AdvancedStatisticsAnalyzer all_students(db_handler);
        all_students.load_students();
        
        std::cout << "ДЕТАЛЬНЫЙ АНАЛИЗ РИСКОВ:" << std::endl;
        auto students = all_students.get_students();
        int high_risk_count = 0;
        int medium_risk_count = 0;
        int low_risk_count = 0;
        
        for (const auto& student : students) {
            double risk = student->calculate_risk_score();
            if (risk >= 70) high_risk_count++;
            else if (risk >= 40) medium_risk_count++;
            else low_risk_count++;
        }
        
        std::cout << "   Высокий риск (>=70%): " << high_risk_count << " студентов" << std::endl;
        std::cout << "   Средний риск (40-69%): " << medium_risk_count << " студентов" << std::endl;
        std::cout << "   Низкий риск (<40%): " << low_risk_count << " студентов" << std::endl;
        std::cout << std::endl;
    }

    std::string get_description() const override {
        return "Анализ академических рисков";
    }
};

// Класс-контекст для выполнения анализа
class AnalysisContext {
private:
    std::vector<std::unique_ptr<AnalysisStrategy>> strategies;
    AdvancedMongoDBHandler& db_handler;

public:
    AnalysisContext(AdvancedMongoDBHandler& handler) : db_handler(handler) {
        // Добавляем стратегии анализа
        strategies.push_back(std::make_unique<AgeGroupAnalysis>(db_handler));
        strategies.push_back(std::make_unique<GenderAnalysis>(db_handler));
        strategies.push_back(std::make_unique<RiskAnalysis>(db_handler));
    }

    void execute_all_analyses() {
        std::cout << "=== РАСШИРЕННАЯ СИСТЕМА АНАЛИЗА УСПЕВАЕМОСТИ (ООП) ===" << std::endl;
        std::cout << std::endl;
        
        for (const auto& strategy : strategies) {
            std::cout << "Выполняется: " << strategy->get_description() << std::endl;
            strategy->analyze();
        }
        
        // Дополнительный комплексный анализ
        std::cout << "=== КОМПЛЕКСНЫЙ АНАЛИЗ ===" << std::endl;
        
        db_handler.clear_filter();
        AdvancedStatisticsAnalyzer overall_analyzer(db_handler);
        overall_analyzer.load_students();
        overall_analyzer.print_advanced_statistics("ОБЩАЯ СТАТИСТИКА:");
        
        std::cout << "=== АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
    }
};

int main() {
    try {
        // Создание расширенного обработчика базы данных
        AdvancedMongoDBHandler db_handler("mongodb://localhost:27017", "university", "students");
        
        // Создание контекста анализа
        AnalysisContext context(db_handler);
        
        // Выполнение всех анализов
        context.execute_all_analyses();
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
