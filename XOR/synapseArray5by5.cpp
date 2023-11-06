#include "synapseArray5by5.h"

synapseArray5by5::synapseArray5by5()
{
    // Resize
    this->_ADCvalueN5.resize(5);
    this->_ADCvalueN6.resize(5);
    this->_ADCvalueN5N6.resize(5);

    this->_min.resize(5);
    this->_mid.resize(5);
    this->_max.resize(5);

    this->initialWeight.resize(5);
    this->targetWeight.resize(5);
    this->targetBias.resize(5);

    for (int i = 0; i < 5; i++)
    {
        this->initialWeight[i].resize(5);
        this->targetWeight[i].resize(5);
        this->targetBias[i].resize(5);
    }

    // Initialize
    for (int i = 0; i < 5; i++)
    {
        this->_WL[i] = 0;
        this->_ADCvalueN5[i] = 0;
        this->_ADCvalueN6[i] = 0;
        this->_ADCvalueN5N6[i] = 0;
        for (int j = 0; j < 5; j++)
        {
            this->_min[i][j] = 0;
            this->_mid[i][j] = 0;
            this->_max[i][j] = 0;

            this->initialWeight[i][j] = 0;
            this->targetWeight[i][j] = 0;
            this->targetBias[i][j] = 0;
        }
    }
}
//--------------------------------------------------------------
synapseArray5by5::~synapseArray5by5()
{
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

    std::vector<std::vector<double>> vec{
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
            this->initialWeight[i][j] = vec[i][j];
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

    std::vector<std::vector<double>> vec{
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
            this->targetWeight[i][j] = vec[i][j];
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
    std::vector<std::vector<double>> vec{
        {1.6303, 0, 4.9258, 0, -0.3763},
        {0, -2.9429, 0, 0, 0},
        {1.6303, 0, 4.9258, 0, -0.3763},
        {0, -2.9429, 0, 0, 0},
        {0, -2.9429, 0, 0, 0}};

    // Synchronize
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            this->targetBias[i][j] = vec[i][j];
        }
    }
}