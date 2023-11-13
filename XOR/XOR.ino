#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#undef min
#undef max
#include <vector>
// #include <Vector.h>
#include "synapseArray5by5.h"
#include "neuronLayer.h"

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
#define Bit_length 300   // update할 때의 timing을 맞추기 위해서 따로 정의
#define VAR_NUM 10       // 총 Input variable 수를 정의할 것
#define learning_rate 1. // 1st layer의 learning rate 정의
#define amplification_factor 8
#define DECISION_BOUNDARY 0
#define PRINTER(name) printer(#name, (name))

enum : int
{
    min_val,
    max_val
};

int DFF1, FB, CR, CON, CON2, HL_CHOP;
int N1[5][Bit_length], N2[5][Bit_length], N3[5][Bit_length], N4[5][Bit_length], N5[5][Bit_length];

synapseArray5by5 core;
neuronLayer inputLayer;
neuronLayer hiddenLayer;
neuronLayer outputLayer;

double p[5]; // p0, p1, p2
double q[5]; // q0, q1, q2, q3, q4
int P1[5], P2[5];
int Q1[5], Q2[5];
int result[5];

double error, loss;
int correctAnswerTimes;

// Export
std::vector<double> ErrorEpochRecorder;
std::vector<double> LossEpochRecorder;
// std::vector<double> RightWrongEpochRecorder;

// FORWARD DECLARATION ************************************

void state_switch(int state);
void read_scaling_pulse(int ds0, int ds1, int ds2, int ds3, int ds4, int *result);
void Potentiation(int *N1_0, int *N1_1, int *N1_2, int *N1_3, int *N1_4, int *N2_0, int *N2_1, int *N2_2, int *N2_3, int *N2_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time);
void Depression(int *N3_0, int *N3_1, int *N3_2, int *N3_3, int *N3_4, int *N4_0, int *N4_1, int *N4_2, int *N4_3, int *N4_4, int pulse_width, int pre_enable_time, int post_enable_time, int zero_time);
void Feedforward(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core);
void Backpropagation(double *arg_X, int *arg_pulseWidthWL, synapseArray5by5 &arg_core);
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
    String set_num_string, update_num_string, read_period_string, read_delay_string, read_time_string, read_set_time_string;
    String Input_string;
    int epoch;
    int X1, X2;
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
        // Control Panel
        bool enable_GNDTEST = true;
        bool enable_noise = false;

        bool inputX1X2FourTypes = true;
        bool inputX1X2Random = false;
        bool inputX1X200 = false;
        bool inputX1X201 = false;
        bool inputX1X210 = false;
        bool inputX1X211 = false;

        bool randomWeight = false;

        // SERIAL INPUT ******************************************************
        Input_string = Serial.readString();
        index[0] = Input_string.indexOf(",");
        for (int i = 0; i < VAR_NUM - 1; i++)
        {
            index[i + 1] = Input_string.indexOf(",", index[i] + 1);
        }

        epoch_string = Input_string.substring(0, index[0]);

        pulse_width_string = Input_string.substring(index[0] + 1, index[1]); // micro
        pre_enable_string = Input_string.substring(index[1] + 1, index[2]);  // micro
        post_enable_string = Input_string.substring(index[2] + 1, index[3]); // micro

        set_num_string = Input_string.substring(index[3] + 1, index[4]);
        read_period_string = Input_string.substring(index[4] + 1, index[5]);

        zero_time_string = Input_string.substring(index[5] + 1, index[6]);     // micro
        read_delay_string = Input_string.substring(index[6] + 1, index[7]);    // micro
        read_time_string = Input_string.substring(index[7] + 1, index[8]);     // micro
        read_set_time_string = Input_string.substring(index[8] + 1, index[9]); // micro

        update_num_string = Input_string.substring(index[9] + 1, index[10]);

        // SERIAL PROMPT CODE EXAMPLE
        // 100, 1, 1, 1, 10, 100, 1, 1, 2, 1, 1000

        // 1
        epoch = epoch_string.toInt();
        // 2, 3, 4
        int pulseWidth = pulse_width_string.toInt();     // [micro s]
        int preEnableTime = pre_enable_string.toInt();   // [micro s]
        int postEnableTime = post_enable_string.toInt(); // [micro s]
        // 5, 6
        int setNum = set_num_string.toInt();         // usually x10
        int readPeriod = read_period_string.toInt(); // usually x100
        // 7, 8, 9
        int zeroTime = zero_time_string.toInt();      // [micro s]
        int readDelay = read_period_string.toInt();   // [x10 micro s]
        int readTime = read_period_string.toInt();    // [ms]
        int readSetTime = read_period_string.toInt(); // [ms]
        // 10
        int updateNum = update_num_string.toInt(); // usually x1000

        // #define read_set_time 1  // [ms]
        // #define read_delay 1     // [ms]

        // ****************************************************** SERIAL END

        // ARRAY INITIALIZE SAVE ADC VALUE ***************************************
        Serial.println("ARRAY INITIALIZE SAVE ADC VALUE ****************************************");
        int cycle_num = 1; // 초기화  cycle_num = read_period;
                           /*
                           cycle_num 지정
                           if (remainder_string == "T")
                           {
                               if (update_num % read_period == 0)
                               {
                                   cycle_num = read_period;
                               }
                               else
                               {
                                   cycle_num = 1;
                               }
                           }
                           else
                           {
                               cycle_num = 1;
                           }
                           */

        // 0. ADC ZEROING --------------------------------------------------------
        Serial.println("0. ADC ZEROING ---------------------------------------------------------");
        /*
        Condition Palette ===================
        pulseWidth           0 microsec
        preEnableTime        1 micro
        postEnableTime       1 micro
        set_num              10
        read_period          1000
        zeroTime             1 micro
        read_delay           1 x10 micro
        read_time            2000 micro
        read_set_time        1 mili
        update_num           3000
        =============== Condition Palette END
        */
        // 0-1. Condition Setup
        pulseWidth = 0;
        preEnableTime = 1;
        postEnableTime = 1;
        setNum = 10;
        readPeriod = 1000;
        zeroTime = 1;
        readDelay = 1;
        readTime = 2000;
        readSetTime = 1;
        updateNum = 3000;

        // 0-2. Potentiation and Depression to Zero
        for (int i = 0; i < setNum; i++)
        {
            for (int j = 0; j < cycle_num; j++)
            {
                for (int k = 0; k < updateNum; k++)
                {
                    for (int rowNum = 0; rowNum < 5; rowNum++)
                    {
                        Potentiation_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                }
                for (int k = 0; k < updateNum; k++)
                {
                    for (int rowNum = 0; rowNum < 5; rowNum++)
                    {
                        Depression_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                }
            }
        }

        // 0-3. Read at the very last moment. No need to check during zeroing process!
        for (int rowNum = 0; rowNum < 5; rowNum++)
        {
            Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
        }

        delay(50);
        Serial.println("-------------------------------------------------------- ADC ZEROING END");
        // -------------------------------------------------------- ADC ZEROING END

        // 1. ADC MIN, MAX VALUE EXTRACTION ---------------------------------------
        Serial.println("1. ADC MIN, MAX VALUE EXTRACTION ---------------------------------------");
        /*
        Condition Palette ===================
        pulseWidth           5 microsec
        preEnableTime        1 micro
        postEnableTime       1 micro
        set_num              10
        read_period          10
        zeroTime             1 micro
        read_delay           1 x10 micro
        read_time            2000 micro
        read_set_time        1 mili
        update_num           1000
        =============== Condition Palette END
        */
        for (int rowNum = 0; rowNum < 5; rowNum++)
        {
            // 0. Zeroing
            // Condition Setup
            pulseWidth = 0;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 3000;

            // Potentiation and Depression to Zero
            for (int i = 0; i < setNum; i++)
            {
                for (int j = 0; j < cycle_num; j++)
                {
                    for (int k = 0; k < updateNum; k++)
                    {
                        Potentiation_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                    for (int k = 0; k < updateNum; k++)
                    {
                        Depression_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                }
            }

            // 1. Potentiation and Depression
            // Condition Setup
            pulseWidth = 5;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 1000;

            // Potentiation and Depression for max and min
            for (int i = 0; i < setNum; i++)
            {
                for (int j = 0; j < cycle_num; j++)
                {
                    for (int k = 0; k < updateNum; k++)
                    {
                        Potentiation_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                    // Read the potentiation peak
                    Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
                    core.setADCmaxValue(rowNum);

                    for (int k = 0; k < updateNum; k++)
                    {
                        Depression_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                    // Read the depression peak
                    Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
                    core.setADCminValue(rowNum);
                }
            }

            // 2. Zeroing
            // Condition Setup
            pulseWidth = 0;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 3000;

            // Potentiation and Depression to Zero
            for (int i = 0; i < setNum; i++)
            {
                for (int j = 0; j < cycle_num; j++)
                {
                    for (int k = 0; k < updateNum; k++)
                    {
                        Potentiation_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                    for (int k = 0; k < updateNum; k++)
                    {
                        Depression_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                    }
                }
            }
        }
        delay(50);

        Serial.println("-------------------------------------- ADC MIN, MAX VALUE EXTRACTION END");
        // -------------------------------------- ADC MIN, MAX VALUE EXTRACTION END

        // 2. ADC ZERO VALUE EXTRACTION -------------------------------------------
        Serial.println("2. ADC ZERO VALUE EXTRACTION -------------------------------------------");
        /*
        Condition Palette ===================
        pulseWidth           0 microsec
        preEnableTime        1 micro
        postEnableTime       1 micro
        set_num              10
        read_period          10
        zeroTime             1 micro
        read_delay           1 x10 micro
        read_time            2000 micro
        read_set_time        1 mili
        update_num           1000
        =============== Condition Palette END
        */

        // Condition Setup
        pulseWidth = 0;
        preEnableTime = 1;
        postEnableTime = 1;
        setNum = 10;
        readPeriod = 1000;
        zeroTime = 1;
        readDelay = 1;
        readTime = 2000;
        readSetTime = 1;
        updateNum = 3000;

        for (int rowNum = 0; rowNum < 5; rowNum++)
        {
            for (int i = 0; i < setNum; i++)
            {
                for (int j = 0; j < cycle_num; j++)
                {
                    for (int k = 0; k < updateNum; k++)
                    {
                        Potentiation_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                        if ((k + 1) % readPeriod == j)
                        {
                            Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
                        }

                        Depression_6T(pulseWidth, preEnableTime, postEnableTime, zeroTime, rowNum);
                        if ((k + 1) % readPeriod == j)
                        {
                            Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
                        }
                    }
                }
            }
            // Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
            // core.setADCmidValue(rowNum);
        }

        for (int rowNum = 0; rowNum < 5; rowNum++)
        {
            Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
            core.setADCmidValue(rowNum);
            for (int colNum = 0; colNum < 5; colNum++)
            {
                core._weightADC[rowNum][colNum] = core._ADCvalueN5N6[colNum];
            }
        }

        delay(50);

        Serial.println("------------------------------------------ ADC ZERO VALUE EXTRACTION END");
        // 2. --------------------------------------- ADC ZERO VALUE EXTRACTION END

        // 3. GROUNDING TEST ------------------------------------------------------

        if (enable_GNDTEST)
        {
            Serial.println("3. GROUNDING TEST ------------------------------------------------------");

            // Condition Setup
            pulseWidth = 1;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 1000;

            // Grounding procedure and save ground values
            for (int rowNum = 0; rowNum < 5; rowNum++)
            {
                GroundAllCells(readTime, readSetTime, readDelay, rowNum, core);
                core.setADCgndValue(rowNum);
            }

            Serial.println("----------------------------------------------------- GROUNDING TEST END");
        }
        // ----------------------------------------------------- GROUNDING TEST END

        // 4. ARRAY NOISE -----------------------------------------------------

        if (enable_noise)
        {
            Serial.println("4. ARRAY NOISE ---------------------------------------------------------");

            // Condition Setup
            pulseWidth = 0;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 0;
            readSetTime = 1;
            updateNum = 1000;

            for (int rowNum = 0; rowNum < 5; rowNum++)
            {
                Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
                core.setADCnoiseValue(rowNum);
            }

            Serial.println("-------------------------------------------------------- ARRAY NOISE END");
        }
        // ---------------------------------------------------- ARRAY NOISE

        Serial.println("************************************ ARRAY INITIALIZE SAVE ADC VALUE END");
        // ************************************ ARRAY INITIALIZE SAVE ADC VALUE END

        Serial.println("RESULT BELOW");
        printADCminValue(core);
        printADCmaxValue(core);
        printADCmidValue(core);

        if (enable_GNDTEST)
        {
            printADCgndValue(core);
        }

        // XOR PROBLEM SOLVING ****************************************************
        Serial.println("XOR PROBLEM SOLVING ****************************************************");
        Serial.println("TESTING ADC VALUE RANGE ************************************************");
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

        // EPOCH ---------------------------------------------------------
        core.initialize();
        // core.setRange(100);
        if (randomWeight)
        {
            core.setRandomInitialWeight();
        }

        for (int i = 0; i < epoch; i++)
        {
            // Serial.println("// ---------------------------------------------------------");
            Serial.print("epoch = ");
            Serial.print(i + 1);

            core.refresh();
            inputLayer.refresh();
            hiddenLayer.refresh();
            outputLayer.refresh();

            X1 = rand() % 2;
            X2 = rand() % 2;

            // Input X1, X2 values
            if (inputX1X2FourTypes)
            {
                switch (i % 4)
                {
                case 0:
                    X1 = 0;
                    X2 = 0;
                    break;
                case 1:
                    X1 = 0;
                    X2 = 1;
                    break;
                case 2:
                    X1 = 1;
                    X2 = 0;
                    break;
                case 3:
                    X1 = 1;
                    X2 = 1;
                    break;
                default:
                    X1 = 0;
                    X2 = 0;
                    break;
                }
            }
            if (inputX1X2Random)
            {
                X1 = rand() % 2;
                X2 = rand() % 2;
            }
            if (inputX1X200)
            {
                X1 = 0;
                X2 = 0;
            }
            if (inputX1X201)
            {
                X1 = 0;
                X2 = 1;
            }
            if (inputX1X210)
            {
                X1 = 1;
                X2 = 0;
            }
            if (inputX1X211)
            {
                X1 = 1;
                X2 = 1;
            }

            int solution = X1 ^ X2;
            double solution_double = (solution == 0) ? 0.0 : 1.0;
            int answer;
            // Serial.print(" XOR Problem of ");
            // Serial.print(X1);
            // Serial.print(" and ");
            // Serial.println(X2);

            // double testValue[5] = {1, 0, 1, 0, 1};
            // inputLayer.setPreNeuronValues(testValue);
            inputLayer._preNeuronValue[0] = X1;
            inputLayer._preNeuronValue[1] = 0;
            inputLayer._preNeuronValue[2] = X2;
            inputLayer._preNeuronValue[3] = 0;
            inputLayer._preNeuronValue[4] = 0;

            // FF: input->hidden
            FeedForward(readTime, readSetTime, readDelay, inputLayer, core);
            // Serial.println("Obtained ADC Value after input->hidden FF");
            // printADCN5N6value(core);

            referencing_FF(inputLayer, core);
            // Serial.println("Referenced Value after input->hidden FF");
            // printLayerPostNeuronValue(inputLayer);

            inputLayer.sigmoidActivation();
            // Serial.println("Activation Value after input->hidden FF");
            // printLayerPostNeuronActivationValue(inputLayer);

            // FF: hidden->output
            // There is a hidden error here. the values that should be zero could have a possibility of not having 0
            hiddenLayer._preNeuronValue[0] = 0;
            hiddenLayer._preNeuronValue[1] = inputLayer._postNeuronActivationValue[0];
            hiddenLayer._preNeuronValue[2] = 0;
            hiddenLayer._preNeuronValue[3] = inputLayer._postNeuronActivationValue[2];
            hiddenLayer._preNeuronValue[4] = inputLayer._postNeuronActivationValue[4];

            // Serial.println("PreNeuron Value of hidden layer");
            // printLayerPreNeuronValue(hiddenLayer);

            FeedForward(readTime, readSetTime, readDelay, hiddenLayer, core);
            // Serial.println("Obtained ADC Value after hidden->output FF");
            // printADCN5N6value(core);

            referencing_FF(hiddenLayer, core);
            // Serial.println("Referenced Value after hidden->output FF");
            // printLayerPostNeuronValue(hiddenLayer);

            hiddenLayer.sigmoidActivation();
            // Serial.println("Activation Value after hidden->output FF");
            // printLayerPostNeuronActivationValue(hiddenLayer);

            outputLayer._preNeuronValue[0] = 0;
            outputLayer._preNeuronValue[1] = hiddenLayer._postNeuronActivationValue[1];
            outputLayer._preNeuronValue[2] = 0;
            outputLayer._preNeuronValue[3] = 0;
            outputLayer._preNeuronValue[4] = 0;

            // Serial.println("XOR Problem Solving FF Result");
            // Serial.print("output value : ");
            // Serial.println(outputLayer._preNeuronValue[1]);
            outputLayer.copyPreToPostNeuronValues();

            answer = (outputLayer._preNeuronValue[1] > 0.5) ? 1 : 0;
            if (answer == solution)
            {
                // Serial.print(" Correct ");
                // RightWrongEpochRecorder.push_back(1);
                correctAnswerTimes++;
            }
            // if (answer == solution)
            // {
            //
            // }
            // else
            // {
            //     Serial.print(" Wrong ");
            // }

            // Backpropagation -----------------------------------------------------

            // loss, error, accuracy calculation
            loss += MeanSquareError(outputLayer._preNeuronValue[1], solution_double);
            LossEpochRecorder.push_back(loss);
            // Serial.print("loss: ");
            // Serial.println(loss);

            error = outputLayer._preNeuronValue[1] - solution_double;
            Serial.print(" error: ");
            Serial.print(error);
            Serial.print(" ");
            Serial.print(" loss: ");
            Serial.print(loss);
            Serial.println(" ");
            ErrorEpochRecorder.push_back(error);

            get_dW2(hiddenLayer, core, error);
            // BP: output->hidden to get dH (=W2 x error)

            // BackPropagation setup

            outputLayer._postNeuronValue[0] = 0;
            outputLayer._postNeuronValue[1] = 0; // this should be changed to 'error'
            outputLayer._postNeuronValue[2] = 0;
            outputLayer._postNeuronValue[3] = 0;
            outputLayer._postNeuronValue[4] = 0;
            BackPropagation(readTime, readSetTime, readDelay, outputLayer, core);
            core.setADCvalueBPZero();

            outputLayer._postNeuronValue[0] = 0;
            outputLayer._postNeuronValue[1] = 1;
            outputLayer._postNeuronValue[2] = 0;
            outputLayer._postNeuronValue[3] = 0;
            outputLayer._postNeuronValue[4] = 0;
            BackPropagation(readTime, readSetTime, readDelay, outputLayer, core);
            core.setADCvalueBPOne();

            outputLayer._postNeuronValue[0] = 0;
            outputLayer._postNeuronValue[1] = error;
            outputLayer._postNeuronValue[2] = 0;
            outputLayer._postNeuronValue[3] = 0;
            outputLayer._postNeuronValue[4] = 0;
            BackPropagation(readTime, readSetTime, readDelay, outputLayer, core);
            core.setADCvalueBPTemp();

            // referencing_BP(outputLayer, core);
            set_dH(outputLayer, core, error); // only take 1 , 3 , 4
            // printdH(core);

            // dZ1 = dH x input.postActiv x(1 - input.postActiv);

            core._dZ[0] = core._dH[1] * hiddenLayer._preNeuronValue[1] * (1 - hiddenLayer._preNeuronValue[1]);
            core._dZ[2] = core._dH[3] * hiddenLayer._preNeuronValue[3] * (1 - hiddenLayer._preNeuronValue[3]);
            core._dZ[4] = core._dH[4] * hiddenLayer._preNeuronValue[4] * (1 - hiddenLayer._preNeuronValue[4]);

            core._dW1[0][0] = inputLayer._preNeuronValue[0] * core._dZ[0];
            core._dW1[0][2] = inputLayer._preNeuronValue[0] * core._dZ[2];
            core._dW1[0][4] = inputLayer._preNeuronValue[0] * core._dZ[4];
            core._dW1[1][0] = inputLayer._preNeuronValue[1] * core._dZ[0];
            core._dW1[1][2] = inputLayer._preNeuronValue[1] * core._dZ[2];
            core._dW1[1][4] = inputLayer._preNeuronValue[1] * core._dZ[4];

            // printdW1(core);
            // printdW2(core);

            // Get P[] Q[]
            for (int row_num = 0; row_num < 5; row_num++)
            {
                // P[i] = core._dW2[i] *
            }

            // Weight Update
            // 1. Potentiation and Depression
            // Condition Setup
            pulseWidth = 1;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 1000;

            // Weight Update 1 : Output -> Hidden Layer ----------------------------
            /* Weight Update

                    Q2[0] --

                    Q2[1] --           **(1,1)

                    Q2[2] --

                    Q2[3] --           **(3,1)

                    Q2[4] --           **(4,1)

                                |      |       |       |       |
                                P2[0]  P2[1]   P2[2]   P2[3]   P2[4]

            */

            for (int row_num = 0; row_num < 5; row_num++)
            {
                for (int col_num = 0; col_num < 5; col_num++)
                {
                    if (core._dW2[row_num][col_num] == 0)
                    {
                        continue;
                    }
                    // Serial.print("Q2 value would be: ");
                    // Serial.println(core._dW2[row_num][col_num]);

                    if (core._dW2[row_num][col_num] < 0)
                    {
                        core.Q2[row_num] = (-1) * (core._dW2[row_num][col_num]) * learning_rate;
                        core.P2[col_num] = 1;
                        for (int i = 0; i < Bit_length; i++)
                        {
                            if (i < core.Q2[row_num] * 100)
                            {
                                Potentiation_single_cell(pulseWidth, preEnableTime, postEnableTime, zeroTime, row_num, col_num);
                            }
                        }
                        // SGDsetRegisterPotentiation(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // GDsetRegisterPotentiation(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // SGDsetRegisterDepression(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                    }
                    else // core._dW2[row_num][col_num] > 0
                    {
                        core.Q2[row_num] = (core._dW2[row_num][col_num]) * learning_rate;
                        core.P2[col_num] = 1;
                        for (int i = 0; i < Bit_length; i++)
                        {
                            if (i < core.Q2[row_num] * 100)
                            {
                                Depression_single_cell(pulseWidth, preEnableTime, postEnableTime, zeroTime, row_num, col_num);
                            }
                        }
                        // SGDsetRegisterPotentiation(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // SGDsetRegisterDepression(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // GDsetRegisterDepression(core.P2, core.Q2, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                    }
                    core.clearP2();
                    core.clearQ2();
                }
            }

            // Weight Update 2 : Hidden -> Visible Layer -----------------------------
            /*

                        Q1[0] --     **              **              **

                        Q1[1] --

                        Q1[2] --     **              **              **

                        Q1[3] --

                        Q1[4] --

                                    |      |       |       |       |
                                    P1[0]  P1[1]   P1[2]   P1[3]   P1[4]

            */
            // inputLayer._preNeuronValue[row_num];

            for (int row_num = 0; row_num < 5; row_num++)
            {
                for (int col_num = 0; col_num < 5; col_num++)
                {
                    if (core._dW1[row_num][col_num] == 0)
                    {
                        continue;
                    }

                    if (core._dW1[row_num][col_num] < 0)
                    {
                        core.Q1[row_num] = (-1) * (core._dW1[row_num][col_num]) * learning_rate;
                        core.P1[col_num] = 1;
                        for (int i = 0; i < Bit_length; i++)
                        {
                            if (i < core.Q1[row_num] * 100)
                            {
                                Potentiation_single_cell(pulseWidth, preEnableTime, postEnableTime, zeroTime, row_num, col_num);
                            }
                        }
                        // SGDsetRegisterPotentiation(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // GDsetRegisterPotentiation(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // SGDsetRegisterDepression(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                    }
                    else
                    {
                        core.Q1[row_num] = (core._dW1[row_num][col_num]) * learning_rate;
                        core.P1[col_num] = 1;
                        for (int i = 0; i < Bit_length; i++)
                        {
                            if (i < core.Q1[row_num] * 100)
                            {
                                Depression_single_cell(pulseWidth, preEnableTime, postEnableTime, zeroTime, row_num, col_num);
                            }
                        }
                        // SGDsetRegisterPotentiation(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // SGDsetRegisterDepression(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                        // GDsetRegisterDepression(core.P1, core.Q1, pulseWidth, preEnableTime, postEnableTime, zeroTime);
                    }
                    core.clearP1();
                    core.clearQ1();
                }
            }

            // Condition Setup
            pulseWidth = 0;
            preEnableTime = 1;
            postEnableTime = 1;
            setNum = 10;
            readPeriod = 1000;
            zeroTime = 1;
            readDelay = 1;
            readTime = 2000;
            readSetTime = 1;
            updateNum = 3000;
            updateCurrentWeight(readTime, readSetTime, readDelay, core);

            // ADC -> digital weight modifying/dereferencing
            // zeroTime = 1;
            // readDelay = 1;
            // readTime = 2000;
            // readSetTime = 1;

            // for (int rowNum = 0; rowNum < 5; rowNum++)
            // {
            //     Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
            //     for (int colNum = 0; colNum < 5; colNum++)
            //     {
            //         core._weight[rowNum][colNum] += (core._ADCvalueN5N6[colNum] - core._weightADC[rowNum][colNum]) / core._range;
            //         core._weightADC[rowNum][colNum] = core._ADCvalueN5N6[colNum];
            //     }
            // }
            // core.setADCrefValue();

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
            // Serial.println("************************************************ XOR PROBLEM SOLVING END");
            // ************************************************ XOR PROBLEM SOLVING END
            // Print errors at the last moment
            if (i == epoch - 1)
            {
                // error print
                Serial.println("Error print ");
                for (int i = 0; i < epoch; i++)
                {
                    Serial.println(ErrorEpochRecorder[i] * 100);
                }
                Serial.println("Loss print ");
                for (int i = 0; i < epoch; i++)
                {
                    Serial.println(LossEpochRecorder[i] * 100);
                }

                Serial.print("Accuracy print : ");
                Serial.print(correctAnswerTimes / (double)epoch * 100);
                Serial.println("%");
            }
        }
        printDigitalWeight(core);
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

void read_scaling_single(int read_time, int read_delay, int read_row, synapseArray5by5 &arg_core)
{
    int ADC_0, ADC_1, ADC_2, ADC_3, ADC_4;

    int wl_0, wl_1, wl_2, wl_3, wl_4;
    if (read_row == 0)
    {
        wl_0 = 1;
    }
    else
    {
        wl_0 = 0;
    }
    if (read_row == 1)
    {
        wl_1 = 1 << 1;
    }
    else
    {
        wl_1 = 0;
    }
    if (read_row == 2)
    {
        wl_2 = 1 << 2;
    }
    else
    {
        wl_2 = 0;
    }
    if (read_row == 3)
    {
        wl_3 = 1 << 3;
    }
    else
    {
        wl_3 = 0;
    }
    if (read_row == 4)
    {
        wl_4 = 1 << 6;
    }
    else
    {
        wl_4 = 0;
    }
    /*
    int wl_0 = 1;
    int wl_1 = 1 << 1;
    int wl_2 = 1 << 2;
    int wl_3 = 1 << 3;
    int wl_4 = 1 << 6;
    */
    int wl = wl_0 | wl_1 | wl_2 | wl_3 | wl_4;

    PIOB->PIO_CODR = 1 << 14; // CON
    PIOA->PIO_SODR = 1 << 19; // DS
    PIOB->PIO_SODR = 1 << 21; // CR

    for (int i = 0; i < read_time; i++)
    {
        PIOD->PIO_SODR = wl;     // WL SET
        PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
        PIOD->PIO_CODR = wl;     // WL clear
        delayMicroseconds(1);    //
    }
    for (int i = 0; i < read_delay; i++)
    {
        PIOD->PIO_SODR = 0;      //
        PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
        PIOD->PIO_CODR = wl;     // WL clear
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

    arg_core.setADCvalueTemp(ADC_0 / 4, ADC_1 / 4, ADC_2 / 4, ADC_3 / 4, ADC_4 / 4);
    //  return ADC_0;
}

void read_scaling_multiple(int read_time, int read_delay, neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    int wl_0 = 1;
    int wl_1 = 1 << 1;
    int wl_2 = 1 << 2;
    int wl_3 = 1 << 3;
    int wl_4 = 1 << 6;
    int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
    int WL[read_time];
    int wl_clear = wl_0 | wl_1 | wl_2 | wl_3 | wl_4;
    int WL0[read_time], WL1[read_time], WL2[read_time], WL3[read_time], WL4[read_time];

    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._preNeuronValue[0] * read_time))
            WL0[i] = wl_0;
        else
            WL0[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._preNeuronValue[1] * read_time))
            WL1[i] = wl_1;
        else
            WL1[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._preNeuronValue[2] * read_time))
            WL2[i] = wl_2;
        else
            WL2[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._preNeuronValue[3] * read_time))
            WL3[i] = wl_3;
        else
            WL3[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._preNeuronValue[4] * read_time))
            WL4[i] = wl_4;
        else
            WL4[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        WL[i] = WL0[i] | WL1[i] | WL2[i] | WL3[i] | WL4[i];
    }

    PIOB->PIO_CODR = 1 << 14; // CON
    PIOA->PIO_SODR = 1 << 19; // DS
    PIOB->PIO_SODR = 1 << 21; // CR

    for (int i = 0; i < read_time; i++)
    {
        PIOD->PIO_SODR = WL[i];    // WL SET
        PIOA->PIO_SODR = 1 << 7;   // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7;   // DFF1 CLK LOW
        PIOD->PIO_CODR = wl_clear; // WL clear
        delayMicroseconds(1);      //
    }
    for (int i = 0; i < read_delay; i++)
    {
        PIOD->PIO_SODR = 0;        //
        PIOA->PIO_SODR = 1 << 7;   // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7;   // DFF1 CLK LOW
        PIOD->PIO_CODR = wl_clear; // WL clear
        delayMicroseconds(10);     //
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

    arg_core.setADCvalueTemp(ADC_0 / 4, ADC_1 / 4, ADC_2 / 4, ADC_3 / 4, ADC_4 / 4);
    //  return ADC_0;
}

void read_scaling_multiple_post(int read_time, int read_delay, neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    int wl_0 = 1;
    int wl_1 = 1 << 1;
    int wl_2 = 1 << 2;
    int wl_3 = 1 << 3;
    int wl_4 = 1 << 6;
    int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
    int WL[read_time];
    int wl_clear = wl_0 | wl_1 | wl_2 | wl_3 | wl_4;
    int WL0[read_time], WL1[read_time], WL2[read_time], WL3[read_time], WL4[read_time];

    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._postNeuronValue[0] * read_time))
            WL0[i] = wl_0;
        else
            WL0[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._postNeuronValue[1] * read_time))
            WL1[i] = wl_1;
        else
            WL1[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._postNeuronValue[2] * read_time))
            WL2[i] = wl_2;
        else
            WL2[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._postNeuronValue[3] * read_time))
            WL3[i] = wl_3;
        else
            WL3[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        if (i < round(arg_neurons._postNeuronValue[4] * read_time))
            WL4[i] = wl_4;
        else
            WL4[i] = 0;
    }
    for (int i = 0; i < read_time; i++)
    {
        WL[i] = WL0[i] | WL1[i] | WL2[i] | WL3[i] | WL4[i];
    }

    PIOB->PIO_CODR = 1 << 14; // CON
    PIOA->PIO_SODR = 1 << 19; // DS
    PIOB->PIO_SODR = 1 << 21; // CR

    for (int i = 0; i < read_time; i++)
    {
        PIOD->PIO_SODR = WL[i];    // WL SET
        PIOA->PIO_SODR = 1 << 7;   // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7;   // DFF1 CLK LOW
        PIOD->PIO_CODR = wl_clear; // WL clear
        delayMicroseconds(1);      //
    }
    for (int i = 0; i < read_delay; i++)
    {
        PIOD->PIO_SODR = 0;        //
        PIOA->PIO_SODR = 1 << 7;   // DFF1 CLK HIGH
        PIOA->PIO_CODR = 1 << 7;   // DFF1 CLK LOW
        PIOD->PIO_CODR = wl_clear; // WL clear
        delayMicroseconds(10);     //
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

    arg_core.setADCvalueTemp(ADC_0 / 4, ADC_1 / 4, ADC_2 / 4, ADC_3 / 4, ADC_4 / 4);
    //  return ADC_0;
}

void GroundAllCells(int readTime, int readSetTime, int readDelay, int rowNum, synapseArray5by5 &arg_core)
{
    int n1;
    int n2;
    int n3;

    n1 = (1 << 12); // row number 0
    n1 = (1 << 13); // row number 1
    n1 = (1 << 14); // row number 2
    n1 = (1 << 15); // row number 3
    n1 = (1 << 16); // row number 4

    // n2 = (1 << 7); // column number 0
    // n2 = (1 << 6); // column number 1
    // n2 = (1 << 5); // column number 2
    // n2 = (1 << 4); // column number 3
    // n2 = (1 << 3); // column number 4

    n3 = (1 << 17);
    n3 = (1 << 18);
    n3 = (1 << 19);
    n3 = (1 << 9);
    n3 = (1 << 8);

    state_switch(1);
    PIOA->PIO_CODR = 1 << 20; // VH VDDHAFL1
    PIOC->PIO_SODR = n1;      // N1 ON
    // PIOC->PIO_SODR = n2;   // N2 ON
    PIOC->PIO_SODR = n3; // N3 ON
    delay(10);           // Ground Capacitor for 10 milisec
    // PIOC->PIO_CODR = 1 << 2; // N1 clear
    // PIOC->PIO_CODR = 1 << 7; // N2 clear
    PIOC->PIO_CODR = n1; // N1 clear
    PIOC->PIO_CODR = n3; // N3 clear
    Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, arg_core);
    delay(50);
}

void Read_operation_forward_6T(int readTime, int readSetTime, int readDelay, int rowNum, synapseArray5by5 &arg_core)
{
    int n1;
    int n3;

    if (rowNum == 0)
    {
        n1 = (1 << 12);
        n3 = (1 << 17);
    }
    else if (rowNum == 1)
    {
        n1 = (1 << 13);
        n3 = (1 << 18);
    }
    else if (rowNum == 2)
    {
        n1 = (1 << 14);
        n3 = (1 << 19);
    }
    else if (rowNum == 3)
    {
        n1 = (1 << 15);
        n3 = (1 << 9);
    }
    else if (rowNum == 4)
    {
        n1 = (1 << 16);
        n3 = (1 << 8);
    }

    // Use N56
    state_switch(3);
    PIOC->PIO_SODR = n3; // N3 SET
    delayMicroseconds(readSetTime);
    read_scaling_single(readTime, readDelay, rowNum, arg_core);
    arg_core.setADCvalueN5();
    PIOC->PIO_CODR = n3; // N3 Clear

    delayMicroseconds(20); // to separate N5 / N6 reads

    // Serial.print("N6 - ");
    state_switch(5);
    PIOC->PIO_SODR = n1; // N1 SET
    delayMicroseconds(readSetTime);
    read_scaling_single(readTime, readDelay, rowNum, arg_core);
    arg_core.setADCvalueN6();
    PIOC->PIO_CODR = n1; // N1 Clear

    arg_core.setADCvalueN5N6();
}

void FeedForward(int readTime, int readSetTime, int readDelay, neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    int n1;
    int n3;

    if (arg_neurons._preNeuronValue[0] != 0)
    {
        n1 = (1 << 12);
        n3 = (1 << 17);
    }
    else if (arg_neurons._preNeuronValue[1] != 0)
    {
        n1 = (1 << 13);
        n3 = (1 << 18);
    }
    else if (arg_neurons._preNeuronValue[2] != 0)
    {
        n1 = (1 << 14);
        n3 = (1 << 19);
    }
    else if (arg_neurons._preNeuronValue[3] != 0)
    {
        n1 = (1 << 15);
        n3 = (1 << 9);
    }
    else if (arg_neurons._preNeuronValue[4] != 0)
    {
        n1 = (1 << 16);
        n3 = (1 << 8);
    }

    // Use N56
    state_switch(3);
    PIOC->PIO_SODR = n3; // N3 SET
    delayMicroseconds(readSetTime);
    read_scaling_multiple(readTime, readDelay, arg_neurons, arg_core);
    arg_core.setADCvalueN5();
    PIOC->PIO_CODR = n3; // N3 Clear

    delayMicroseconds(20); // to separate N5 / N6 reads

    // Serial.print("N6 - ");
    state_switch(5);
    PIOC->PIO_SODR = n1; // N1 SET
    delayMicroseconds(readSetTime);
    read_scaling_multiple(readTime, readDelay, arg_neurons, arg_core);
    arg_core.setADCvalueN6();
    PIOC->PIO_CODR = n1; // N1 Clear

    arg_core.setADCvalueN5N6();
}

void BackPropagation(int readTime, int readSetTime, int readDelay, neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    int n1;
    int n3;

    if (arg_neurons._postNeuronValue[0] != 0)
    {
        n1 = (1 << 12);
        n3 = (1 << 17);
    }
    else if (arg_neurons._postNeuronValue[1] != 0)
    {
        n1 = (1 << 13);
        n3 = (1 << 18);
    }
    else if (arg_neurons._postNeuronValue[2] != 0)
    {
        n1 = (1 << 14);
        n3 = (1 << 19);
    }
    else if (arg_neurons._postNeuronValue[3] != 0)
    {
        n1 = (1 << 15);
        n3 = (1 << 9);
    }
    else if (arg_neurons._postNeuronValue[4] != 0)
    {
        n1 = (1 << 16);
        n3 = (1 << 8);
    }

    // Use N56
    state_switch(4);
    PIOC->PIO_SODR = n3; // N3 SET
    delayMicroseconds(readSetTime);
    read_scaling_multiple_post(readTime, readDelay, arg_neurons, arg_core);
    arg_core.setADCvalueN5();
    PIOC->PIO_CODR = n3; // N3 Clear

    delayMicroseconds(20); // to separate N5 / N6 reads

    // Serial.print("N6 - ");
    state_switch(6);
    PIOC->PIO_SODR = n1; // N1 SET
    delayMicroseconds(readSetTime);
    read_scaling_multiple_post(readTime, readDelay, arg_neurons, arg_core);
    arg_core.setADCvalueN6();
    PIOC->PIO_CODR = n1; // N1 Clear

    arg_core.setADCvalueN5N6();
}

void Potentiation_6T(int pulse_width, int pre_enable_time, int post_enable_time, int zero_time, int row_num)
{ // only 0 index, 1번만
    int n1;
    int n2 = 1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3;

    if (row_num == 0)
    {
        n1 = (1 << 12);
    }
    else if (row_num == 1)
    {
        n1 = (1 << 13);
    }
    else if (row_num == 2)
    {
        n1 = (1 << 14);
    }
    else if (row_num == 3)
    {
        n1 = (1 << 15);
    }
    else if (row_num == 4)
    {
        n1 = (1 << 16);
    }

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    PIOC->PIO_CODR = n1; // N1 clear
    PIOC->PIO_CODR = n2; //// N2 clear --> for 010 like pulse input
    delayMicroseconds(zero_time);
    PIOC->PIO_SODR = n1; // N1 set
    delayMicroseconds(pre_enable_time);
    PIOC->PIO_SODR = n2; // N2 set
    delayMicroseconds(pulse_width);
    PIOC->PIO_CODR = n2; // N2 clear
    delayMicroseconds(post_enable_time);
    PIOC->PIO_CODR = n1; // N1 clear
    delayMicroseconds(zero_time);
}

void Potentiation_single_cell(int pulse_width, int pre_enable_time, int post_enable_time, int zero_time, int row_num, int col_num)
{
    int n1;
    int n2;

    if (row_num == 0)
    {
        n1 = (1 << 12);
    }
    else if (row_num == 1)
    {
        n1 = (1 << 13);
    }
    else if (row_num == 2)
    {
        n1 = (1 << 14);
    }
    else if (row_num == 3)
    {
        n1 = (1 << 15);
    }
    else if (row_num == 4)
    {
        n1 = (1 << 16);
    }

    if (col_num == 0)
    {
        n2 = (1 << 7);
    }
    else if (col_num == 1)
    {
        n2 = (1 << 6);
    }
    else if (col_num == 2)
    {
        n2 = (1 << 5);
    }
    else if (col_num == 3)
    {
        n2 = (1 << 4);
    }
    else if (col_num == 4)
    {
        n2 = (1 << 3);
    }

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    PIOC->PIO_CODR = n1; // N1 clear
    PIOC->PIO_CODR = n2; //// N2 clear --> for 010 like pulse input
    delayMicroseconds(zero_time);
    PIOC->PIO_SODR = n1; // N1 set
    delayMicroseconds(pre_enable_time);
    PIOC->PIO_SODR = n2; // N2 set
    delayMicroseconds(pulse_width);
    PIOC->PIO_CODR = n2; // N2 clear
    delayMicroseconds(post_enable_time);
    PIOC->PIO_CODR = n1; // N1 clear
    delayMicroseconds(zero_time);
}

void Depression_6T(int pulse_width, int pre_enable_time, int post_enable_time, int zero_time, int row_num)
{
    int n3;
    int n4 = 1 << 2 | 1 << 1 | 1 << 23 | 1 << 24 | 1 << 25;

    if (row_num == 0)
    {
        n3 = (1 << 17);
    }
    else if (row_num == 1)
    {
        n3 = (1 << 18);
    }
    else if (row_num == 2)
    {
        n3 = (1 << 19);
    }
    else if (row_num == 3)
    {
        n3 = (1 << 9);
    }
    else if (row_num == 4)
    {
        n3 = (1 << 8);
    }

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    PIOC->PIO_CODR = n3; // N3 clear
    PIOC->PIO_CODR = n4; //// N4 clear
    delayMicroseconds(zero_time);
    PIOC->PIO_SODR = n3; // N3 set
    delayMicroseconds(pre_enable_time);
    PIOC->PIO_SODR = n4; // N4 set
    delayMicroseconds(pulse_width);
    PIOC->PIO_CODR = n4; // N4 clear
    delayMicroseconds(post_enable_time);
    PIOC->PIO_CODR = n3; // N3 clear
    delayMicroseconds(zero_time);
}

void Depression_single_cell(int pulse_width, int pre_enable_time, int post_enable_time, int zero_time, int row_num, int col_num)
{
    int n3;
    int n4;

    if (row_num == 0)
    {
        n3 = (1 << 17);
    }
    else if (row_num == 1)
    {
        n3 = (1 << 18);
    }
    else if (row_num == 2)
    {
        n3 = (1 << 19);
    }
    else if (row_num == 3)
    {
        n3 = (1 << 9);
    }
    else if (row_num == 4)
    {
        n3 = (1 << 8);
    }

    if (col_num == 0)
    {
        n4 = (1 << 2);
    }
    else if (col_num == 1)
    {
        n4 = (1 << 1);
    }
    else if (col_num == 2)
    {
        n4 = (1 << 23);
    }
    else if (col_num == 3)
    {
        n4 = (1 << 24);
    }
    else if (col_num == 4)
    {
        n4 = (1 << 25);
    }

    PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
    PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)

    PIOC->PIO_CODR = n3; // N3 clear
    PIOC->PIO_CODR = n4; //// N4 clear
    delayMicroseconds(zero_time);
    PIOC->PIO_SODR = n3; // N3 set
    delayMicroseconds(pre_enable_time);
    PIOC->PIO_SODR = n4; // N4 set
    delayMicroseconds(pulse_width);
    PIOC->PIO_CODR = n4; // N4 clear
    delayMicroseconds(post_enable_time);
    PIOC->PIO_CODR = n3; // N3 clear
    delayMicroseconds(zero_time);
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

void SGDsetRegisterPotentiation(double *P, double *Q, int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int row_num = 0; row_num < 5; row_num++)
        {
            if (Q[row_num] == 0)

            {
                N1[row_num][i] = 0;
            }
            else
            {
                N1[row_num][i] = ((rand() % 100) + 1 <= Q[row_num] * 100) ? 1 : 0;
            }
            for (int col_num = 0; col_num < 5; col_num++)
            {
                if (P[col_num] == 0)
                {
                    N2[col_num][i] = 0;
                }
                else
                {
                    N2[col_num][i] = ((rand() % 100) + 1 <= P[col_num] * 100) ? 1 : 0;
                }
            }
        }

        // for (int j = 0; j < 5; j++)
        // {
        //     if (Q[j] == 0)
        //     {
        //         N1[j][i] = 0;
        //     }
        //     else
        //     {
        //         N1[j][i] = ((rand() % 100) + 1 <= Q[j] * 100) ? 1 : 0;
        //         // Serial.print(N1[j][i]);
        //     }

        //     if (P[j] == 0)
        //     {
        //         N2[j][i] = 0;
        //     }
        //     else
        //     {
        //         N2[j][i] = ((rand() % 100) + 1 <= P[j] * 100) ? 1 : 0;
        //     }
        // }
    }

    // // Print
    // Serial.print("N1 PRINT : ");
    // for (int j = 0; j < 5; j++)
    // {
    //     Serial.print("Q[j] = ");
    //     Serial.print(Q[j]);
    //     Serial.print(" ");
    //     Serial.print("j = ");
    //     Serial.print(j);
    //     Serial.print(" ");
    //     for (int i = 0; i < Bit_length; i++)
    //     {
    //         Serial.print(N1[j][i]);
    //         Serial.print(" ");
    //     }
    //     Serial.println(" ");
    // }

    // Serial.print("N2 PRINT : ");
    // for (int j = 0; j < 5; j++)
    // {
    //     Serial.print("P[j] = ");
    //     Serial.print(P[j]);
    //     Serial.print(" ");
    //     Serial.print("j = ");
    //     Serial.print(j);
    //     Serial.print(" ");
    //     for (int i = 0; i < Bit_length; i++)
    //     {
    //         Serial.print(N2[j][i]);
    //         Serial.print(" ");
    //     }
    //     Serial.println(" ");
    // }

    Potentiation(N1[0], N1[1], N1[2], N1[3], N1[4], N2[0], N2[1], N2[2], N2[3], N2[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
}

void GDsetRegisterPotentiation(double *P, double *Q, int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                if (Q[j] == 0 || P[k] == 0)
                {
                    N1[j][i] = 0;
                    N2[j][i] = 0;
                }
                else
                {
                    if (Q[j] * 100 < Bit_length)
                    {
                        Potentiation_single_cell(pulseWidth, preEnableTime, postEnableTime, zeroTime, j, k);
                        N1[j][i] = 1;
                        N2[j][i] = 1;
                    }
                    // N1[j][i] = ((rand() % 100) + 1 <= Q[j] * 100) ? 1 : 0;
                    // N2[j][i] = ((rand() % 100) + 1 <= P[j] * 100) ? 1 : 0;
                }
            }
        }
    }
    // Potentiation(N1[0], N1[1], N1[2], N1[3], N1[4], N2[0], N2[1], N2[2], N2[3], N2[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
}

void GDsetRegisterDepression(double *P, double *Q, int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (Q[j] == 0 || P[j] == 0)
            {
                N3[j][i] = 0;
                N4[j][i] = 0;
            }
            else
            {
                if (Q[j] * 100 < Bit_length)
                {
                    N3[j][i] = 1;
                    N4[j][i] = 1;
                }
                // N3[j][i] = ((rand() % 100) + 1 <= Q[j] * 100) ? 1 : 0;
                // N4[j][i] = ((rand() % 100) + 1 <= P[j] * 100) ? 1 : 0;
            }
        }
    }
    // Depression(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
}

void SGDsetRegisterDepression(double *P, double *Q, int pulseWidth, int preEnableTime, int postEnableTime, int zeroTime)
{
    for (int i = 0; i < Bit_length; i++)
    {
        for (int row_num = 0; row_num < 5; row_num++)
        {
            if (Q[row_num] == 0)
            {
                N3[row_num][i] = 0;
            }
            else
            {
                N3[row_num][i] = ((rand() % 100) + 1 <= Q[row_num] * 100) ? 1 : 0;
            }
            for (int col_num = 0; col_num < 5; col_num++)
            {
                if (P[col_num] == 0)
                {
                    N4[col_num][i] = 0;
                }
                else
                {
                    N4[col_num][i] = ((rand() % 100) + 1 <= P[col_num] * 100) ? 1 : 0;
                }
            }
        }

        // for (int j = 0; j < 5; j++)
        // {
        //     if (Q[j] == 0)
        //     {
        //         N3[j][i] = 0;
        //     }
        //     else
        //     {
        //         N3[j][i] = ((rand() % 100) + 1 <= Q[j] * 100) ? 1 : 0;
        //     }

        //     if (P[j] == 0)
        //     {
        //         N4[j][i] = 0;
        //     }
        //     else
        //     {
        //         N4[j][i] = ((rand() % 100) + 1 <= P[j] * 100) ? 1 : 0;
        //     }
        // }
    }

    // Print
    // Serial.print("N3 PRINT : ");
    // for (int j = 0; j < 5; j++)
    // {
    //     Serial.print("Q[j] = ");
    //     Serial.print(Q[j]);
    //     Serial.print(" ");
    //     Serial.print("j = ");
    //     Serial.print(j);
    //     Serial.print(" ");
    //     for (int i = 0; i < Bit_length; i++)
    //     {
    //         Serial.print(N3[j][i]);
    //         Serial.print(" ");
    //     }
    //     Serial.println(" ");
    // }

    // Serial.print("N4 PRINT : ");
    // for (int j = 0; j < 5; j++)
    // {
    //     Serial.print("P[j] = ");
    //     Serial.print(P[j]);
    //     Serial.print(" ");
    //     Serial.print("j = ");
    //     Serial.print(j);
    //     Serial.print(" ");
    //     for (int i = 0; i < Bit_length; i++)
    //     {
    //         Serial.print(N4[j][i]);
    //         Serial.print(" ");
    //     }
    //     Serial.println(" ");
    // }

    Depression(N3[0], N3[1], N3[2], N3[3], N3[4], N4[0], N4[1], N4[2], N4[3], N4[4], pulseWidth, preEnableTime, postEnableTime, zeroTime);
}

double BinaryCrossentropy(double &y_hat, double &y)
{
    return -y * log(y_hat) + (1 - y) * log(1 - y_hat);
}

double MeanSquareError(double &y_hat, double &y)
{
    return (y_hat - y) * (y_hat - y);
}

void referencing_FF(neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    // Purpose : get postNeuronValue
    // ADC - x *new_ref -adc_bias
    // adcvalueN5N6[i] - arg_core._ref[i][j] - arg_core._ADCbias[i]
    // divided by arg_core.range
    // would become wx+b value

    double tempArr[5] = {0};
    for (int col_num = 0; col_num < 5; col_num++)
    {
        tempArr[col_num] += arg_core._ADCvalueN5N6[col_num];
        for (int row_num = 0; row_num < 5; row_num++)
        {
            tempArr[col_num] += (-1) * arg_neurons._preNeuronValue[row_num] * arg_core._ref[row_num][col_num];
            // tempArr[col_num] += (-1) * arg_core._noise[row_num][col_num];

            // Add bias
            if (arg_core._targetBias[row_num][col_num] != 0)
            {
                tempArr[col_num] += arg_core._range * arg_core._targetBias[row_num][col_num];
            }
        }

        arg_neurons._postNeuronValue[col_num] = tempArr[col_num] / arg_core._range;
    }
}

void referencing_BP(neuronLayer &arg_neurons, synapseArray5by5 &arg_core)
{
    // Purpose : get postNeuronValue
    // ADC - x *new_ref -adc_bias
    // adcvalueN5N6[i] - arg_core._ref[i][j] - arg_core._ADCbias[i]
    // divided by arg_core.range
    // would become wx+b value

    double ADCgrid[5] = {0};
    double tempArr[5] = {0};
    for (int row_num = 0; row_num < 5; row_num++)
    {
        for (int col_num = 0; col_num < 5; col_num++)
        {
            if (arg_neurons._postNeuronValue[col_num] != 0 && arg_core._weight[row_num][col_num] != 0)
            {
                ADCgrid[row_num] = arg_core._ADCvalueBPOne[row_num] - arg_core._ADCvalueBPZero[row_num];

                arg_neurons._preNeuronValue[row_num] = (arg_core._ADCvalueBPTemp[row_num] - arg_core._ADCvalueBPZero[row_num]) / arg_neurons._postNeuronValue[col_num];
            }
        }
    }
}

void get_dW2(neuronLayer &arg_neurons, synapseArray5by5 &arg_core, double error)
{
    arg_core._dW2[1][1] = arg_neurons._preNeuronValue[1] * error;
    arg_core._dW2[3][1] = arg_neurons._preNeuronValue[3] * error;
    arg_core._dW2[4][1] = arg_neurons._preNeuronValue[4] * error;
}

void set_dH(neuronLayer &arg_neurons, synapseArray5by5 &arg_core, double error)
{
    // for (int row_num = 0; row_num < 5; row_num++)
    // {
    //     arg_core._dH[row_num] = arg_neurons._preNeuronValue[row_num];
    // }

    // For real answer,
    arg_core._dH[1] = arg_core._weight[1][1] * error;
    arg_core._dH[3] = arg_core._weight[3][1] * error;
    arg_core._dH[4] = arg_core._weight[4][1] * error;
}

void printer(char *name, double value)
{
    Serial.print("name: ");
    Serial.print(name);
    Serial.print(" value: ");
    Serial.print(value);
    Serial.println("");
}

void printADCN5N6value(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC N5N6 value print");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_core._ADCvalueN5N6[i]);
        Serial.print(" ");
    }
    Serial.println("");
    Serial.println("-------------------");
}

void printADCminValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC min value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._min[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printADCmaxValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC max value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._max[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printADCmidValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC mid value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._mid[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printADCgndValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC GND value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._gnd[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printADCnoiseValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC Noise value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._noise[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printADCrefValue(synapseArray5by5 &arg_core)
{
    Serial.println("-------------------");
    Serial.println("ADC REF value print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._ref[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println("-------------------");
}

void printDigitalWeight(synapseArray5by5 &arg_core)
{
    Serial.println("Digital Weight Print");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._weight[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
}

void printLayerPreNeuronValue(neuronLayer &arg_neurons)
{
    Serial.println("-------------------");
    Serial.println("Pre Neuron Values ");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_neurons._preNeuronValue[i]);
        Serial.print(" ");
    }
    Serial.println(" ");
    Serial.println("-------------------");
}

void printLayerPreNeuronActivationValue(neuronLayer &arg_neurons)
{
    Serial.println("-------------------");
    Serial.println("Pre Neuron Activation Values ");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_neurons._preNeuronActivationValue[i]);
        Serial.print(" ");
    }
    Serial.println(" ");
    Serial.println("-------------------");
}

void printLayerPostNeuronValue(neuronLayer &arg_neurons)
{
    Serial.println("-------------------");
    Serial.println("Post Neuron Values ");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_neurons._postNeuronValue[i]);
        Serial.print(" ");
    }
    Serial.println(" ");
    Serial.println("-------------------");
}

void printLayerPostNeuronActivationValue(neuronLayer &arg_neurons)
{
    Serial.println("-------------------");
    Serial.println("Post Neuron Activation Values ");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_neurons._postNeuronActivationValue[i]);
        Serial.print(" ");
    }
    Serial.println(" ");
    Serial.println("-------------------");
}

void printdW1(synapseArray5by5 &arg_core)
{
    Serial.println("dW1 values print: ");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._dW1[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println(" ");
}

void printdH(synapseArray5by5 &arg_core)
{
    Serial.println("dH values print: ");
    for (int i = 0; i < 5; i++)
    {
        Serial.print(arg_core._dH[i]);
        Serial.print(" ");
    }
    Serial.println(" ");
}

void printdW2(synapseArray5by5 &arg_core)
{
    Serial.println("dW2 values print: ");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Serial.print(arg_core._dW2[i][j]);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    Serial.println(" ");
}

void doubleArraySync(double *arr, double *temp)
{
    for (int i = 0; i < 5; i++)
    {
        arr[i] = temp[i];
    }
}

void print2Darray(int *P)
{
    Serial.print("Print2Darray");
    for (int i = 0; i < 5; i++)
    {
    }
}

void updateCurrentWeight(int readTime, int readSetTime, int readDelay, synapseArray5by5 &arg_core)
{
    for (int rowNum = 0; rowNum < 5; rowNum++)
    {
        Read_operation_forward_6T(readTime, readSetTime, readDelay, rowNum, core);
        arg_core.modifyCurrentWeight(rowNum);
        // for (int colNum = 0; colNum < 5; colNum++)
        // {
        //     arg_core._weightADC[rowNum][colNum] = arg_core._ADCvalueN5N6[colNum];
        // }
    }
}