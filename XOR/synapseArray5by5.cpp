#include "synapseArray5by5.h"

synapseArray5by5::synapseArray5by5()
{
    this->_range = 200;

    for (int i = 0; i < 5; i++)
    {
        this->_WL[i] = 0;
        this->_ADCvalueTemp[i] = 0;
        this->_ADCvalueN5[i] = 0;
        this->_ADCvalueN6[i] = 0;
        this->_ADCvalueN5N6[i] = 0;
        this->_ADCbias[i] = 0;

        this->_dH[i] = 0;
        this->_dZ[i] = 0;

        for (int j = 0; j < 5; j++)
        {
            this->_min[i][j] = 0;
            this->_mid[i][j] = 0;
            this->_max[i][j] = 0;
            this->_gnd[i][j] = 0;
            this->_ref[i][j] = 0;
            this->_noise[i][j] = 0;

            this->_weight[i][j] = 0;
            this->_weightADC[i][j] = 0;
            this->_initialWeight[i][j] = 0;
            this->_targetWeight[i][j] = 0;
            this->_targetBias[i][j] = 0;

            this->_dW1[i][j] = 0;
            this->_dW2[i][j] = 0;
        }
    }
}
//--------------------------------------------------------------
synapseArray5by5::~synapseArray5by5()
{
}
//--------------------------------------------------------------
void synapseArray5by5::initialize()
{
    this->setInitialWeight();
    this->setTargetWeight();
    this->setTargetBias();
    this->setWeightToInitialWeight();
    this->setADCrefValue();
}
//--------------------------------------------------------------
void synapseArray5by5::refresh()
{
    for (int i = 0; i < 5; i++)
    {
        this->_dH[i] = 0;
        this->_dZ[i] = 0;
        for (int j = 0; j < 5; j++)
        {
            this->_dW1[i][j] = 0;
            this->_dW2[i][j] = 0;
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setRange(int &range)
{
    this->_range = range;
}
//--------------------------------------------------------------
void synapseArray5by5::setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4)
{
    this->_WL[0] = WL0;
    this->_WL[1] = WL1;
    this->_WL[2] = WL2;
    this->_WL[3] = WL3;
    this->_WL[4] = WL4;
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueTemp(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4)
{
    this->_ADCvalueTemp[0] = ADC_0;
    this->_ADCvalueTemp[1] = ADC_1;
    this->_ADCvalueTemp[2] = ADC_2;
    this->_ADCvalueTemp[3] = ADC_3;
    this->_ADCvalueTemp[4] = ADC_4;
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueN5()
{
    this->_ADCvalueN5[0] = this->_ADCvalueTemp[0];
    this->_ADCvalueN5[1] = this->_ADCvalueTemp[1];
    this->_ADCvalueN5[2] = this->_ADCvalueTemp[2];
    this->_ADCvalueN5[3] = this->_ADCvalueTemp[3];
    this->_ADCvalueN5[4] = this->_ADCvalueTemp[4];
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueN6()
{
    this->_ADCvalueN6[0] = this->_ADCvalueTemp[0];
    this->_ADCvalueN6[1] = this->_ADCvalueTemp[1];
    this->_ADCvalueN6[2] = this->_ADCvalueTemp[2];
    this->_ADCvalueN6[3] = this->_ADCvalueTemp[3];
    this->_ADCvalueN6[4] = this->_ADCvalueTemp[4];
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueN5N6()
{
    for (int i = 0; i < 5; i++)
    {
        this->_ADCvalueN5N6[i] = this->_ADCvalueN5[i] - this->_ADCvalueN6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCminValue(int rowNum)
{
    setADCvalueN5N6();
    for (int i = 0; i < 5; i++)
    {
        this->_min[rowNum][i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCmidValue(int rowNum)
{
    for (int i = 0; i < 5; i++)
    {
        this->_mid[rowNum][i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCmaxValue(int rowNum)
{
    for (int i = 0; i < 5; i++)
    {
        this->_max[rowNum][i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCgndValue(int rowNum)
{
    for (int i = 0; i < 5; i++)
    {
        this->_gnd[rowNum][i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCnoiseValue(int rowNum)
{
    for (int i = 0; i < 5; i++)
    {
        this->_noise[rowNum][i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCrefValue()
{
    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->_ref[i][j] = this->_mid[i][j] - (int)round(this->_range * this->_weight[i][j]);
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setWeightToInitialWeight()
{
    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->_weight[i][j] = this->_initialWeight[i][j];
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setInitialWeight()
{

    /*
    Synapse Weight Distribution
    H : input - hidden layer
    O : hidden - output layer

        0   1   2   3   4
    0---H---.---H---.---H
        |   |   |   |   |
    1---.---O---.---.---.
        |   |   |   |   |
    2---H---.---H---.---H
        |   |   |   |   |
    3---.---O---.---.---.
        |   |   |   |   |
    4---.---O---.---.---.

    */

    // 75.80% accuracy
    // Temporary array
    double arr[5][5] = {
        {-4.8384, 0, -2.5380, 0, -0.0782},
        {0, -7.2057, 0, 0, 0},
        {-4.5076, 0, -2.4469, 0, -2.0661},
        {0, 6.5687, 0, 0, 0},
        {0, 0.6730, 0, 0, 0}};

    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->_initialWeight[i][j] = arr[i][j];
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setTargetWeight()
{

    /*
    Synapse Weight Distribution
    H : input - hidden layer
    O : hidden - output layer

        0   1   2   3   4
    0---H---.---H---.---H
        |   |   |   |   |
    1---.---O---.---.---.
        |   |   |   |   |
    2---H---.---H---.---H
        |   |   |   |   |
    3---.---O---.---.---.
        |   |   |   |   |
    4---.---O---.---.---.

    */

    // 100% accuracy
    // Temporary array
    double arr[5][5] = {
        {-4.4431, 0, -3.3611, 0, -0.0424},
        {0, -6.8898, 0, 0, 0},
        {-4.3886, 0, -3.2379, 0, -1.6680},
        {0, 6.3654, 0, 0, 0},
        {0, 0.5086, 0, 0, 0}};

    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->_targetWeight[i][j] = arr[i][j];
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setTargetBias()
{
    /*
    Synapse Weight Distribution
    H : input - hidden layer
    O : hidden - output layer

        0   1   2   3   4
    0---H---.---H---.---H
        |   |   |   |   |
    1---.---O---.---.---.
        |   |   |   |   |
    2---H---.---H---.---H
        |   |   |   |   |
    3---.---O---.---.---.
        |   |   |   |   |
    4---.---O---.---.---.

    */

    // 100% accuracy
    // Temporary array
    double arr[5][5] = {
        {1.6303, 0, 4.9258, 0, -0.3763},
        {0, -2.9429, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}};

    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->_targetBias[i][j] = arr[i][j];
        }
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setdHfromADCvalue()
{
    for (int i = 0; i < 5; i++)
    {
        this->_dH[i] = this->_ADCvalueN5N6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::clearP1()
{
    for (int i = 0; i < 5; i++)
    {
        this->P1[i] = 0;
    }
}
//--------------------------------------------------------------
void synapseArray5by5::clearQ1()
{
    for (int i = 0; i < 5; i++)
    {
        this->Q1[i] = 0;
    }
}
//--------------------------------------------------------------
void synapseArray5by5::clearP2()
{
    for (int i = 0; i < 5; i++)
    {
        this->P2[i] = 0;
    }
}
//--------------------------------------------------------------
void synapseArray5by5::clearQ2()
{
    for (int i = 0; i < 5; i++)
    {
        this->Q2[i] = 0;
    }
}