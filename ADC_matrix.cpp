#include "ADC_matrix.h"

ADC_matrix::ADC_matrix()
{
    for (int i = 0; i < 5; i++)
    {
        _output[i] = 0;

        for (int j = 0; j < 5; j++)
        {
            _mid[i][j] = 20;
        }
    }

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

ADC_matrix::~ADC_matrix() {}

void ADC_matrix::readScalingValue(int Ds0, int Ds1, int Ds2, int Ds3, int Ds4, int _delayMicroseconds = 1000)
{

    int d1 = 1;
    int d2 = 1 << 1;
    int d3 = 1 << 2;
    int d4 = 1 << 3;
    int d5 = 1 << 6;
    int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
    int set[MAX];
    int c = d1 | d2 | d3 | d4 | d5;
    int D1[MAX], D2[MAX], D3[MAX], D4[MAX], D5[MAX];

    Serial.println(Ds0);
    for (int i = 0; i < MAX; i++)
    {
        if (i < Ds0)
            D1[i] = d1;
        else
            D1[i] = 0;
        Serial.println(D1[i]);
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < Ds1)
            D2[i] = d2;
        else
            D2[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < Ds2)
            D3[i] = d3;
        else
            D3[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < Ds3)
            D4[i] = d4;
        else
            D4[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < Ds4)
            D5[i] = d5;
        else
            D5[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        set[i] = D1[i] | D2[i] | D3[i] | D4[i] | D5[i];
    }

    PIOB->PIO_CODR = 1 << 14; // CON
    PIOA->PIO_SODR = 1 << 19; // DS
    PIOB->PIO_SODR = 1 << 21; // CR

    for (int i = 0; i < MAX; i++)
    {
        PIOD->PIO_SODR = set[i]; // D SET
        PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
        PIOD->PIO_CODR = c;      // D clear
        delayMicroseconds(_delayMicroseconds);
    }
    PIOA->PIO_SODR = 1 << 7;  // DFF1 CLK HIGH
    PIOA->PIO_CODR = 1 << 7;  // DFF1 CLK LOW
    PIOA->PIO_CODR = 1 << 19; // DS ON
    PIOB->PIO_SODR = 1 << 14; // CON OFF

    ADC_0 = ADC->ADC_CDR[7];  // read data on A0
    ADC_1 = ADC->ADC_CDR[6];  // read data on A1
    ADC_2 = ADC->ADC_CDR[5];  // read data on A2
    ADC_3 = ADC->ADC_CDR[4];  // read data on A3
    ADC_4 = ADC->ADC_CDR[3];  // read data on A4
    PIOB->PIO_CODR = 1 << 21; // CR

    Serial.print("ADC0 Value =");
    Serial.print(ADC_0 / 4);
    Serial.print(", ADC1 Value =");
    Serial.print(ADC_1 / 4);
    Serial.print(", ADC2 Value =");
    Serial.print(ADC_2 / 4);
    Serial.print(", ADC3 Value =");
    Serial.print(ADC_3 / 4);
    Serial.print(", ADC4 Value =");
    Serial.println(ADC_4 / 4);
}

int ADC_matrix::getScalingValue()
{
}