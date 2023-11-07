#ifndef _NEURON_LAYER_H_
#define _NEURON_LAYER_H_

#include <vector>
#include <math.h>
#include <cmath>

//**************************************************************************************************************//
class neuronLayer
{
    /*---------------------fields-----------------*/
public:
    double _preNeuronValue[5];
    double _preNeuronActivationValue[5];
    double _postNeuronValue[5];
    double _postNeuronActivationValue[5];

    /*---------------------methods----------------*/
public:
    neuronLayer();
    ~neuronLayer();

    double sigmoidActivFunc(double &x);
    double tanhActivFunc(double &x);

    void setPreNeuronValues(double *p);
    void setPostNeuronValues(double *p);
    void setPreNeuronActivationValues();
    void setPostNeuronActivationValues();

    void sigmoidActivation();
    void sigmoidActivationPreNeurons();
    void tanhActivation();
    void syncPostToPreNeuronValues(neuronLayer &arg_neurons);
    void copyPreToPostNeuronValues();
};
//**************************************************************************************************************//
#endif