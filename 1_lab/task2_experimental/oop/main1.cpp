#include <iostream>
#include <cmath>
#include <algorithm>
#include <Eigen/Dense>

class ActivationFunction {
protected:
    bool supports_hadamard_derivative;  // Поле вместо функции
    
public:
    // Конструктор для инициализации поля
    ActivationFunction(bool supports_hadamard = true) 
        : supports_hadamard_derivative(supports_hadamard) {}
    
    // Активация вектора-столбца
    virtual Eigen::VectorXd activate(const Eigen::VectorXd& x) = 0;
    
    virtual Eigen::VectorXd derivative(const Eigen::VectorXd& x) = 0;
    
    virtual ~ActivationFunction() = default;
    
    // Геттер для поля
    bool getSupportsHadamardDerivative() const {
        return supports_hadamard_derivative;
    }
};

class Sigmoid : public ActivationFunction {
public:
    Sigmoid() : ActivationFunction(true) {}  // Инициализируем поле
    
    Eigen::VectorXd activate(const Eigen::VectorXd& x) override {
        return (1.0 + (-x).array().exp()).inverse();
    }
    
    Eigen::VectorXd derivative(const Eigen::VectorXd& x) override {
        Eigen::VectorXd s = activate(x);
        return s.array() * (1.0 - s.array());
    }
};

class ReLU : public ActivationFunction {
public:
    ReLU() : ActivationFunction(true) {}  // Инициализируем поле
    
    Eigen::VectorXd activate(const Eigen::VectorXd& x) override {
        return x.cwiseMax(0.0);
    }
    
    Eigen::VectorXd derivative(const Eigen::VectorXd& x) override {
        Eigen::VectorXd activated = activate(x);
        return (activated.array() > 0.0).cast<double>();
    }
};

class Softmax : public ActivationFunction {
public:
    Softmax() : ActivationFunction(false) {}  // Инициализируем поле как false
    
    Eigen::VectorXd activate(const Eigen::VectorXd& x) override {
        // Вычитаем максимум для численной стабильности
        double max_val = x.maxCoeff();
        Eigen::VectorXd exp_x = (x.array() - max_val).exp();
        double sum = exp_x.sum();
        return exp_x / sum;
    }
    
    Eigen::VectorXd derivative(const Eigen::VectorXd& x) override {
        Eigen::VectorXd s = activate(x);
        return s.array() * (1.0 - s.array());
    }
};

int main() {
    // Пример использования
    Eigen::VectorXd input(3);
    input << 1.0, 2.0, 3.0;
    
    Sigmoid sigmoid;
    Eigen::VectorXd output = sigmoid.activate(input);
    std::cout << "Sigmoid output:\n" << output << std::endl;
    
    ReLU relu;
    output = relu.activate(input);
    std::cout << "ReLU output:\n" << output << std::endl;
    
    Softmax softmax;
    output = softmax.activate(input);
    std::cout << "Softmax output:\n" << output << std::endl;
    std::cout << "Softmax sum: " << output.sum() << std::endl; // Должно быть ~1.0
    
    // Пример умножения вектора на матрицу
    Eigen::MatrixXd weights(2, 3);  // Матрица весов 2x3
    weights << 1.0, 0.5, 0.3,
               0.2, 0.8, 0.6;
    
    Eigen::VectorXd result = weights * output;  // Умножение матрицы на вектор
    std::cout << "\nMatrix * Vector result:\n" << result << std::endl;
    
    return 0;
}