#include "Params.h"
#include <random>

Params::Params()
{
    int num_hidden = 2;

    this->B2 = 0;
    this->dB2 = 0;

    for (int i = 0; i < 2; i++)
    {
        B1[i] = 0;
        dB1[i] = 0;
        W2[i] = 0;
        dW2[i] = 0;
        for (int j = 0; j < 2; j++)
        {
            W1[i][j] = 0;
            dW1[i][j] = 0;
            dZ1[i][j] = 0;
        }
    }
}

Params::~Params()
{
}

void Params::init_random_parameters()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double deviation = 0.1;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < this->num_hidden; j++)
        {
            W1[i][j] = dist(gen) * deviation;
            dW1[i][j] = 0; // Initialize gradients to 0
        }
    }

    for (int i = 0; i < this->num_hidden; i++)
    {
        B1[i] = dist(gen) * deviation;
        dB1[i] = 0; // Initialize gradients to 0
    }

    for (int i = 0; i < this->num_hidden; i++)
    {
        W2[i] = dist(gen) * deviation;
        dW2[i] = 0; // Initialize gradients to 0
    }

    B2 = dist(gen) * deviation;
    dB2 = 0; // Initialize gradients to 0
}

void Params::print_params()
{
    int num_hidden = 2;
    // Print the initialized parameters
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < num_hidden; j++)
        {
            Serial.print("W1 : ");
            Serial.print(this->W1[i][j]);
        }
        Serial.println();
    }

    for (int i = 0; i < num_hidden; i++)
    {
        Serial.print("B1 : ");
        Serial.print(this->B1[i]);
    }
    Serial.println();

    for (int i = 0; i < num_hidden; i++)
    {
        Serial.print("W2 : ");
        Serial.print(this->W2[i]);
    }
    Serial.println();

    Serial.print("B2 : ");
    Serial.print(this->B2);
    Serial.println();
}