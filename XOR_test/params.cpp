#include "params.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

Params::Params(int num_hidden, double deviation)
{
    input_size = 2;
    hidden_size = num_hidden;
    output_size = 1;
    num_samples = 4;

    deviation = deviation;
    learning_rate = 0.1;
    iteration = 5000;

    X[0][4] = {0, 0, 1, 1};
    X[1][4] = {0, 1, 0, 1};
    Y[4] = {0, 1, 1, 0};

    // double X[2][4] = {{0, 0, 1, 1}, {0, 1, 0, 1}}; // Example input data
    // double Y[4] = {0, 1, 1, 0};                    // Example output data
}

void Params::init_random_parameters()
{
    std::srand(std::time(0));

    for (int i = 0; i < input_size; i++)
    {
        for (int j = 0; j < hidden_size; j++)
        {
            W1[i][j] = ((double)std::rand() / RAND_MAX) * deviation;
        }
    }

    for (int i = 0; i < hidden_size; i++)
    {
        B1[i] = ((double)std::rand() / RAND_MAX) * deviation;
        W2[i] = ((double)std::rand() / RAND_MAX) * deviation;
    }

    B2 = ((double)std::rand() / RAND_MAX) * deviation;
}

void Params::affine(const double W[4], const double X[2], double B, double result[4])
{
    for (int i = 0; i < hidden_size; i++)
    {
        result[i] = 0;
        for (int j = 0; j < input_size; j++)
        {
            result[i] += W[i] * X[j];
        }
        result[i] += B;
    }
}

double Params::tanh(double x)
{
    double ex = std::exp(x);
    double enx = std::exp(-x);
    return (ex - enx) / (ex + enx);
}

void Params::tanh_activation(const double Z[4], double H[4])
{
    for (int i = 0; i < hidden_size; i++)
    {
        H[i] = tanh(Z[i]);
    }
}

double Params::sigmoid(double x)
{
    return 1 / (1 + std::exp(-x));
}

double Params::loss_eval_tanh()
{
    double Z1[4];
    double H[4];
    double Z2;

    // Forward: input Layer
    affine(W1[0], X[0], B1[0], Z1);
    affine(W1[1], X[1], B1[1], H);

    // Forward: Hidden Layer
    affine(W2, H, B2, &Z2);
    double Y_hat = sigmoid(Z2);

    double loss = 0.0;
    for (int i = 0; i < num_samples; i++)
    {
        loss += -1 * (Y[i] * std::log(Y_hat) + (1 - Y[i]) * std::log(1 - Y_hat));
    }

    return loss / num_samples;
}

void Params::get_gradients_tanh(double dW1[2][4], double dB1[4], double dW2[4], double &dB2, double &loss)
{
    double Z1[4];
    double H[4];
    double Z2;
    loss = loss_eval_tanh();

    // BackPropagate: Hidden Layer
    double dH[4];
    for (int i = 0; i < hidden_size; i++)
    {
        dW2[i] = H[i] * (sigmoid(Z2) - Y[i]);
        dB2 = (sigmoid(Z2) - Y[i]);
    }

    // BackPropagate: Input Layer
    for (int i = 0; i < hidden_size; i++)
    {
        dH[i] = W2[i] * (sigmoid(Z2) - Y[i]);
    }
    for (int i = 0; i < hidden_size; i++)
    {
        dZ1[i] = dH[i] * (1 - (H[i] * H[i]));
    }

    for (int i = 0; i < input_size; i++)
    {
        for (int j = 0; j < hidden_size; j++)
        {
            dW1[i][j] = X[i][j] * dZ1[j];
        }
    }

    for (int i = 0; i < hidden_size; i++)
    {
        dB1[i] = dZ1[i];
    }

    loss = loss / num_samples;
}

double Params::get_loss_trace(int epoch)
{
    return loss_trace[epoch];
}

double Params::get_Y_hat_predict()
{
    return Y_hat_predict;
}

void Params::optimize_tanh(double learning_rate, int iteration)
{
    for (int epoch = 0; epoch < iteration; epoch++)
    {
        double dparams[hidden_size + hidden_size + 2];
        double dW1[2][4];
        double dB1[4];
        double dW2[4];
        double dB2;
        double loss;

        get_gradients_tanh(dW1, dB1, dW2, dB2, loss);
        for (int i = 0; i < hidden_size; i++)
        {
            params[i] += -learning_rate * dparams[i];
        }
        for (int i = 0; i < hidden_size; i++)
        {
            params[i + hidden_size] += -learning_rate * dparams[i + hidden_size];
        }
        params[hidden_size + hidden_size] += -learning_rate * dparams[hidden_size + hidden_size];

        for (int i = 0; i < hidden_size; i++)
        {
            B1[i] = params[i];
        }
        for (int i = 0; i < hidden_size; i++)
        {
            W2[i] = params[i + hidden_size];
        }
        B2 = params[hidden_size + hidden_size];
        loss_trace[epoch] = loss;

        if (epoch % 100 == 0)
        {
            Y_hat_predict = loss_eval_tanh();
            std::cout << "Epoch " << epoch << " Loss: " << loss << std::endl;
        }
    }
}
