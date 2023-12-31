#include "neuronLayer.h"

//--------------------------------------------------------------
neuronLayer::neuronLayer()
{
    for (int i = 0; i < 5; i++)
    {
        this->_preNeuronValue[i] = 0.0;
        this->_postNeuronValue[i] = 0.0;
        this->_preNeuronActivationValue[i] = 0.0;
        this->_postNeuronActivationValue[i] = 0.0;
    }
}
//--------------------------------------------------------------
neuronLayer::~neuronLayer()
{
}
//--------------------------------------------------------------
void neuronLayer::refresh()
{
    for (int i = 0; i < 5; i++)
    {
        this->_preNeuronValue[i] = 0.0;
        this->_postNeuronValue[i] = 0.0;
        this->_preNeuronActivationValue[i] = 0.0;
        this->_postNeuronActivationValue[i] = 0.0;
    }
}
//--------------------------------------------------------------
double neuronLayer::sigmoidActivFunc(double &x)
{
    return 1.0 / (1.0 + std::exp(-x));
}
//--------------------------------------------------------------
double neuronLayer::tanhActivFunc(double &x)
{
    double ex = std::exp(x);
    double enx = std::exp(-x);
    return (ex - enx) / (ex + enx);
}
//--------------------------------------------------------------
void neuronLayer::setPreNeuronValues(double *p)
{
    for (int i = 0; i < 5; i++)
    {
        this->_preNeuronValue[i] = p[i];
    }
}
//--------------------------------------------------------------
void neuronLayer::setPostNeuronValues(double *p)
{
    for (int i = 0; i < 5; i++)
    {
        this->_postNeuronValue[i] = p[i];
    }
}
//--------------------------------------------------------------
void neuronLayer::sigmoidActivation()
{
    for (int i = 0; i < 5; i++)
    {
        this->_postNeuronActivationValue[i] = this->sigmoidActivFunc(this->_postNeuronValue[i]);
    }
}
//--------------------------------------------------------------
void neuronLayer::sigmoidActivationPreNeurons()
{
    for (int i = 0; i < 5; i++)
    {
        this->_preNeuronActivationValue[i] = this->sigmoidActivFunc(this->_preNeuronValue[i]);
    }
}
//--------------------------------------------------------------
void neuronLayer::tanhActivation()
{
    for (int i = 0; i < 5; i++)
    {
        this->_postNeuronActivationValue[i] = this->tanhActivFunc(this->_postNeuronValue[i]);
    }
}
//--------------------------------------------------------------
void neuronLayer::syncPostToPreNeuronValues(neuronLayer &arg_neurons)
{
    for (int i = 0; i < 5; i++)
    {
        this->_preNeuronValue[i] = arg_neurons._postNeuronActivationValue[i];
    }
}
//--------------------------------------------------------------
void neuronLayer::copyPreToPostNeuronValues()
{
    for (int i = 0; i < 5; i++)
    {
        this->_postNeuronValue[i] = this->_preNeuronValue[i];
    }
}