#include "synapseArray5by5.h"

synapseArray5by5::synapseArray5by5()
{
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
void synapseArray5by5::setInitialWeight(const std::string &file_path)
{
    // std::ifstream file(file_path);
    // if (file)
    // {
    //     std::string line;
    //     while (std::getline(file, line))
    //     {
    //         std::vector<double> row;
    //         std::istringstream iss(line);
    //         double value;
    //         while (iss >> value)
    //         {
    //             row.push_back(value);
    //         }
    //         initialWeight.push_back(row);
    //     }
    // }
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
void synapseArray5by5::setADCvalueN5(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4)
{
    this->_ADCvalueN5[0] = ADC_0;
    this->_ADCvalueN5[1] = ADC_1;
    this->_ADCvalueN5[2] = ADC_2;
    this->_ADCvalueN5[3] = ADC_3;
    this->_ADCvalueN5[4] = ADC_4;
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueN6(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4)
{
    this->_ADCvalueN6[0] = ADC_0;
    this->_ADCvalueN6[1] = ADC_1;
    this->_ADCvalueN6[2] = ADC_2;
    this->_ADCvalueN6[3] = ADC_3;
    this->_ADCvalueN6[4] = ADC_4;
}
//--------------------------------------------------------------
void synapseArray5by5::setADCvalueN5N6(int ADC_0, int ADC_1, int ADC_2, int ADC_3, int ADC_4)
{
    for (int i = 0; i < 5; i++)
    {
        this->_ADCvalueN5N6[i] = this->_ADCvalueN5[i] - this->_ADCvalueN6[i];
    }
}
//--------------------------------------------------------------
void synapseArray5by5::setADCminValue(int rowNum)
{
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