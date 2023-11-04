#ifndef _PARAMS_H_
#define _PARAMS_H_

#include <vector>
#include <math.h>

class Params
{
public:
    int num_hidden;
    double W1[2][2];
    double B1[2];
    double W2[2];
    double B2;
    double dW1[2][2];
    double dB1[2];
    double dW2[2];
    double dB2;
    double dZ1[2][2];

public:
    Params();
    ~Params();

    void init_random_parameters();
    void print_params();
};