#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <Eigen/Dense>

class ActivationFunction {
protected:
    bool supports_hadamard_derivative;
    
public:
    ActivationFunction(bool supports_hadamard = true) 
        : supports_hadamard_derivative(supports_hadamard) {}
    
    virtual Eigen::VectorXd activate(const Eigen::VectorXd& x) = 0;
    
    virtual Eigen::VectorXd derivative(const Eigen::VectorXd& x) = 0;
    
    virtual Eigen::MatrixXd jacobian(const Eigen::VectorXd& x) {
        Eigen::VectorXd diag = derivative(x);
        return diag.asDiagonal();
    }
    
    virtual ~ActivationFunction() = default;
    
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
    Softmax() : ActivationFunction(false) {}
    
        Eigen::VectorXd activate(const Eigen::VectorXd& x) override {
        double max_val = x.maxCoeff();
        Eigen::VectorXd exp_x = (x.array() - max_val).exp();
        double sum = exp_x.sum();
        return exp_x / sum;
    }
    
    Eigen::VectorXd derivative(const Eigen::VectorXd& x) override {
        Eigen::VectorXd s = activate(x);
        return s.array() * (1.0 - s.array());
    }
    
    Eigen::MatrixXd jacobian(const Eigen::VectorXd& x) override {
        Eigen::VectorXd s = activate(x);
        Eigen::MatrixXd diag_s = s.asDiagonal();
        return diag_s - s * s.transpose();
    }
};

class LossFunction {
public:
    virtual double loss(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target) = 0;

    virtual Eigen::VectorXd derivative(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target) = 0;
    
    virtual ~LossFunction() = default;
};

// Mean Squared Error (MSE)
class MSE : public LossFunction {
public:
    double loss(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target) override {
        Eigen::VectorXd diff = predicted - target;
        return diff.squaredNorm() / predicted.size();
    }
    
    Eigen::VectorXd derivative(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target) override {
        return 2.0 * (predicted - target) / predicted.size();
    }
};

class Layer {
protected:
    Eigen::MatrixXd weights;
    Eigen::VectorXd biases;
    Eigen::VectorXd last_z;  // Сохраняем z для backward
    Eigen::VectorXd last_input;  // Сохраняем input для backward
    
public:
    int input_size;
    int output_size;
    ActivationFunction& activation;
    
    Layer(int input_size, int output_size, ActivationFunction& activation)
        : input_size(input_size), output_size(output_size), activation(activation),
          weights(output_size, input_size), biases(output_size) {
        // Xavier initialization
        double limit = std::sqrt(6.0 / (input_size + output_size));
        weights = Eigen::MatrixXd::Random(output_size, input_size) * limit;        
        biases.setZero();
    }
    
    Eigen::VectorXd forward(const Eigen::VectorXd& input) {
        last_input = input; 
        last_z = weights * input + biases; 
        return activation.activate(last_z);
    }
    
    Eigen::VectorXd backward(const Eigen::VectorXd& gradient, double learning_rate = 0.01) {
        Eigen::VectorXd delta;
        
        if (activation.getSupportsHadamardDerivative()) {
            // Для hadamard-совместимых функций используем derivative() и hadamard
            Eigen::VectorXd activation_grad = activation.derivative(last_z);
            delta = gradient.array() * activation_grad.array();  // Hadamard product
        } else {
            // Для softmax используем jacobian() и матричное умножение
            Eigen::MatrixXd J = activation.jacobian(last_z);
            delta = J * gradient;  // Матричное умножение
        }
        
        // Сохраняем старые веса для вычисления градиента предыдущего слоя
        Eigen::MatrixXd old_weights = weights;
        
        // Обновляем веса и смещения
        weights -= learning_rate * delta * last_input.transpose();
        biases -= learning_rate * delta;
        
        // Возвращаем градиент для предыдущего слоя (используем старые веса)
        return old_weights.transpose() * delta; 
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