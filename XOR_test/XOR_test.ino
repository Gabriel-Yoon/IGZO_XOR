#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#undef min
#undef max
#include <vector>
// #include <Vector.h>
#include "synapseArray5by5.h"

#undef max
#undef min
/*

SAM3X-Arduino Pin Mapping
(Arduino DUE pin mapping table)

    PIN     PIN NAME
2   PB25    Digital Pin 2       HL_CHOP
3
4
5   PC25    Digital Pin 5       UD_EN4
6   PC24    Digital Pin 6       UD_EN3
7   PC23    Digital Pin 7       UD_EN2
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22  PB26    Digital Pin 22      FB
23
24
25  PD0     Digital Pin 25      D0
26  PD1     Digital Pin 26      D1
27  PD2     Digital Pin 26      D2
28  PD3     Digital Pin 26      D3
29  PD6     Digital Pin 26      D4
30
31  PA7     Digital Pin 31      DFF1
32
33  PC1     Digital Pin 33      UD_EN1
34  PC2     Digital Pin 34      UD_EN0
35  PC3     Digital Pin 35      UP_EN4
36  PC4     Digital Pin 36      UP_EN3
37  PC5     Digital Pin 37      UP_EN2
38  PC6     Digital Pin 38      UP_EN1
39  PC7     Digital Pin 39      UP_EN0
40  PC8     Digital Pin 40      UD4
41  PC9     Digital Pin 41      UD3
42  PA19    Digital Pin 42      DS
43
44  PC19    Digital Pin 44      UD2
45  PC18    Digital Pin 45      UD2
46  PC17    Digital Pin 46      UD1
47  PC16    Digital Pin 47      UP4
48  PC15    Digital Pin 48      UP3
49  PC14    Digital Pin 49      UP2
50  PC13    Digital Pin 50      UP1
51  PC12    Digital Pin 51      UP0
52  PB21    Digital Pin 52      CR
53  PB14    Digital Pin 53      CON

*/

// FIELDS *********************************************

#define MAX 120          // Read pulse set까지의 시간을 위하여
#define Bit_length 10    // update할 때의 timing을 맞추기 위해서 따로 정의
#define read_set_time 1  // [ms]
#define read_delay 1     // [ms]
#define VAR_NUM 10       // 총 Input variable 수를 정의할 것
#define learning_rate 30 // 1st layer의 learning rate 정의
#define amplification_factor 8
#define DECISION_BOUNDARY 0
#define PRINTER(name) printer(#name, (name))

enum : int
{
    min_val,
    max_val
};

struct layer
{
    double wsum[5];
    double activationValue[5];
};

int DFF1, FB, CR, CON, CON2, HL_CHOP;
int N1[5][Bit_length], N2[5][Bit_length], N3[5][Bit_length], N4[5][Bit_length], N5[5][Bit_length];

synapseArray5by5 core;

int target;
double target_real;

double X[5]; // X0, X1, X2, X3, X4
double Y[5];
int X0_real, X1_real;
int pulseWidthWL[5];

double p[5]; // p0, p1, p2
double q[5]; // q0, q1, q2, q3, q4
int P[5];
int Q[5];
int result[5];

double error, loss;
double learning_rate_partial;
double b3, b4;

// Export
double ErrorEpochRecorder[100];

// FORWARD DECLARATION ************************************

void state_switch(int state);
void read_scaling_pulse(int ds0, int ds1, int ds2, int ds3, int ds4, int *result);
void Potentiation(int *N1_0, int *N1_1, int *N1_2, int *N1_3, int *N1_4, int *N2_0, int *N2_1, int *N2_2, int *N2_3, int *N2_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time);
void Depression(int *N3_0, int *N3_1, int *N3_2, int *N3_3, int *N3_4, int *N4_0, int *N4_1, int *N4_2, int *N4_3, int *N4_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time);
void Feedforward(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core);
void Backpropagation(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core);
void calculateLayerValues(double *arg_X, synapseArray5by5 &arg_core, layer &arg_layer);
void SGDsetRegisterPotentiation(int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime);
void SGDsetRegisterDepression(int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime);

// SETUP **************************************************
void setup()
{
    pinMode(53, OUTPUT);
    pinMode(52, OUTPUT);
    pinMode(51, OUTPUT);
    pinMode(50, OUTPUT);
    pinMode(49, OUTPUT);
    pinMode(48, OUTPUT);
    pinMode(47, OUTPUT);
    pinMode(46, OUTPUT);
    pinMode(45, OUTPUT);
    pinMode(44, OUTPUT);
    pinMode(43, OUTPUT); // not sure why
    pinMode(42, OUTPUT);
    pinMode(41, OUTPUT);
    pinMode(40, OUTPUT);
    pinMode(39, OUTPUT);
    pinMode(38, OUTPUT);
    pinMode(37, OUTPUT);
    pinMode(36, OUTPUT);
    pinMode(35, OUTPUT);
    pinMode(34, OUTPUT);
    pinMode(33, OUTPUT);
    pinMode(31, OUTPUT);
    pinMode(29, OUTPUT);
    pinMode(28, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);

    ADC->ADC_MR |= 0x80;  // Mode FREERUN, register in the ADC module that controls its operating modes and settings
    ADC->ADC_CR = 2;      // Start converter, control register in the ADC module that is used to trigger conversions.
    ADC->ADC_CHER = 0xF8; // Enabling all channels, channel enable register in the ADC module

    Serial.begin(115200); // Board Rate 115200
}

// LOOP **************************************************
void loop()
{
    String D0_string, D1_string, D2_string, D3_string, D4_string, pulse_width_string, pre_enable_string, post_enable_string, zero_time_string, epoch_string;
    String Input_string;

    int index[VAR_NUM] = {
        0,
    }; // set every elements to zero

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < Bit_length; j++)
        {
            N1[i][Bit_length] = 0;
            N2[i][Bit_length] = 0;
            N3[i][Bit_length] = 0;
            N4[i][Bit_length] = 0;
            N5[i][Bit_length] = 0;
        }
    }

    if (Serial.available() > 0)
    {
        // SERIAL INPUT --------------------------------------------------
        Input_string = Serial.readString();
        index[0] = Input_string.indexOf(",");
        for (int i = 0; i < VAR_NUM - 1; i++)
        {
            index[i + 1] = Input_string.indexOf(",", index[i] + 1);
        }

        D0_string = Input_string.substring(0, index[0]);
        D1_string = Input_string.substring(index[0] + 1, index[1]);
        D2_string = Input_string.substring(index[1] + 1, index[2]);
        D3_string = Input_string.substring(index[2] + 1, index[3]);
        D4_string = Input_string.substring(index[3] + 1, index[4]);

        pulse_width_string = Input_string.substring(index[4] + 1, index[5]); // micro
        pre_enable_string = Input_string.substring(index[5] + 1, index[6]);  // micro
        post_enable_string = Input_string.substring(index[6] + 1, index[7]); // micro
        zero_time_string = Input_string.substring(index[7] + 1, index[8]);   // micro
        epoch_string = Input_string.substring(index[8] + 1, index[9]);

        int D0 = D0_string.toInt();
        int D1 = D1_string.toInt();
        int D2 = D2_string.toInt();
        int D3 = D3_string.toInt();
        int D4 = D4_string.toInt();

        int pulseWidth = pulse_width_string.toInt();
        int preEnableTime = pre_enable_string.toInt();
        int postEnableTime = post_enable_string.toInt();
        int zeroTime = zero_time_string.toInt();
        int epoch = epoch_string.toInt();

        // XOR Problem Scheme
        layer inputLayer;
        layer hiddenLayer;
        layer outputLayer;
        Serial.println("This is a test code to examine BackPropagation");

        // EPOCH ---------------------------------------------------------
        for (int i = 0; i < epoch; i++)
        {
            Serial.println("// ---------------------------------------------------------");
            Serial.print("epoch = ");
            Serial.println(i + 1);

            // XOR Problem Scheme
            /* Feed-Forward 1

                    X0=> WL0--  **     **

                    X1=> WL1--  **     **

                    X2=> WL2--  **     **       ##

                         WL3--                  ##

                         WL4--                  ##

                                |      |       |       |       |
                                adc0    adc1    adc2    adc3    adc4
                                h0      h1
            */

            /* Feed-Forward 2

                         WL0--

                         WL1--

            b2 (bias) => WL2--                  (2,2)
            hiddenLayer
            Value[0]  => WL3--                  (2,3)                   q3
            hiddenLayer
            Value[1]  => WL4--                  (2,4)                   q4

                                |      |       |       |       |
                                adc0    adc1    adc2    adc3    adc4
                                                outputLayer
                                                Value[0]
            */

            // Feed-Forward 1 : Visible -> Hidden Layer ----------------------------
            X[0] = rand() % 2;
            X[1] = rand() % 2;
            X[2] = 1;
            X[3] = 0;
            X[4] = 0;

            for (int i = 0; i < 5; i++)
            {
                pulseWidthWL[i] = pulseWidth;
            }

            X0_real = (X[0] == 0) ? 0 : 1; // WL0 = round(6200 * X0_real);
            X1_real = (X[1] == 0) ? 0 : 1; // WL1 = round(6200 * X1_real);

            target = X0_real ^ X1_real;
            target_real = (target == 0) ? 0.25 : 0.75;

            // N1, N3 line pulse occurence probability
            q[0] = X0_real;
            q[1] = X1_real;
            q[2] = X[2];

            Feedforward(X, pulseWidthWL, core);
            hiddenLayer = calculateLayerValues(X, core);
            // calculateLayerValues(X, core, hiddenLayer);

            // Feed-Forward 2 : Hidden -> Output Layer ----------------------------
            X[0] = 0;
            X[1] = 0;
            X[2] = 1;
            X[3] = hiddenLayer.activationValue[0]; // h0
            X[4] = hiddenLayer.activationValue[1]; // h1

            PRINTER(X[3]);
            PRINTER(X[4]);

            for (int i = 0; i < 5; i++)
            {
                pulseWidthWL[i] = pulseWidth;
            }

            // N1, N3 line pulse occurence probability
            q[3] = X[3];
            q[4] = X[4];

            Feedforward(X, pulseWidthWL, core);
            outputLayer = calculateLayerValues(X, core);
            // calculateLayerValues(X, core, outputLayer);

            // Back Propagation 1--------------------------------------------------
            /*
                    adc0 --

                    adc1 --

                    adc2 --                  **(2,2)

                    adc3 --                  **(2,3)
                    b3
                    adc4 --                  **(2,4)
                    b4
                                |      |       |       |       |
                                Y[0]    Y[1]    Y[2]    Y[3]    Y[4]
                                                BP_outputLayer
            */

            /*
                Gradient Descent : Chain rule

            E       = 1/2 * (target - output)^2

            dE/dw   = dE/d_output       * d_output/d_input      * d_input/dw
                    = -(target-output)  * output * (1-output)   * last_value of sigmoid output
                    = error             * output * (1-output)   * last_value of sigmoid output
            */

            // BackPropagation: Hidden Layer
            // Y_hat = outputLayer.activationValue[0]
            // Y = target_real

            // BackPropagation: Input Layer

            // Calculate error
            error = outputLayer.activationValue[0] - target_real; // (y-t) value
            double Error = 0.5 * error * error;
            loss = 100 * error * error;
            Serial.print("loss  = ");
            Serial.println(loss);
            ErrorEpochRecorder[epoch] = loss;

            PRINTER(outputLayer.activationValue[0]);

            // Multiply amplification_factor for the value to be inside ADC(0~1023 10bit) range
            // double BP_outputLayer = error * (outputLayer.activationValue[0]) * (1 - outputLayer.activationValue[0]) * amplification_factor;

            // 앞서 정의된 BP_outputLayer의 값에서 소자의 값을 반영한 특정값을 곱해준 만큼 역전(backpropagation)을 시켜줌(특정값은 feedforward 과정에서 곱해준 일정한 상수와 일치를 시켜주어야 한다)
            // PRINTER(BP_outputLayer);

            Y[0] = 0;
            Y[1] = 0;
            Y[2] = 0.5;
            Y[3] = 0;
            Y[4] = 0;

            for (int i = 0; i < 5; i++)
            {
                pulseWidthWL[i] = pulseWidth;
            }

            Backpropagation(Y, pulseWidthWL, core);
            layer hiddenLayerBackProp;
            hiddenLayerBackProp = calculateLayerValues(Y, core);
            // calculateLayerValues(Y, core, hiddenLayerBackProp);

            // Weight Update 1 : Output -> Hidden Layer ----------------------------
            /* Weight Update

                    Q[0] --

                    Q[1] --

                    Q[2] --                  **(2,2)

                    Q[3] --                  **(2,3)

                    Q[4] --                  **(2,4)

                                |      |       |       |       |
                                P[0]    P[1]    P[2]    P[3]    P[4]

            */
        }
    }
}
//**************************************************************************************************************//
/*---------------------METHODS----------------*/

void state_switch(int state)
{                   // Read 동작시에 CR핀 set할것, 조금씩 누적되는 경우 발생!
    if (state == 1) // Potentiation
    {
        PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
    }
    else if (state == 2) // Depression
    {
        PIOB->PIO_CODR = 1 << 26;
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
    }
    else if (state == 3)
    { // Read Forward 이는 Feedforward를 수행하는 경우입니다. Feedforward를 통해 N5를 통해 흐르는 전류를 읽는 과정이 state 3번입니다!
        PIOB->PIO_CODR = 1 << 26;
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
                                  // delay(3000);
    }
    else if (state == 4)
    { // Read Backward, 이는 Backpropagation을 수행하는 경우입니다. Backpropagation을 통해 N5를 통해 흐르는 전류를 읽는 과정이 state 4번입니다!
        PIOB->PIO_SODR = 1 << 26;
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
                                  // delay(3000);
    }
    else if (state == 5)
    { // Read LOW Forward 이는 Feedforward를 수행하는데, N6를 통해 흐르는 전류를 읽는 과정을 나타냅니다. 6T1C 소자 configuration 상, N6가 밑에 달려있어 Read Low forward라고 주석에 달아놓은 것으로 보입니다.
        PIOB->PIO_CODR = 1 << 26;
        PIOB->PIO_SODR = 1 << 25; // digitalWrite(HL_CHOP,HIGH)
                                  // delay(3000);
    }
    else if (state == 6)
    { // Read LOW Backward 이는 마찬가지로 N6를 통해 흐르는 전류를 읽는데 Backpropagation을 통해 흐르는 전류를 읽는 state입니다. 3번부터 6번까지의 state를 보시면 1<<25, 1<<26이 각각 CODR인지 SODR인지 4개의 조합이 있는 것을 보실 수 있습니다. 이는 Feedforward와 Backpropagation 그리고 N5를 읽는 과정, N6를 읽는 과정이 2개의 chopper라는 회로로 이뤄지기 때문입니다. 이에 대한 설명은 추가로 드리는 PPT에 그림으로 잘 나와있을테니 이해가 되실겁니다!
        PIOB->PIO_SODR = 1 << 26;
        PIOB->PIO_SODR = 1 << 25; // digitalWrite(HL_CHOP,HIGH)
                                  // delay(3000);
    }
    else
    {
        PIOB->PIO_CODR = 1 << 26;
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW) // 즉, state_switch 에서는 각각 성원님께서 수행하고자 하는 과정 속에서 chopper를 어떻게 컨트롤해야하는지 자세히 나와있습니다. 이를 보시고, XOR 코드를 보시면 더 이해가 잘 가실 것으로 생각됩니다!
    }
}

void read_scaling_pulse(int ds0, int ds1, int ds2, int ds3, int ds4, int *result)
{
    int d1 = 1;
    int d2 = 1 << 1;
    int d3 = 1 << 2;
    int d4 = 1 << 3;
    int d5 = 1 << 6;
    int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
    int c = d1 | d2 | d3 | d4 | d5;
    int set[MAX];
    int WL_0[MAX], WL_1[MAX], WL_2[MAX], WL_3[MAX], WL_4[MAX];

    for (int i = 0; i < MAX; i++)
    {
        if (i < ds0)
            WL_0[i] = d1;
        else
            WL_0[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds1)
            WL_1[i] = d2;
        else
            WL_1[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds2)
            WL_2[i] = d3;
        else
            WL_2[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds3)
            WL_3[i] = d4;
        else
            WL_3[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds4)
            WL_4[i] = d5;
        else
            WL_4[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        set[i] = WL_0[i] | WL_1[i] | WL_2[i] | WL_3[i] | WL_4[i];
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
        delayMicroseconds(100);  // read 한 번 돌때의 단위로 추정됨. 이는 random한 input data를 가지고 실험할 때는 더 줄여야 함.
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

    result[0] = ADC_0 / 4; // ADC0 value
    result[1] = ADC_1 / 4; // ADC1 value
    result[2] = ADC_2 / 4; // ADC2 value
    result[3] = ADC_3 / 4; // ADC3 value
    result[4] = ADC_4 / 4; // ADC4 value
}

void read_scaling(int ds0, int ds1, int ds2, int ds3, int ds4, int *result)
{
    int d1 = 1;
    int d2 = 1 << 1;
    int d3 = 1 << 2;
    int d4 = 1 << 3;
    int d5 = 1 << 6;
    int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
    int c = d1 | d2 | d3 | d4 | d5;
    int set[MAX];
    int WL_0[MAX], WL_1[MAX], WL_2[MAX], WL_3[MAX], WL_4[MAX];

    for (int i = 0; i < MAX; i++)
    {
        if (i < ds0)
            WL_0[i] = d1;
        else
            WL_0[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds1)
            WL_1[i] = d2;
        else
            WL_1[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds2)
            WL_2[i] = d3;
        else
            WL_2[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds3)
            WL_3[i] = d4;
        else
            WL_3[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        if (i < ds4)
            WL_4[i] = d5;
        else
            WL_4[i] = 0;
    }
    for (int i = 0; i < MAX; i++)
    {
        set[i] = WL_0[i] | WL_1[i] | WL_2[i] | WL_3[i] | WL_4[i];
    }

    PIOB->PIO_CODR = 1 << 14; // CON
    PIOA->PIO_SODR = 1 << 19; // DS
    PIOB->PIO_SODR = 1 << 21; // CR

    for (int i = 0; i < read_set_time; i++)
    {
        PIOD->PIO_SODR = set[i]; // WL SET
        PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
        PIOD->PIO_CODR = c;      // WL clear
        delayMicroseconds(1000); //
    }
    for (int i = 0; i < read_delay; i++)
    {
        PIOD->PIO_SODR = 0;      //
        PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
        PIOD->PIO_CODR = set[i]; // WL clear
        delayMicroseconds(10);   //
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

    // Serial.print(ADC_0 / 4);
    // Serial.print(",");
    // Serial.print(ADC_1 / 4);
    // Serial.print(",");
    // Serial.print(ADC_2 / 4);
    // Serial.print(",");
    // Serial.print(ADC_3 / 4);
    // Serial.print(",");
    // Serial.print(ADC_4 / 4);

    result[0] = ADC_0 / 4; // ADC0 value
    result[1] = ADC_1 / 4; // ADC1 value
    result[2] = ADC_2 / 4; // ADC2 value
    result[3] = ADC_3 / 4; // ADC3 value
    result[4] = ADC_4 / 4; // ADC4 value

    //  return ADC_0;
}

void Potentiation(int *N1_0, int *N1_1, int *N1_2, int *N1_3, int *N1_4, int *N2_0, int *N2_1, int *N2_2, int *N2_3, int *N2_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time)
{
    int n1_0, n1_1, n1_2, n1_3, n1_4, n2_0, n2_1, n2_2, n2_3, n2_4 = 0;
    int set[Bit_length];
    int set_update[Bit_length];
    int n1_clear = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
    int n2_clear = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3);

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    for (int i = 0; i < Bit_length; i++)
    {
        if (N1_0[i] == 1)
            n1_0 = 1 << 12;
        else
            n1_0 = 0;
        if (N1_1[i] == 1)
            n1_1 = 1 << 13;
        else
            n1_1 = 0;
        if (N1_2[i] == 1)
            n1_2 = 1 << 14;
        else
            n1_2 = 0;
        if (N1_3[i] == 1)
            n1_3 = 1 << 15;
        else
            n1_3 = 0;
        if (N1_4[i] == 1)
            n1_4 = 1 << 16;
        else
            n1_4 = 0;
        if (N2_0[i] == 1)
            n2_0 = 1 << 7;
        else
            n2_0 = 0;
        if (N2_1[i] == 1)
            n2_1 = 1 << 6;
        else
            n2_1 = 0;
        if (N2_2[i] == 1)
            n2_2 = 1 << 5;
        else
            n2_2 = 0;
        if (N2_3[i] == 1)
            n2_3 = 1 << 4;
        else
            n2_3 = 0;
        if (N2_4[i] == 1)
            n2_4 = 1 << 3;
        else
            n2_4 = 0;

        set[i] = n1_0 | n1_1 | n1_2 | n1_3 | n1_4;
        set_update[i] = n2_0 | n2_1 | n2_2 | n2_3 | n2_4;
    }

    for (int i = 0; i < Bit_length; i++)
    {
        PIOC->PIO_CODR = n1_clear; // N1 clear
        PIOC->PIO_CODR = n2_clear; // N2 clear
        delayMicroseconds(zero_time);
        PIOC->PIO_SODR = set[i]; // N1 set
        delayMicroseconds(pre_enable_time);
        PIOC->PIO_SODR = set_update[i];      // N2 set
        delayMicroseconds(pulse_width);      // N1 N2 pulse width setting
        PIOC->PIO_CODR = n2_clear;           // N2 clear
        delayMicroseconds(post_enable_time); // N1 N2 pulse delay setting
        PIOC->PIO_CODR = n1_clear;           // N1 clear
        delayMicroseconds(zero_time);
    }
}

void Depression(int *N3_0, int *N3_1, int *N3_2, int *N3_3, int *N3_4, int *N4_0, int *N4_1, int *N4_2, int *N4_3, int *N4_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time)
{
    int n3_0, n3_1, n3_2, n3_3, n3_4, n4_0, n4_1, n4_2, n4_3, n4_4 = 0;
    int set[Bit_length];
    int set_update[Bit_length];
    int n3_clear = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
    int n4_clear = (1 << 2) | (1 << 1) | (1 << 23) | (1 << 24) | (1 << 25);

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    for (int i = 0; i < Bit_length; i++)
    {
        if (N3_0[i] == 1)
            n3_0 = 1 << 17;
        else
            n3_0 = 0;
        if (N3_1[i] == 1)
            n3_1 = 1 << 18;
        else
            n3_1 = 0;
        if (N3_2[i] == 1)
            n3_2 = 1 << 19;
        else
            n3_2 = 0;
        if (N3_3[i] == 1)
            n3_3 = 1 << 9;
        else
            n3_3 = 0;
        if (N3_4[i] == 1)
            n3_4 = 1 << 8;
        else
            n3_4 = 0;
        if (N4_0[i] == 1)
            n4_0 = 1 << 2;
        else
            n4_0 = 0;
        if (N4_1[i] == 1)
            n4_1 = 1 << 1;
        else
            n4_1 = 0;
        if (N4_2[i] == 1)
            n4_2 = 1 << 23;
        else
            n4_2 = 0;
        if (N4_3[i] == 1)
            n4_3 = 1 << 24;
        else
            n4_3 = 0;
        if (N4_4[i] == 1)
            n4_4 = 1 << 25;
        else
            n4_4 = 0;

        set[i] = n3_0 | n3_1 | n3_2 | n3_3 | n3_4;
        set_update[i] = n4_0 | n4_1 | n4_2 | n4_3 | n4_4;
    }

    for (int i = 0; i < Bit_length; i++)
    {
        PIOC->PIO_CODR = n3_clear; // N3 clear
        PIOC->PIO_CODR = n4_clear; //// N4 clear
        delayMicroseconds(zero_time);
        PIOC->PIO_SODR = set[i]; // N3 set
        delayMicroseconds(pre_enable_time);
        PIOC->PIO_SODR = set_update[i];      // N4 set
        delayMicroseconds(pulse_width);      // N3 N4 pulse width setting
        PIOC->PIO_CODR = n4_clear;           // N4 clear
        delayMicroseconds(post_enable_time); // N3 N4 pulse delay setting
        PIOC->PIO_CODR = n3_clear;           // N3 clear
        delayMicroseconds(zero_time);
    }
}

void Feedforward(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core)
{
    int WL[5];

    for (int i = 0; i < 5; i++)
    {
        WL[i] = round(arg_X[i] * arg_pulseWidthWL[i]);
    }

    arg_core.setWLPulseWidth(WL[0], WL[1], WL[2], WL[3], WL[4]);

    state_switch(3);
    int N3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
    PIOC->PIO_SODR = N3; // N3 SET
    delayMicroseconds(read_set_time);
    read_scaling(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    // read_scaling_pulse(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    PIOC->PIO_CODR = N3; // N3 clear

    delayMicroseconds(20); // to separate N5 / N6 reads

    state_switch(5);
    int N1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
    PIOC->PIO_SODR = N1; // N1 SET
    delayMicroseconds(read_set_time);
    read_scaling(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    // read_scaling_pulse(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN6);
    PIOC->PIO_CODR = N1; // N1 clear

    arg_core.setADCvalue();

    for (int i = 0; i < 5; i++)
    {
        PRINTER(arg_core._ADCvalue[i]);
    }
};

void Backpropagation(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core)
{
    int WL[5];

    for (int i = 0; i < 5; i++)
    {
        WL[i] = round(arg_X[i] * arg_pulseWidthWL[i]);
    }

    arg_core.setWLPulseWidth(WL[0], WL[1], WL[2], WL[3], WL[4]);

    state_switch(4);
    int N3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
    PIOC->PIO_SODR = N3; // N3 SET
    delayMicroseconds(read_set_time);
    read_scaling(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    // read_scaling_pulse(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    PIOC->PIO_CODR = N3; // N3 Clear

    delayMicroseconds(20); // to separate N5 / N6 reads

    state_switch(6);
    int N1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
    PIOC->PIO_SODR = N1; // N1 SET
    delayMicroseconds(read_set_time);
    read_scaling(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN5);
    // read_scaling_pulse(WL[0], WL[1], WL[2], WL[3], WL[4], arg_core._ADCvalueN6);
    PIOC->PIO_CODR = N1; // N1 Clear

    // Calculate ADC N5 value - ADC N6 value and save to ADC value in the core
    arg_core.setADCvalue();

    for (int i = 0; i < 5; i++)
    {
        PRINTER(arg_core._ADCvalue[i]);
    }
};

void calculateLayerValues(double *arg_X, synapseArray5by5 &arg_core, layer &arg_layer)
{
    double var_midValueADCSum;
    for (int i = 0; i < 5; i++)
    {
        var_midValueADCSum = 0.0;
        for (int j = 0; j < 5; j++)
        {
            var_midValueADCSum += arg_X[j] * core._mid[i][j];
        }

        arg_layer.wsum[i] = 32 * (double(arg_core._ADCvalue[i]) - var_midValueADCSum) / (double(arg_core._standard[i][max_val]) - double(arg_core._standard[i][min_val]));

        // Activation Function - Sigmoid Function
        arg_layer.activationValue[i] = sigmoidActivFunc(arg_layer.wsum[i]);
        arg_layer.activationValue[i] = (arg_layer.activationValue[i] > DECISION_BOUNDARY) ? 1 : 0;
    }
};

layer calculateLayerValues(double *arg_X, synapseArray5by5 &arg_core)
{
    layer arg_layer;
    double var_midValueADCSum;
    for (int i = 0; i < 5; i++)
    {
        var_midValueADCSum = 0.0;
        for (int j = 0; j < 5; j++)
        {
            var_midValueADCSum += arg_X[j] * core._mid[i][j];
        }

        arg_layer.wsum[i] = 32 * (double(arg_core._ADCvalue[i]) - var_midValueADCSum) / (double(arg_core._standard[i][max_val]) - double(arg_core._standard[i][min_val]));

        // Activation Function - Sigmoid Function
        arg_layer.activationValue[i] = sigmoidActivFunc(arg_layer.wsum[i]);
        // arg_layer.activationValue[i] = (arg_layer.activationValue[i] > DECISION_BOUNDARY) ? 1 : 0;
    }
    return arg_layer;
}

double sigmoidActivFunc(double &x)
{
    return 1.0 / (1.0 + std::exp(-x));
}

double tanhActivFunc(double &x)
{
    double ex = std::exp(x);
    double enx = std::exp(-x);
    return (ex - enx) / (ex + enx);
}

double BinaryCrossentropy(double &y_hat, double &y)
{
    return -y * log(y_hat) + (1 - y) * log(1 - y_hat);
}

void SGDsetRegisterPotentiation(int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (abs(Q[j]) == 0)
            {
                N1[j][i] = 0;
            }
            else
            {
                N1[j][i] = ((rand() % 100000) + 1 <= Q[j]) ? 1 : 0; // j = 2, 3, 4 only
            }

            if (abs(P[j]) == 0)
            {
                N2[j][i] = 0;
            }
            else
            {
                N2[j][i] = ((rand() % 100000) + 1 <= P[j]) ? 1 : 0; // j = 2 only
            }
        }
    }
    Potentiation(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
};

void SGDsetRegisterDepression(int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (abs(Q[j]) == 0)
            {
                N3[j][i] = 0;
            }
            else
            {
                N3[j][i] = ((rand() % 100000) + 1 <= Q[j]) ? 1 : 0; // j = 2, 3, 4 only
            }

            if (abs(P[j]) == 0)
            {
                N4[j][i] = 0;
            }
            else
            {
                N4[j][i] = ((rand() % 100000) + 1 <= P[j]) ? 1 : 0; // j = 2 only
            }
        }
    }
    Depression(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
};

void printer(char *name, double value)
{
    Serial.print("name: ");
    Serial.print(name);
    Serial.print(" value: ");
    Serial.print(value);
    Serial.println("");
}