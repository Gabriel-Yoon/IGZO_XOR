#ifndef _ADC_MATRIX_H_
#define _ADC_MATRIX_H_

enum : int
{
    min,
    max
};

// Forward declarations.

//**************************************************************************************************************//
class ADC_matrix
{ // storing neuron potential. Be aware that there is no position variable

    /*---------------------fields-----------------*/
public:
    int _output[5];
    int _standard[5][2]; // min, max
    int _mid[5][5];
    /*---------------------methods----------------*/

public:
    ADC_matrix();
    ~ADC_matrix();
    void readScalingValue(int Ds0, int Ds1, int Ds2, int Ds3, int Ds4, int _delayMicroseconds);
    int getScalingValue();
};
//**************************************************************************************************************//
#endif
