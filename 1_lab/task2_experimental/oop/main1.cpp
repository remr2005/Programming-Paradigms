#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <vector>
#include <string>
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

class NeuralNetwork {
private:
    std::vector<Layer*> layers;
    LossFunction* loss_function;
    
public:
    NeuralNetwork(LossFunction* loss) : loss_function(loss) {}
    
    ~NeuralNetwork() {
        for (auto* layer : layers) {
            delete layer;
        }
    }
    
    void addLayer(Layer* layer) {
        layers.push_back(layer);
    }
    
    Eigen::VectorXd forward(const Eigen::VectorXd& input) {
        Eigen::VectorXd x = input;
        for (auto* layer : layers) {
            x = layer->forward(x);
        }
        return x;
    }
    
    double train(const Eigen::VectorXd& input, const Eigen::VectorXd& target, double learning_rate = 0.01) {
        // Forward pass
        Eigen::VectorXd output = forward(input);
        
        // Вычисляем loss
        double loss_val = loss_function->loss(output, target);
        
        // Backward pass
        Eigen::VectorXd gradient = loss_function->derivative(output, target);
        for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
            gradient = (*it)->backward(gradient, learning_rate);
        }
        
        return loss_val;
    }
    
    Eigen::VectorXd predict(const Eigen::VectorXd& input) {
        return forward(input);
    }
};

class DigitDataset {
private:
    static const int DIGIT_SIZE = 20;
    
    // Вспомогательная функция для создания цифры из паттерна
    static Eigen::VectorXd createDigitFromPattern(const std::vector<std::string>& pattern, int size = DIGIT_SIZE) {
        Eigen::VectorXd digit(size * size);
        digit.setZero();
        
        int offset_y = (size - pattern.size()) / 2;
        int offset_x = (size - pattern[0].length()) / 2;
        
        for (size_t i = 0; i < pattern.size(); ++i) {
            for (size_t j = 0; j < pattern[i].length(); ++j) {
                int y = offset_y + i;
                int x = offset_x + j;
                if (y >= 0 && y < size && x >= 0 && x < size) {
                    if (pattern[i][j] == '1' || pattern[i][j] == '#') {
                        digit(y * size + x) = 1.0;
                    }
                }
            }
        }
        return digit;
    }
    
public:
    // Функция для отображения цифры из вектора (20x20 = 400 пикселей)
    static void displayDigit(const Eigen::VectorXd& digit, int width = DIGIT_SIZE, int height = DIGIT_SIZE) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int idx = i * width + j;
                if (idx < digit.size() && digit(idx) > 0.5) {
                    std::cout << "██";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Создание датасета с цифрами (20x20 = 400 пикселей)
    static std::vector<std::pair<Eigen::VectorXd, Eigen::VectorXd>> createDataset() {
        std::vector<std::pair<Eigen::VectorXd, Eigen::VectorXd>> dataset;
        
        // Цифра 0
        std::vector<std::string> pattern0 = {
            " 1111111111 ",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit0 = createDigitFromPattern(pattern0);
        Eigen::VectorXd target0(10);
        target0 << 1, 0, 0, 0, 0, 0, 0, 0, 0, 0;
        dataset.push_back({digit0, target0});
        
        // Цифра 1
        std::vector<std::string> pattern1 = {
            "     11     ",
            "    111     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "     11     ",
            "   111111   "
        };
        Eigen::VectorXd digit1 = createDigitFromPattern(pattern1);
        Eigen::VectorXd target1(10);
        target1 << 0, 1, 0, 0, 0, 0, 0, 0, 0, 0;
        dataset.push_back({digit1, target1});
        
        // Цифра 2
        std::vector<std::string> pattern2 = {
            " 1111111111 ",
            "11        11",
            "         11 ",
            "        11  ",
            "       11   ",
            "      11    ",
            "     11     ",
            "    11      ",
            "   11       ",
            "  11        ",
            " 11111111111"
        };
        Eigen::VectorXd digit2 = createDigitFromPattern(pattern2);
        Eigen::VectorXd target2(10);
        target2 << 0, 0, 1, 0, 0, 0, 0, 0, 0, 0;
        dataset.push_back({digit2, target2});
        
        // Цифра 3
        std::vector<std::string> pattern3 = {
            " 1111111111 ",
            "11        11",
            "         11 ",
            "         11 ",
            " 1111111111 ",
            "         11 ",
            "         11 ",
            "         11 ",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit3 = createDigitFromPattern(pattern3);
        Eigen::VectorXd target3(10);
        target3 << 0, 0, 0, 1, 0, 0, 0, 0, 0, 0;
        dataset.push_back({digit3, target3});
        
        // Цифра 4
        std::vector<std::string> pattern4 = {
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            "111111111111",
            "         11 ",
            "         11 ",
            "         11 ",
            "         11 ",
            "         11 "
        };
        Eigen::VectorXd digit4 = createDigitFromPattern(pattern4);
        Eigen::VectorXd target4(10);
        target4 << 0, 0, 0, 0, 1, 0, 0, 0, 0, 0;
        dataset.push_back({digit4, target4});
        
        // Цифра 5
        std::vector<std::string> pattern5 = {
            "111111111111",
            "11          ",
            "11          ",
            "11          ",
            "1111111111  ",
            "         11 ",
            "         11 ",
            "         11 ",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit5 = createDigitFromPattern(pattern5);
        Eigen::VectorXd target5(10);
        target5 << 0, 0, 0, 0, 0, 1, 0, 0, 0, 0;
        dataset.push_back({digit5, target5});
        
        // Цифра 6
        std::vector<std::string> pattern6 = {
            " 1111111111 ",
            "11        11",
            "11          ",
            "11          ",
            "1111111111  ",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit6 = createDigitFromPattern(pattern6);
        Eigen::VectorXd target6(10);
        target6 << 0, 0, 0, 0, 0, 0, 1, 0, 0, 0;
        dataset.push_back({digit6, target6});
        
        // Цифра 7
        std::vector<std::string> pattern7 = {
            "111111111111",
            "         11 ",
            "        11  ",
            "       11   ",
            "      11    ",
            "     11     ",
            "    11      ",
            "   11       ",
            "  11        ",
            " 11         "
        };
        Eigen::VectorXd digit7 = createDigitFromPattern(pattern7);
        Eigen::VectorXd target7(10);
        target7 << 0, 0, 0, 0, 0, 0, 0, 1, 0, 0;
        dataset.push_back({digit7, target7});
        
        // Цифра 8
        std::vector<std::string> pattern8 = {
            " 1111111111 ",
            "11        11",
            "11        11",
            "11        11",
            " 1111111111 ",
            "11        11",
            "11        11",
            "11        11",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit8 = createDigitFromPattern(pattern8);
        Eigen::VectorXd target8(10);
        target8 << 0, 0, 0, 0, 0, 0, 0, 0, 1, 0;
        dataset.push_back({digit8, target8});
        
        // Цифра 9
        std::vector<std::string> pattern9 = {
            " 1111111111 ",
            "11        11",
            "11        11",
            "11        11",
            " 11111111111",
            "         11 ",
            "         11 ",
            "         11 ",
            "11        11",
            " 1111111111 "
        };
        Eigen::VectorXd digit9 = createDigitFromPattern(pattern9);
        Eigen::VectorXd target9(10);
        target9 << 0, 0, 0, 0, 0, 0, 0, 0, 0, 1;
        dataset.push_back({digit9, target9});
    
        return dataset;
    }
};

int main() {
    // Создаём датасет с цифрами
    auto dataset = DigitDataset::createDataset();
    
    std::cout << "=== Отображение цифр из датасета ===" << std::endl;
    for (size_t i = 0; i < dataset.size(); ++i) {
        std::cout << "Цифра " << i << ":" << std::endl;
        DigitDataset::displayDigit(dataset[i].first);
    }
    
    // Создаём нейронную сеть для распознавания цифр
    // Архитектура: 400 (вход 20x20) -> 128 -> 10 (выход)
    Sigmoid sigmoid1;
    Softmax softmax_output;
    
    MSE mse_loss;
    NeuralNetwork nn(&mse_loss);
    
    nn.addLayer(new Layer(400, 128, sigmoid1));
    nn.addLayer(new Layer(128, 10, softmax_output));
    
    std::cout << "=== Обучение нейронной сети ===" << std::endl;
    const int epochs = 500;
    const double learning_rate = 0.1;
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_loss = 0.0;
        for (const auto& [digit, target] : dataset) {
            total_loss += nn.train(digit, target, learning_rate);
        }
        if (epoch % 20 == 0 || epoch == epochs - 1) {
            std::cout << "Epoch " << epoch << ", Loss: " << total_loss / dataset.size() << std::endl;
        }
    }
    
    std::cout << "\n=== Тестирование сети ===" << std::endl;
    for (size_t i = 0; i < dataset.size(); ++i) {
        Eigen::VectorXd prediction = nn.predict(dataset[i].first);
        int predicted_digit = 0;
        double max_prob = prediction(0);
        for (int j = 1; j < 10; ++j) {
            if (prediction(j) > max_prob) {
                max_prob = prediction(j);
                predicted_digit = j;
            }
        }
        
        std::cout << "Цифра " << i << ":" << std::endl;
        DigitDataset::displayDigit(dataset[i].first);
        std::cout << "Предсказание: " << predicted_digit << " (вероятность: " << max_prob << ")" << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}