#include "synapseArray5by5.h"

synapseArray5by5::synapseArray5by5()
{
    for (int i = 0; i < 5; i++)
    {
        this->_WL[i] = 0;
        this->_ADCvalue[i] = 0;
        this->_standard[i][0] = 0;
        this->_standard[i][1] = 0;
        for (int j = 0; j < 5; j++)
        {
            this->_mid[i][j] = 20;
        }
    }

    // For the test value,
    _standard[0][min] = -298;
    _standard[0][max] = 303;
    _standard[1][min] = -324;
    _standard[1][max] = 339;
    _standard[2][min] = -301;
    _standard[2][max] = 325;
    _standard[3][min] = -290;
    _standard[3][max] = 321;
    _standard[4][min] = -290;
    _standard[4][max] = 297;
}

synapseArray5by5::~synapseArray5by5()
{
}

void synapseArray5by5::setWLPulseWidth(int &WL0, int &WL1, int &WL2, int &WL3, int &WL4)
{
    this->_WL[0] = WL0;
    this->_WL[1] = WL1;
    this->_WL[2] = WL2;
    this->_WL[3] = WL3;
    this->_WL[4] = WL4;
}

void synapseArray5by5::setADCvalueN5(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4)
{
    this->_ADCvalueN5[0] = ADC_0;
    this->_ADCvalueN5[1] = ADC_1;
    this->_ADCvalueN5[2] = ADC_2;
    this->_ADCvalueN5[3] = ADC_3;
    this->_ADCvalueN5[4] = ADC_4;
}

void synapseArray5by5::setADCvalueN6(int &ADC_0, int &ADC_1, int &ADC_2, int &ADC_3, int &ADC_4)
{
    this->_ADCvalueN6[0] = ADC_0;
    this->_ADCvalueN6[1] = ADC_1;
    this->_ADCvalueN6[2] = ADC_2;
    this->_ADCvalueN6[3] = ADC_3;
    this->_ADCvalueN6[4] = ADC_4;
}

void synapseArray5by5::setADCvalue()
{
    for (int i = 0; i < 5; i++)
    {
        this->_ADCvalue[i] = this->_ADCvalueN5[i] - this->_ADCvalueN6[i];
    }
}