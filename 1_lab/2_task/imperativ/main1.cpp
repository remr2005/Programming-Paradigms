#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <vector>
#include <string>
#include <Eigen/Dense>

int main() {
    // Инициализация генератора случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Создание датасета с цифрами
    const int DIGIT_SIZE = 20;
    std::vector<Eigen::VectorXd> digits;
    std::vector<Eigen::VectorXd> targets;
    
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
    Eigen::VectorXd digit0(DIGIT_SIZE * DIGIT_SIZE);
    digit0.setZero();
    int offset_y0 = (DIGIT_SIZE - pattern0.size()) / 2;
    int offset_x0 = (DIGIT_SIZE - pattern0[0].length()) / 2;
    for (size_t i = 0; i < pattern0.size(); ++i) {
        for (size_t j = 0; j < pattern0[i].length(); ++j) {
            int y = offset_y0 + i;
            int x = offset_x0 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern0[i][j] == '1' || pattern0[i][j] == '#') {
                    digit0(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target0(10);
    target0 << 1, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    digits.push_back(digit0);
    targets.push_back(target0);
    
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
    Eigen::VectorXd digit1(DIGIT_SIZE * DIGIT_SIZE);
    digit1.setZero();
    int offset_y1 = (DIGIT_SIZE - pattern1.size()) / 2;
    int offset_x1 = (DIGIT_SIZE - pattern1[0].length()) / 2;
    for (size_t i = 0; i < pattern1.size(); ++i) {
        for (size_t j = 0; j < pattern1[i].length(); ++j) {
            int y = offset_y1 + i;
            int x = offset_x1 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern1[i][j] == '1' || pattern1[i][j] == '#') {
                    digit1(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target1(10);
    target1 << 0, 1, 0, 0, 0, 0, 0, 0, 0, 0;
    digits.push_back(digit1);
    targets.push_back(target1);
    
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
    Eigen::VectorXd digit2(DIGIT_SIZE * DIGIT_SIZE);
    digit2.setZero();
    int offset_y2 = (DIGIT_SIZE - pattern2.size()) / 2;
    int offset_x2 = (DIGIT_SIZE - pattern2[0].length()) / 2;
    for (size_t i = 0; i < pattern2.size(); ++i) {
        for (size_t j = 0; j < pattern2[i].length(); ++j) {
            int y = offset_y2 + i;
            int x = offset_x2 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern2[i][j] == '1' || pattern2[i][j] == '#') {
                    digit2(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target2(10);
    target2 << 0, 0, 1, 0, 0, 0, 0, 0, 0, 0;
    digits.push_back(digit2);
    targets.push_back(target2);
    
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
    Eigen::VectorXd digit3(DIGIT_SIZE * DIGIT_SIZE);
    digit3.setZero();
    int offset_y3 = (DIGIT_SIZE - pattern3.size()) / 2;
    int offset_x3 = (DIGIT_SIZE - pattern3[0].length()) / 2;
    for (size_t i = 0; i < pattern3.size(); ++i) {
        for (size_t j = 0; j < pattern3[i].length(); ++j) {
            int y = offset_y3 + i;
            int x = offset_x3 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern3[i][j] == '1' || pattern3[i][j] == '#') {
                    digit3(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target3(10);
    target3 << 0, 0, 0, 1, 0, 0, 0, 0, 0, 0;
    digits.push_back(digit3);
    targets.push_back(target3);
    
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
    Eigen::VectorXd digit4(DIGIT_SIZE * DIGIT_SIZE);
    digit4.setZero();
    int offset_y4 = (DIGIT_SIZE - pattern4.size()) / 2;
    int offset_x4 = (DIGIT_SIZE - pattern4[0].length()) / 2;
    for (size_t i = 0; i < pattern4.size(); ++i) {
        for (size_t j = 0; j < pattern4[i].length(); ++j) {
            int y = offset_y4 + i;
            int x = offset_x4 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern4[i][j] == '1' || pattern4[i][j] == '#') {
                    digit4(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target4(10);
    target4 << 0, 0, 0, 0, 1, 0, 0, 0, 0, 0;
    digits.push_back(digit4);
    targets.push_back(target4);
    
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
    Eigen::VectorXd digit5(DIGIT_SIZE * DIGIT_SIZE);
    digit5.setZero();
    int offset_y5 = (DIGIT_SIZE - pattern5.size()) / 2;
    int offset_x5 = (DIGIT_SIZE - pattern5[0].length()) / 2;
    for (size_t i = 0; i < pattern5.size(); ++i) {
        for (size_t j = 0; j < pattern5[i].length(); ++j) {
            int y = offset_y5 + i;
            int x = offset_x5 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern5[i][j] == '1' || pattern5[i][j] == '#') {
                    digit5(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target5(10);
    target5 << 0, 0, 0, 0, 0, 1, 0, 0, 0, 0;
    digits.push_back(digit5);
    targets.push_back(target5);
    
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
    Eigen::VectorXd digit6(DIGIT_SIZE * DIGIT_SIZE);
    digit6.setZero();
    int offset_y6 = (DIGIT_SIZE - pattern6.size()) / 2;
    int offset_x6 = (DIGIT_SIZE - pattern6[0].length()) / 2;
    for (size_t i = 0; i < pattern6.size(); ++i) {
        for (size_t j = 0; j < pattern6[i].length(); ++j) {
            int y = offset_y6 + i;
            int x = offset_x6 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern6[i][j] == '1' || pattern6[i][j] == '#') {
                    digit6(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target6(10);
    target6 << 0, 0, 0, 0, 0, 0, 1, 0, 0, 0;
    digits.push_back(digit6);
    targets.push_back(target6);
    
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
    Eigen::VectorXd digit7(DIGIT_SIZE * DIGIT_SIZE);
    digit7.setZero();
    int offset_y7 = (DIGIT_SIZE - pattern7.size()) / 2;
    int offset_x7 = (DIGIT_SIZE - pattern7[0].length()) / 2;
    for (size_t i = 0; i < pattern7.size(); ++i) {
        for (size_t j = 0; j < pattern7[i].length(); ++j) {
            int y = offset_y7 + i;
            int x = offset_x7 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern7[i][j] == '1' || pattern7[i][j] == '#') {
                    digit7(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target7(10);
    target7 << 0, 0, 0, 0, 0, 0, 0, 1, 0, 0;
    digits.push_back(digit7);
    targets.push_back(target7);
    
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
    Eigen::VectorXd digit8(DIGIT_SIZE * DIGIT_SIZE);
    digit8.setZero();
    int offset_y8 = (DIGIT_SIZE - pattern8.size()) / 2;
    int offset_x8 = (DIGIT_SIZE - pattern8[0].length()) / 2;
    for (size_t i = 0; i < pattern8.size(); ++i) {
        for (size_t j = 0; j < pattern8[i].length(); ++j) {
            int y = offset_y8 + i;
            int x = offset_x8 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern8[i][j] == '1' || pattern8[i][j] == '#') {
                    digit8(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target8(10);
    target8 << 0, 0, 0, 0, 0, 0, 0, 0, 1, 0;
    digits.push_back(digit8);
    targets.push_back(target8);
    
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
    Eigen::VectorXd digit9(DIGIT_SIZE * DIGIT_SIZE);
    digit9.setZero();
    int offset_y9 = (DIGIT_SIZE - pattern9.size()) / 2;
    int offset_x9 = (DIGIT_SIZE - pattern9[0].length()) / 2;
    for (size_t i = 0; i < pattern9.size(); ++i) {
        for (size_t j = 0; j < pattern9[i].length(); ++j) {
            int y = offset_y9 + i;
            int x = offset_x9 + j;
            if (y >= 0 && y < DIGIT_SIZE && x >= 0 && x < DIGIT_SIZE) {
                if (pattern9[i][j] == '1' || pattern9[i][j] == '#') {
                    digit9(y * DIGIT_SIZE + x) = 1.0;
                }
            }
        }
    }
    Eigen::VectorXd target9(10);
    target9 << 0, 0, 0, 0, 0, 0, 0, 0, 0, 1;
    digits.push_back(digit9);
    targets.push_back(target9);
    
    // Отображение цифр
    std::cout << "=== Отображение цифр из датасета ===" << std::endl;
    for (size_t i = 0; i < digits.size(); ++i) {
        std::cout << "Цифра " << i << ":" << std::endl;
        for (int row = 0; row < DIGIT_SIZE; ++row) {
            for (int col = 0; col < DIGIT_SIZE; ++col) {
                int idx = row * DIGIT_SIZE + col;
                if (idx < digits[i].size() && digits[i](idx) > 0.5) {
                    std::cout << "██";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Создание нейронной сети
    // Слой 1: 400 -> 128
    Eigen::MatrixXd weights1(128, 400);
    Eigen::VectorXd biases1(128);
    double limit1 = std::sqrt(6.0 / (400 + 128));
    weights1 = Eigen::MatrixXd::Random(128, 400) * limit1;
    biases1.setZero();
    Eigen::VectorXd last_z1;
    Eigen::VectorXd last_input1;
    
    // Слой 2: 128 -> 10
    Eigen::MatrixXd weights2(10, 128);
    Eigen::VectorXd biases2(10);
    double limit2 = std::sqrt(6.0 / (128 + 10));
    weights2 = Eigen::MatrixXd::Random(10, 128) * limit2;
    biases2.setZero();
    Eigen::VectorXd last_z2;
    Eigen::VectorXd last_input2;
    
    // Обучение
    std::cout << "=== Обучение нейронной сети ===" << std::endl;
    const int epochs = 500;
    const double learning_rate = 0.1;
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_loss = 0.0;
        
        for (size_t sample_idx = 0; sample_idx < digits.size(); ++sample_idx) {
            // Forward pass
            Eigen::VectorXd x = digits[sample_idx];
            
            // Слой 1 (Sigmoid)
            last_input1 = x;
            last_z1 = weights1 * x + biases1;
            Eigen::VectorXd layer1_output = (1.0 + (-last_z1).array().exp()).inverse();
            
            // Слой 2 (Softmax)
            last_input2 = layer1_output;
            last_z2 = weights2 * layer1_output + biases2;
            double max_val = last_z2.maxCoeff();
            Eigen::VectorXd exp_z2 = (last_z2.array() - max_val).exp();
            double sum = exp_z2.sum();
            Eigen::VectorXd output = exp_z2 / sum;
            
            // Loss (MSE)
            Eigen::VectorXd diff = output - targets[sample_idx];
            double loss_val = diff.squaredNorm() / output.size();
            total_loss += loss_val;
            
            // Backward pass
            Eigen::VectorXd gradient = 2.0 * diff / output.size();
            
            // Backward для слоя 2 (Softmax)
            Eigen::VectorXd s2 = output;
            Eigen::MatrixXd J2 = Eigen::MatrixXd(s2.asDiagonal()) - s2 * s2.transpose();
            Eigen::VectorXd delta2 = J2 * gradient;
            Eigen::MatrixXd old_weights2 = weights2;
            weights2 -= learning_rate * delta2 * last_input2.transpose();
            biases2 -= learning_rate * delta2;
            Eigen::VectorXd grad1 = old_weights2.transpose() * delta2;
            
            // Backward для слоя 1 (Sigmoid)
            Eigen::VectorXd s1 = layer1_output;
            Eigen::VectorXd activation_grad1 = s1.array() * (1.0 - s1.array());
            Eigen::VectorXd delta1 = grad1.array() * activation_grad1.array();
            Eigen::MatrixXd old_weights1 = weights1;
            weights1 -= learning_rate * delta1 * last_input1.transpose();
            biases1 -= learning_rate * delta1;
        }
        
        if (epoch % 20 == 0 || epoch == epochs - 1) {
            std::cout << "Epoch " << epoch << ", Loss: " << total_loss / digits.size() << std::endl;
        }
    }
    
    // Тестирование
    std::cout << "\n=== Тестирование сети ===" << std::endl;
    for (size_t i = 0; i < digits.size(); ++i) {
        // Forward pass для предсказания
        Eigen::VectorXd x = digits[i];
        
        Eigen::VectorXd z1 = weights1 * x + biases1;
        Eigen::VectorXd layer1_out = (1.0 + (-z1).array().exp()).inverse();
        
        Eigen::VectorXd z2 = weights2 * layer1_out + biases2;
        double max_val = z2.maxCoeff();
        Eigen::VectorXd exp_z2 = (z2.array() - max_val).exp();
        double sum = exp_z2.sum();
        Eigen::VectorXd prediction = exp_z2 / sum;
        
        int predicted_digit = 0;
        double max_prob = prediction(0);
        for (int j = 1; j < 10; ++j) {
            if (prediction(j) > max_prob) {
                max_prob = prediction(j);
                predicted_digit = j;
            }
        }
        
        std::cout << "Цифра " << i << ":" << std::endl;
        for (int row = 0; row < DIGIT_SIZE; ++row) {
            for (int col = 0; col < DIGIT_SIZE; ++col) {
                int idx = row * DIGIT_SIZE + col;
                if (idx < digits[i].size() && digits[i](idx) > 0.5) {
                    std::cout << "██";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Предсказание: " << predicted_digit << " (вероятность: " << max_prob << ")" << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
