#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "ADC_matrix.h"
#include "synapse_array_5by5.h"

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

#define MAX 300
#define BitLength 10

enum : int
{
    min,
    max
};

int WL0, WL1, WL2, WL3, WL4, X0, X1, target, P0, P1, P2, Q0, Q1, Q2, Q3, Q4; // input data를 의미하는 값, P1, Q1은 정수값으로 정의하면서 실제 rand와 대소비교를 하기 위한 역할
int ADC_h1_N5, ADC_h1_N6, ADC_h1_N56, ADC_h2_N5, ADC_h2_N6, ADC_h2_N56, ADC_o1_N5, ADC_o1_N6, ADC_o1_N56, ADC_b3_N5, ADC_b4_N5, ADC_b3_N6, ADC_b4_N6, ADC_b3_N56, ADC_b4_N56;

int DFF1, FB, CR, CON, CON2, HL_CHOP;
int ADC_mid[5][5];
int ADC_std[5][2]; // min, max

float X0_real, X1_real, target_real, error, p0, p1, p2, loss, q0, q1, q2, q3, q4, X2, X3, X4, Activation_input_h1, Activation_input_h2, Activation_output_h1, Activation_output_h2, Activation_input_o1, Activation_output_o1, Output_b1, Output_b1_plus, b3, b4, learning_rate_partial;
int N1[5][BitLength], N2[5][BitLength], N3[5][BitLength], N4[5][BitLength], N5[5][BitLength];

struct ADC_matrix
{
    int adc0;
    int adc1;
    int adc2;
    int adc3;
    int adc4;
};

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
    String _D0, _D1, _D2, _D3, _D4, _pulseWidth, _preEnableTime, _postEnableTime, _zeroTime, _epoch;
    String Input_string;

    initialize();

    if (Serial.available() > 0)
    {
        // SERIAL INPUT --------------------------------------------------
        // read and save "," position into index[]
        Input_string = Serial.readString();
        index[0] = Input_string.indexOf(",");
        for (int i = 0; i < VAR_NUM - 1; i++)
        {
            index[i + 1] = Input_string.indexOf(",", index[i] + 1);
        }

        // parse String input
        _D0 = Input_string.substring(0, index[0]);
        _D1 = Input_string.substring(index[0] + 1, index[1]);
        _D2 = Input_string.substring(index[1] + 1, index[2]);
        _D3 = Input_string.substring(index[2] + 1, index[3]);
        _D4 = Input_string.substring(index[3] + 1, index[4]);

        _pulseWidth = Input_string.substring(index[4] + 1, index[5]);
        _preEnableTime = Input_string.substring(index[5] + 1, index[6]);
        _postEnableTime = Input_string.substring(index[6] + 1, index[7]);
        _zeroTime = Input_string.substring(index[7] + 1, index[8]);
        _epoch = Input_string.substring(index[8] + 1, index[9]);

        int D0 = _D0.toInt();
        int D1 = _D1.toInt();
        int D2 = _D2.toInt();
        int D3 = _D3.toInt();
        int D4 = _D4.toInt();

        int pulseWidth = _pulseWidth.toInt();
        int preEnableTime = _preEnableTime.toInt();
        int postEnableTime = _postEnableTime.toInt();
        int zeroTime = _zeroTime.toInt();
        int epoch = _epoch.toInt(); // 여기서부터 epoch 수를 지정해서 for문을 돌려야 할 것으로 보임

        // get arguments and return
        auto getActivationOutput = [&](auto N5_adc0, auto N6_adc0, auto _queue2, auto _queue3)
        {
            Activation_input_h1 = (32 * (double(feedforward1_N5.adc0 - feedforward1_N6.adc0) - double(X0 * ADC_mid[0][0]) - double(X1 * ADC_mid[0][1]) - double(X2 * ADC_mid[0][2]))) / (double(ADC_standard_max1) - double(ADC_standard_min1)); // activation function의 input으로 들어가는 부분
            Activation_input_h2 = (32 * (double(feedforward1_N5.adc1 - feedforward1_N6.adc1) - double(X0 * ADC_mid10) - double(X1 * ADC_mid11) - double(X2 * ADC_mid12))) / (double(ADC_standard_max2) - double(ADC_standard_min2));             // activation function의 input으로 들어가는 부분
            Activation_output_h1 = 1.0 / (1.0 + exp(-Activation_input_h1));
            Activation_output_h2 = 1.0 / (1.0 + exp(-Activation_input_h2));

            // double minVal = std::numeric_limits<double>::max();
            // for (int i = size - 1; i >= 0; i--)
            // {
            //     if (time[i] != -1 && time[i] < minVal)
            //     {
            //         minVal = time[i];
            //         case_id = i;
            //         // if(time[i] == 0 || time[i] == 0.0000001 || time[i] <-100){
            //         //     std::cout << "wrong case_id = " << i << std::endl;
            //         // }
            //     }
            // }
            // return minVal;
        };

        // EPOCH --------------------------------------------------
        for (int i = 0; i < epoch; i++)
        {

            // Feed-Forward 1--------------------------------------------------
            // Pre-set Feed-Forward 1
            // What is D0 ?
            if (D0 = 10)
            {
                X0 = rand() % 2;
                X1 = rand{} % 2;
                X2 = 1;
                X0_real = (X0 == 0) ? 0 : 1;
                X1_real = (X1 == 0) ? 0 : 1;

                WL0 = round(6200 * X0_real);
                WL1 = round(6200 * X1_real);
                WL2 = 6200 * 1;
                WL3 = 0;
                WL4 = 0;

                target = X0 ^ X1;
                target_real = (target == 0) ? 0.25 : 0.75;
            }

            // N1, N3 line pulse occurence probability
            q0 = X0_real;
            q1 = X1_real;
            q2 = X2;

            struct ADC_matrix feedforward1_N5;
            state_switch(3);
            int n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
            PIOC->PIO_SODR = n3; // N3 SET
            delayMicroseconds(read_set_time);
            feedforward1_N5 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n3; // N3 clear

            struct ADC_matrix feedforward1_N6;
            state_switch(5);
            int n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
            PIOC->PIO_SODR = n1; // N1 SET
            delayMicroseconds(read_set_time);
            feedforward1_N6 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n1; // N1 clear

            Activation_input_h1 = (32 * (double(feedforward1_N5.adc0 - feedforward1_N6.adc0) - double(X0 * ADC_mid[0][0]) - double(X1 * ADC_mid[0][1]) - double(X2 * ADC_mid[0][2]))) / (double(ADC_standard_max1) - double(ADC_standard_min1)); // activation function의 input으로 들어가는 부분
            Activation_input_h2 = (32 * (double(feedforward1_N5.adc1 - feedforward1_N6.adc1) - double(X0 * ADC_mid10) - double(X1 * ADC_mid11) - double(X2 * ADC_mid12))) / (double(ADC_standard_max2) - double(ADC_standard_min2));             // activation function의 input으로 들어가는 부분
            Activation_output_h1 = 1.0 / (1.0 + exp(-Activation_input_h1));
            Activation_output_h2 = 1.0 / (1.0 + exp(-Activation_input_h2));

            // Feed-Forward 2--------------------------------------------------
            // Pre-set Feed-Forward 2

            /*

                         WL0--

                         WL1--

            b2        => WL2--                  (2,2)
            activation
            output h1 => WL3--                  (3,2)                   q3
            activation
            output h2 => WL4--                  (4,2)                   q4

                                |      |       |       |       |
                                adc0    adc1    adc2    adc3    adc4
            */

            X2 = 1;
            X3 = Activation_output_h1;
            X4 = Activation_output_h2;

            // N1, N3 line pulse occurence probability
            q3 = X3; // 4th row
            q4 = X4; // 5th row

            WL0 = 0;                // 2nd layer에 들어가는 input data이기 때문에 0이 되어야 함
            WL1 = 0;                // 2nd layer에 들어가는 input data이기 때문에 0이 되어야 함
            WL2 = 6200;             // 동일하게 bias 뉴런에 들어가는 고정값이 인가가 됨
            WL3 = round(6200 * X3); // 첫 번째 layer에서 나온 값을 반영하여 값을 결정해주어야 함(실험 데이터가 들어가는 부분)
            WL4 = round(6200 * X4); // 첫 번째 layer에서 나온 값을 반영하여 값을 결정해주어야 함(실험 데이터가 들어가는 부분)

            state_switch(3);
            n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
            PIOC->PIO_SODR = n3; // N3 SET
            delayMicroseconds(read_set_time);
            ADC_o1_N5 = feedforward_operation_2nd(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n3; // N3 Clear. 기존 코드에서 이 부분이 누락되어 있었는데 따라서 array 내 존재하는 모든 시냅스의 weight들이 weight 0으로 초기화 되었을 것으로 예상됨(N1과 N3가 동시에 켜져서)

            state_switch(5);
            n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
            PIOC->PIO_SODR = n1; // N1 SET
            delayMicroseconds(read_set_time);
            ADC_o1_N6 = feedforward_operation_2nd(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n1; // N1 Clear

            ADC_o1_N56 = ADC_o1_N5 - ADC_o1_N6;
            Activation_input_o1 = (32 * (double(ADC_o1_N56) - double(X2 * ADC_mid22) - double(X3 * ADC_mid23) - double(X4 * ADC_mid24))) / (double(ADC_standard_max3) - double(ADC_standard_min3));
            Activation_output_o1 = 1.0 / (1.0 + exp(-Activation_input_o1));                                 // sigmoid function?
            error = Activation_output_o1 - target_real;                                                     // error값을 (y-t)값으로 정의를 하였음
            loss = 100 * error * error;                                                                     //  선형회귀에서 잘못 용어 정의한 것을 수정하여 loss라고 정의를 하였음
            Output_b1 = amplification_factor * (Activation_output_o1) * (1 - Activation_output_o1) * error; // 해당 값은 backpropagate되는 양을 의미함. 이 때 ADC(0~1023 10bit)의 범위를 맞추기 위해서 amplification_factor를 곱해준다.
            Output_b1_plus = abs(Output_b1);

            // Back Propagation 1--------------------------------------------------
            // Pre-set Back Propagation
            WL0 = 0;
            WL1 = 0;
            WL2 = round(6200 * Output_b1_plus);
            WL3 = 0;
            WL4 = 0; // 앞서 정의된 Output_b1의 값에서 소자의 값을 반영한 특정값을 곱해준 만큼 역전(backpropagation)을 시켜줌(특정값은 feedforward 과정에서 곱해준 일정한 상수와 일치를 시켜주어야 한다)

            struct ADC_matrix backpropagation_N5;
            state_switch(4);
            n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
            PIOC->PIO_SODR = n3; // N3 SET
            delayMicroseconds(read_set_time);
            backpropagation_N5 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n3;                 // N3 Clear
            ADC_b3_N5 = backpropagation_N5.adc3; // 앞서 정의한 구조체의 x성분을 ADC_current_h1에 대입하는 과정
            ADC_b4_N5 = backpropagation_N5.adc4; // 앞서 정의한 구조체의 y성분을 ADC_current_h2에 대입하는 과정

            struct ADC_matrix backpropagation_N6;
            state_switch(6);
            n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
            PIOC->PIO_SODR = n1; // N1 SET
            delayMicroseconds(read_set_time);
            backpropagation_N6 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
            PIOC->PIO_CODR = n1; // N1 Clear
            ADC_b3_N6 = backpropagation_N6.adc3;
            ADC_b4_N6 = backpropagation_N6.adc4;

            ADC_b3_N56 = ADC_b3_N5 - ADC_b3_N6;
            ADC_b4_N56 = ADC_b4_N5 - ADC_b4_N6;

            b3 = (32 * (double(ADC_b3_N56) - double(Output_b1_plus) * double((ADC_mid23)))) / (double(ADC_standard_max4) - double(ADC_standard_min4)); // 이 값은 -Output_b1 ~ Output_b1의 값을 가지며, amplification factor가 곱해진 P 펄스 발생 확률과 연관이 됨
            b4 = (32 * (double(ADC_b4_N56) - double(Output_b1_plus) * double((ADC_mid24)))) / (double(ADC_standard_max5) - double(ADC_standard_min5)); // 이 값은 -Output_b1 ~ Output_b1의 값을 가지며, amplification factor가 곱해진 P 펄스 발생 확률과 연관이 됨
            p2 = error * (Activation_output_o1) * (1 - Activation_output_o1);                                                                          // 사실 p2는 backpropagate전에 이미 결정된 값이지만, p0와 p1과 함께 표기하기 위해 이 곳에 표기, 이 때 p0, p1은 case에 따라서 부호를 달리해야할 것으로 보임

            if (error >= 0)
            {
                p0 = double(b3 * Activation_output_h1) * (1 - Activation_output_h1) / double(amplification_factor);
                p1 = double(b4 * Activation_output_h2) * (1 - Activation_output_h2) / double(amplification_factor);
            }
            else
            {
                p0 = -1 * double(b3 * Activation_output_h1) * (1 - Activation_output_h1) / double(amplification_factor);
                p1 = -1 * double(b4 * Activation_output_h2) * (1 - Activation_output_h2) / double(amplification_factor);
            }

            learning_rate_partial = sqrt(learning_rate);
            P2 = round(6 * learning_rate_partial * 100000 * p2);
            Q2 = round(0.1667 * learning_rate_partial * 100000 * q2);
            Q3 = round(learning_rate_partial * 100000 * q3);
            Q4 = round(learning_rate_partial * 100000 * q4);

            if (P2 < 0)
            {
                for (int i = 0; i < BitLength; i++)
                {
                    N1[2][i] = ((rand() % 100000) + 1 <= Q2) ? 1 : 0;
                    N1[3][i] = ((rand() % 100000) + 1 <= Q3) ? 1 : 0;
                    N1[4][i] = ((rand() % 100000) + 1 <= Q4) ? 1 : 0;
                    N2[2][i] = ((rand() % 100000) + 1 <= -P2) ? 1 : 0; // why (-) value?
                }

                Potentiation(N1[0], N1[1], N1[2], N1[3], N1[4], N2[0], N2[1], N2[2], N2[3], N2[4], pulse_width, pre_enable_time, post_enable_time, zero_time);
            }
            else
            {
                for (int i = 0; i < BitLength; i++)
                {
                    N3[2][i] = ((rand() % 100000) + 1 <= Q2) ? 1 : 0;
                    N3[3][i] = ((rand() % 100000) + 1 <= Q3) ? 1 : 0;
                    N3[4][i] = ((rand() % 100000) + 1 <= Q4) ? 1 : 0;
                    N4[2][i] = ((rand() % 100000) + 1 <= P2) ? 1 : 0;
                }

                Depression(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulse_width, pre_enable_time, post_enable_time, zero_time);
            }

            P0 = round(12 * learning_rate_partial * 100000 * p0);
            P1 = round(12 * learning_rate_partial * 100000 * p1);
            Q0 = round(0.1667 * learning_rate_partial * 100000 * q0);
            Q1 = round(0.1667 * learning_rate_partial * 100000 * q1);
            Q2 = round(0.1667 * learning_rate_partial * 100000 * q2);
            Q3 = 0;
            Q4 = 0;
            P2 = 0;

            for (int i = 0; i < BitLength; i++)
            {
                N1[0][i] = ((rand() % 100000) + 1 <= Q0) ? 1 : 0;
                N1[1][i] = ((rand() % 100000) + 1 <= Q1) ? 1 : 0;
                N1[2][i] = ((rand() % 100000) + 1 <= Q2) ? 1 : 0;
                N1[3][i] = ((rand() % 100000) + 1 <= Q3) ? 1 : 0;
                N1[4][i] = ((rand() % 100000) + 1 <= Q4) ? 1 : 0;
                N2[0][i] = ((rand() % 100000) + 1 <= -P0) ? 1 : 0;
                N2[1][i] = ((rand() % 100000) + 1 <= -P1) ? 1 : 0;
            }
            Potentiation(N1[0], N1[1], N1[2], N1[3], N1[4], N2[0], N2[1], N2[2], N2[3], N2[4], pulse_width, pre_enable_time, post_enable_time, zero_time);

            for (int i = 0; i < BitLength; i++)
            {
                N3[0][i] = ((rand() % 100000) + 1 <= Q0) ? 1 : 0;
                N3[1][i] = ((rand() % 100000) + 1 <= Q1) ? 1 : 0;
                N3[2][i] = ((rand() % 100000) + 1 <= Q2) ? 1 : 0;
                N3[3][i] = ((rand() % 100000) + 1 <= Q3) ? 1 : 0;
                N3[4][i] = ((rand() % 100000) + 1 <= Q4) ? 1 : 0;
                N4[0][i] = ((rand() % 100000) + 1 <= -P0) ? 1 : 0;
                N4[1][i] = ((rand() % 100000) + 1 <= -P1) ? 1 : 0;
            }
            Depression(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulse_width, pre_enable_time, post_enable_time, zero_time);
        }
    }
}

void initialize()
{
    String D0_string, D1_string, D2_string, D3_string, D4_string, pulse_width_string, pre_enable_string, post_enable_string, zero_time_string, epoch_string;
    String Input_string;
    int index[VAR_NUM] = {
        0,
    }; // 이는 배열의 모든 요소를 0으로 초기화한다는 의미임

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < BitLength; j++)
        {
            N1[i][BitLength] = 0;
            N2[i][BitLength] = 0;
            N3[i][BitLength] = 0;
            N4[i][BitLength] = 0;
            N5[i][BitLength] = 0;
        }
    }

    // ADC mid value initialization
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            ADC_mid[i][j] = 20;
        }
    }
}

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

void read_scaling_pulse(int Ds0, int Ds1, int Ds2, int Ds3, int Ds4)
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
        delayMicroseconds(1000);
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

void Potentiation(int *UP0, int *UP1, int *UP2, int *UP3, int *UP4, int *UP_EN0, int *UP_EN1, int *UP_EN2, int *UP_EN3, int *UP_EN4)
{
    int c0, c1, up0, up1, up2, up3, up4, upen0, upen1, upen2, upen3, upen4 = 0;
    int set[MAX];
    int seten[MAX];
    for (int i = 0; i < MAX; i++)
    {
        if (UP0[i] == 1)
            up0 = 1 << 12; // 여기서의 up는 update potentiation의 줄임말로(고려대학교에서 처음 지정한 용어이기 때문에 조금 어색할 수 있습니다!) up0은 N1_0의 어레이 라인을 의미합니다.
        else
            up0 = 0;
        if (UP1[i] == 1)
            up1 = 1 << 13; // 역시나 up1은 N1_1 line을 의미합니다.
        else
            up1 = 0;
        if (UP2[i] == 1)
            up2 = 1 << 14; // up2는 N1_2 line을 의미
        else
            up2 = 0;
        if (UP3[i] == 1)
            up3 = 1 << 15; // up3은 N1_3 line을 의미
        else
            up3 = 0;
        if (UP4[i] == 1)
            up4 = 1 << 16; // up4는 N1_4 line을 의미
        else
            up4 = 0;
        if (UP_EN0[i] == 1)
            upen0 = 1 << 7; // upen은 update potentiation enable의 줄임말로(역시나 고려대학교 지정 언어로 조금 어색합니다!) N2를 의미합니다. 즉, upen0는 N2_0 line을 의미합니다.
        else
            upen0 = 0;
        if (UP_EN1[i] == 1)
            upen1 = 1 << 6; // upen1은 N2_1 line을 의미합니다.
        else
            upen1 = 0;
        if (UP_EN2[i] == 1)
            upen2 = 1 << 5; // upen2는 N2_2 line을 의미합니다.
        else
            upen2 = 0;
        if (UP_EN3[i] == 1)
            upen3 = 1 << 4; // upen3은 N2_3 line을 의미합니다.
        else
            upen3 = 0;
        if (UP_EN4[i] == 1)
            upen4 = 1 << 3; // upen4는 N2_4 line을 의미합니다. 각각 비트 이동연산자가 쓰인 우변이 실제 어레이에서는 어떤 라인에 해당되는지 이걸보면 더 이해가 잘 되시지 않을까 생각합니다!
        else
            upen4 = 0;

        set[i] = up0 | up1 | up2 | up3 | up4;
        seten[i] = upen0 | upen1 | upen2 | upen3 | upen4;
    }

    for (int i = 0; i < MAX; i++)
    {
        PIOC->PIO_SODR = set[i];   // up set
        delayMicroseconds(10);     // N1 N2 pulse delay setting, pulse rising edge
        PIOC->PIO_SODR = seten[i]; // up_en set
        delayMicroseconds(10);     // N1 N2 pulse width setting
        PIOC->PIO_CODR = seten[i]; // up_en clear
        delayMicroseconds(10);     // N1 N2 pulse delay setting. pulse falling edge
        PIOC->PIO_CODR = set[i];   // up clear
    }
}

void Depression(int *UD0, int *UD1, int *UD2, int *UD3, int *UD4, int *UD_EN0, int *UD_EN1, int *UD_EN2, int *UD_EN3, int *UD_EN4)
{
    int ud0, ud1, ud2, ud3, ud4, uden0, uden1, uden2, uden3, uden4 = 0;
    int set[MAX];
    int seten[MAX];
    for (int i = 0; i < MAX; i++)
    {
        if (UD0[i] == 1)
            ud0 = 1 << 17;
        else
            ud0 = 0;
        if (UD1[i] == 1)
            ud1 = 1 << 18;
        else
            ud1 = 0;
        if (UD2[i] == 1)
            ud2 = 1 << 19;
        else
            ud2 = 0;
        if (UD3[i] == 1)
            ud3 = 1 << 9;
        else
            ud3 = 0;
        if (UD4[i] == 1)
            ud4 = 1 << 8;
        else
            ud4 = 0;
        if (UD_EN0[i] == 1)
            uden0 = 1 << 2;
        else
            uden0 = 0;
        if (UD_EN1[i] == 1)
            uden1 = 1 << 1;
        else
            uden1 = 0;
        if (UD_EN2[i] == 1)
            uden2 = 1 << 23;
        else
            uden2 = 0;
        if (UD_EN3[i] == 1)
            uden3 = 1 << 24;
        else
            uden3 = 0;
        if (UD_EN4[i] == 1)
            uden4 = 1 << 25;
        else
            uden4 = 0;

        set[i] = ud0 | ud1 | ud2 | ud3 | ud4;
        seten[i] = uden0 | uden1 | uden2 | uden3 | uden4;
    }

    for (int i = 0; i < MAX; i++)
    {
        PIOC->PIO_SODR = set[i]; // ud set, SODR : off->on
        delayMicroseconds(10);
        PIOC->PIO_SODR = seten[i]; // ud_en set
        delayMicroseconds(10);
        PIOC->PIO_CODR = seten[i]; // ud_en clear, CODR : on->off
        delayMicroseconds(10);
        PIOC->PIO_CODR = set[i]; // ud clear
    }
}