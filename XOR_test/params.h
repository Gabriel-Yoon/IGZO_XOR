#ifndef PARAMS_H
#define PARAMS_H

class Params
{
public:
    Params(int num_hidden, double deviation);
    void init_random_parameters();
    void optimize_tanh(double learning_rate, int iteration);

    double loss_eval_tanh();
    double get_loss_trace(int epoch);
    double get_Y_hat_predict();

private:
    int input_size;
    int hidden_size;
    int output_size;
    int num_samples;

    double deviation;
    double learning_rate;
    int iteration;

    double X[2][4];
    double Y[4];

    double W1[2][4];
    double B1[4];
    double W2[4];
    double B2;

    double loss_trace[5000];
    double Y_hat_predict;

    void affine(const double W[4], const double X[2], double B, double result[4]);
    double tanh(double x);
    void tanh_activation(const double Z[4], double H[4]);
    double sigmoid(double x);
    void get_gradients_tanh(double dW1[2][4], double dB1[4], double dW2[4], double &dB2, double &loss);
};

#endif
