#include <iostream>
#include <cmath>
#include <algorithm>
#include <Eigen/Core>

class ActivationFunction {
public:
    virtual double activate(double x) = 0;
    virtual double derivative(double x) = 0;

};

class Sigmoid : public ActivationFunction {
public:
    double activate(double x) override {
        return 1.0 / (1.0 + std::exp(-x));
    }
    
    double derivative(double x) override {
        double s = activate(x);
        return s * (1.0 - s);
    }
};

class ReLU : public ActivationFunction {
public:
    double activate(double x) override {
        return std::max(0.0, x);
    }
    
    double derivative(double x) override {
        return x > 0.0 ? 1.0 : 0.0;
    }
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}