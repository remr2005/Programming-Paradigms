#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <vector>
#include <string>
#include <Eigen/Dense>

// Структуры данных
struct ActivationFunction {
    bool supports_hadamard_derivative;
    int type;  // 0 = Sigmoid, 1 = ReLU, 2 = Softmax
};

struct Layer {
    Eigen::MatrixXd weights;
    Eigen::VectorXd biases;
    Eigen::VectorXd last_z;
    Eigen::VectorXd last_input;
    int input_size;
    int output_size;
    ActivationFunction* activation;
};

struct NeuralNetwork {
    std::vector<Layer*> layers;
    int loss_type;  // 0 = MSE
};

struct Dataset {
    std::vector<Eigen::VectorXd> digits;
    std::vector<Eigen::VectorXd> targets;
};

// Процедуры для функций активации
void sigmoid_activate(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    *result = (1.0 + (-x).array().exp()).inverse();
}

void sigmoid_derivative(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    Eigen::VectorXd s;
    sigmoid_activate(x, &s);
    *result = s.array() * (1.0 - s.array());
}

void relu_activate(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    *result = x.cwiseMax(0.0);
}

void relu_derivative(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    Eigen::VectorXd activated;
    relu_activate(x, &activated);
    *result = (activated.array() > 0.0).cast<double>();
}

void softmax_activate(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    double max_val = x.maxCoeff();
    Eigen::VectorXd exp_x = (x.array() - max_val).exp();
    double sum = exp_x.sum();
    *result = exp_x / sum;
}

void softmax_derivative(const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    Eigen::VectorXd s;
    softmax_activate(x, &s);
    *result = s.array() * (1.0 - s.array());
}

void softmax_jacobian(const Eigen::VectorXd& x, Eigen::MatrixXd* result) {
    Eigen::VectorXd s;
    softmax_activate(x, &s);
    Eigen::MatrixXd diag_s = Eigen::MatrixXd(s.asDiagonal());
    *result = diag_s - s * s.transpose();
}

void activation_activate(ActivationFunction* act, const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    if (act->type == 0) {
        sigmoid_activate(x, result);
    } else if (act->type == 1) {
        relu_activate(x, result);
    } else if (act->type == 2) {
        softmax_activate(x, result);
    }
}

void activation_derivative(ActivationFunction* act, const Eigen::VectorXd& x, Eigen::VectorXd* result) {
    if (act->type == 0) {
        sigmoid_derivative(x, result);
    } else if (act->type == 1) {
        relu_derivative(x, result);
    } else if (act->type == 2) {
        softmax_derivative(x, result);
    }
}

void activation_jacobian(ActivationFunction* act, const Eigen::VectorXd& x, Eigen::MatrixXd* result) {
    if (act->type == 2) {
        softmax_jacobian(x, result);
    } else {
        Eigen::VectorXd diag;
        activation_derivative(act, x, &diag);
        *result = diag.asDiagonal();
    }
}

// Процедуры для функции потерь
void mse_loss(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target, double* loss_val) {
    Eigen::VectorXd diff = predicted - target;
    *loss_val = diff.squaredNorm() / predicted.size();
}

void mse_derivative(const Eigen::VectorXd& predicted, const Eigen::VectorXd& target, Eigen::VectorXd* gradient) {
    *gradient = 2.0 * (predicted - target) / predicted.size();
}

// Процедуры для слоя
void layer_init(Layer* layer, int input_size, int output_size, ActivationFunction* activation) {
    layer->input_size = input_size;
    layer->output_size = output_size;
    layer->activation = activation;
    layer->weights = Eigen::MatrixXd(output_size, input_size);
    layer->biases = Eigen::VectorXd(output_size);
    
    // Xavier initialization
    double limit = std::sqrt(6.0 / (input_size + output_size));
    layer->weights = Eigen::MatrixXd::Random(output_size, input_size) * limit;
    layer->biases.setZero();
}

void layer_forward(Layer* layer, const Eigen::VectorXd& input, Eigen::VectorXd* output) {
    layer->last_input = input;
    layer->last_z = layer->weights * input + layer->biases;
    activation_activate(layer->activation, layer->last_z, output);
}

void layer_backward(Layer* layer, const Eigen::VectorXd& gradient, double learning_rate, Eigen::VectorXd* prev_gradient) {
    Eigen::VectorXd delta;
    
    if (layer->activation->supports_hadamard_derivative) {
        Eigen::VectorXd activation_grad;
        activation_derivative(layer->activation, layer->last_z, &activation_grad);
        delta = gradient.array() * activation_grad.array();
    } else {
        Eigen::MatrixXd J;
        activation_jacobian(layer->activation, layer->last_z, &J);
        delta = J * gradient;
    }
    
    Eigen::MatrixXd old_weights = layer->weights;
    
    layer->weights -= learning_rate * delta * layer->last_input.transpose();
    layer->biases -= learning_rate * delta;
    
    *prev_gradient = old_weights.transpose() * delta;
}

// Процедуры для нейронной сети
void network_init(NeuralNetwork* nn, int loss_type) {
    nn->loss_type = loss_type;
    nn->layers.clear();
}

void network_add_layer(NeuralNetwork* nn, Layer* layer) {
    nn->layers.push_back(layer);
}

void network_forward(NeuralNetwork* nn, const Eigen::VectorXd& input, Eigen::VectorXd* output) {
    Eigen::VectorXd x = input;
    for (size_t i = 0; i < nn->layers.size(); ++i) {
        Eigen::VectorXd temp;
        layer_forward(nn->layers[i], x, &temp);
        x = temp;
    }
    *output = x;
}

void network_train(NeuralNetwork* nn, const Eigen::VectorXd& input, const Eigen::VectorXd& target, double learning_rate, double* loss_val) {
    Eigen::VectorXd output;
    network_forward(nn, input, &output);
    
    if (nn->loss_type == 0) {
        mse_loss(output, target, loss_val);
    }
    
    Eigen::VectorXd gradient;
    if (nn->loss_type == 0) {
        mse_derivative(output, target, &gradient);
    }
    
    for (int i = nn->layers.size() - 1; i >= 0; --i) {
        Eigen::VectorXd prev_grad;
        layer_backward(nn->layers[i], gradient, learning_rate, &prev_grad);
        gradient = prev_grad;
    }
}

void network_predict(NeuralNetwork* nn, const Eigen::VectorXd& input, Eigen::VectorXd* output) {
    network_forward(nn, input, output);
}

// Процедуры для датасета
void display_digit(const Eigen::VectorXd& digit, int width, int height) {
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

void create_digit_from_pattern(const std::vector<std::string>& pattern, int size, Eigen::VectorXd* digit) {
    *digit = Eigen::VectorXd(size * size);
    digit->setZero();
    
    int offset_y = (size - pattern.size()) / 2;
    int offset_x = (size - pattern[0].length()) / 2;
    
    for (size_t i = 0; i < pattern.size(); ++i) {
        for (size_t j = 0; j < pattern[i].length(); ++j) {
            int y = offset_y + i;
            int x = offset_x + j;
            if (y >= 0 && y < size && x >= 0 && x < size) {
                if (pattern[i][j] == '1' || pattern[i][j] == '#') {
                    (*digit)(y * size + x) = 1.0;
                }
            }
        }
    }
}

void create_dataset(Dataset* dataset) {
    const int DIGIT_SIZE = 20;
    
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
    Eigen::VectorXd digit0;
    create_digit_from_pattern(pattern0, DIGIT_SIZE, &digit0);
    Eigen::VectorXd target0(10);
    target0 << 1, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    dataset->digits.push_back(digit0);
    dataset->targets.push_back(target0);
    
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
    Eigen::VectorXd digit1;
    create_digit_from_pattern(pattern1, DIGIT_SIZE, &digit1);
    Eigen::VectorXd target1(10);
    target1 << 0, 1, 0, 0, 0, 0, 0, 0, 0, 0;
    dataset->digits.push_back(digit1);
    dataset->targets.push_back(target1);
    
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
    Eigen::VectorXd digit2;
    create_digit_from_pattern(pattern2, DIGIT_SIZE, &digit2);
    Eigen::VectorXd target2(10);
    target2 << 0, 0, 1, 0, 0, 0, 0, 0, 0, 0;
    dataset->digits.push_back(digit2);
    dataset->targets.push_back(target2);
    
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
    Eigen::VectorXd digit3;
    create_digit_from_pattern(pattern3, DIGIT_SIZE, &digit3);
    Eigen::VectorXd target3(10);
    target3 << 0, 0, 0, 1, 0, 0, 0, 0, 0, 0;
    dataset->digits.push_back(digit3);
    dataset->targets.push_back(target3);
    
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
    Eigen::VectorXd digit4;
    create_digit_from_pattern(pattern4, DIGIT_SIZE, &digit4);
    Eigen::VectorXd target4(10);
    target4 << 0, 0, 0, 0, 1, 0, 0, 0, 0, 0;
    dataset->digits.push_back(digit4);
    dataset->targets.push_back(target4);
    
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
    Eigen::VectorXd digit5;
    create_digit_from_pattern(pattern5, DIGIT_SIZE, &digit5);
    Eigen::VectorXd target5(10);
    target5 << 0, 0, 0, 0, 0, 1, 0, 0, 0, 0;
    dataset->digits.push_back(digit5);
    dataset->targets.push_back(target5);
    
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
    Eigen::VectorXd digit6;
    create_digit_from_pattern(pattern6, DIGIT_SIZE, &digit6);
    Eigen::VectorXd target6(10);
    target6 << 0, 0, 0, 0, 0, 0, 1, 0, 0, 0;
    dataset->digits.push_back(digit6);
    dataset->targets.push_back(target6);
    
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
    Eigen::VectorXd digit7;
    create_digit_from_pattern(pattern7, DIGIT_SIZE, &digit7);
    Eigen::VectorXd target7(10);
    target7 << 0, 0, 0, 0, 0, 0, 0, 1, 0, 0;
    dataset->digits.push_back(digit7);
    dataset->targets.push_back(target7);
    
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
    Eigen::VectorXd digit8;
    create_digit_from_pattern(pattern8, DIGIT_SIZE, &digit8);
    Eigen::VectorXd target8(10);
    target8 << 0, 0, 0, 0, 0, 0, 0, 0, 1, 0;
    dataset->digits.push_back(digit8);
    dataset->targets.push_back(target8);
    
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
    Eigen::VectorXd digit9;
    create_digit_from_pattern(pattern9, DIGIT_SIZE, &digit9);
    Eigen::VectorXd target9(10);
    target9 << 0, 0, 0, 0, 0, 0, 0, 0, 0, 1;
    dataset->digits.push_back(digit9);
    dataset->targets.push_back(target9);
}

int main() {
    Dataset dataset;
    create_dataset(&dataset);
    
    // Создаём нейронную сеть
    NeuralNetwork nn;
    network_init(&nn, 0);  // MSE loss
    
    ActivationFunction relu_act;
    relu_act.type = 1;
    relu_act.supports_hadamard_derivative = true;
    
    ActivationFunction softmax_act;
    softmax_act.type = 2;
    softmax_act.supports_hadamard_derivative = false;
    
    Layer layer1;
    layer_init(&layer1, 400, 128, &relu_act);
    network_add_layer(&nn, &layer1);
    
    Layer layer2;
    layer_init(&layer2, 128, 10, &softmax_act);
    network_add_layer(&nn, &layer2);
    
    std::cout << "=== Обучение нейронной сети ===" << std::endl;
    const int epochs = 500;
    const double learning_rate = 0.1;
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_loss = 0.0;
        for (size_t i = 0; i < dataset.digits.size(); ++i) {
            double loss_val;
            network_train(&nn, dataset.digits[i], dataset.targets[i], learning_rate, &loss_val);
            total_loss += loss_val;
        }
        if (epoch % 20 == 0 || epoch == epochs - 1) {
            std::cout << "Epoch " << epoch << ", Loss: " << total_loss / dataset.digits.size() << std::endl;
        }
    }
    
    std::cout << "\n=== Тестирование сети ===" << std::endl;
    for (size_t i = 0; i < dataset.digits.size(); ++i) {
        Eigen::VectorXd prediction;
        network_predict(&nn, dataset.digits[i], &prediction);
        
        int predicted_digit = 0;
        double max_prob = prediction(0);
        for (int j = 1; j < 10; ++j) {
            if (prediction(j) > max_prob) {
                max_prob = prediction(j);
                predicted_digit = j;
            }
        }
        
        std::cout << "Цифра " << i << ":" << std::endl;
        display_digit(dataset.digits[i], 20, 20);
        std::cout << "Предсказание: " << predicted_digit << " (вероятность: " << max_prob << ")" << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}

