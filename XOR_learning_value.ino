#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#define Max_read 120          // Read pulse set까지의 시간을 위하여
#define Bit_length 10         // update할 때의 timing을 맞추기 위해서 따로 정의
#define read_set_time 1       // [ms]
#define VAR_NUM 10            // 총 Input variable 수를 정의할 것
#define learning_rate 30      // 1st layer의 learning rate 정의
#define ADC_standard_max1 303 // ADC standard max, min은 연구노트에 각각 어떤 의미인지 정리해두었으며(32page), input 1일 때를 기준으로 standard를 설정하였음
#define ADC_standard_min1 -298
#define ADC_standard_max2 339
#define ADC_standard_min2 -324
#define ADC_standard_max3 325
#define ADC_standard_min3 -301
#define ADC_standard_max4 321
#define ADC_standard_min4 -290
#define ADC_standard_max5 297
#define ADC_standard_min5 -290
#define amplification_factor 8 // backpropagate될 때 gradient vanishing처럼 너무 값이 작아 ADC가 읽히지 않을 것을 대비하여 역전 시키기 전에 증폭 목적으로 특정 값을 곱해줌. 각 소자의 상황에 대비하여

int WL0, WL1, WL2, WL3, WL4, X0, X1, target, P0, P1, P2, Q0, Q1, Q2, Q3, Q4; // input data를 의미하는 값, P1, Q1은 정수값으로 정의하면서 실제 rand와 대소비교를 하기 위한 역할
int ADC_h1_N5, ADC_h1_N6, ADC_h1_N56, ADC_h2_N5, ADC_h2_N6, ADC_h2_N56, ADC_o1_N5, ADC_o1_N6, ADC_o1_N56, ADC_b3_N5, ADC_b4_N5, ADC_b3_N6, ADC_b4_N6, ADC_b3_N56, ADC_b4_N56;
int ADC_mid00, ADC_mid01, ADC_mid02, ADC_mid10, ADC_mid11, ADC_mid12, ADC_mid22, ADC_mid23, ADC_mid24; // input data 1일 들어왔을 때, 소자의 weight 0 상태를 ADC로 정의하기 위한 산술평균값
float X0_real, X1_real, target_real, error, p0, p1, p2, loss, q0, q1, q2, q3, q4, X2, X3, X4, Activation_input_h1, Activation_input_h2, Activation_output_h1, Activation_output_h2, Activation_input_o1, Activation_output_o1, Output_b1, Output_b1_plus, b3, b4, learning_rate_partial;
int N1_0[Bit_length], N2_0[Bit_length], N3_0[Bit_length], N4_0[Bit_length]; // update를 위한 배열 선언
int N1_1[Bit_length], N2_1[Bit_length], N3_1[Bit_length], N4_1[Bit_length]; // update를 위한 배열 선언
int N1_2[Bit_length], N2_2[Bit_length], N3_2[Bit_length], N4_2[Bit_length]; // update를 위한 배열 선언
int N1_3[Bit_length], N2_3[Bit_length], N3_3[Bit_length], N4_3[Bit_length]; // update를 위한 배열 선언
int N1_4[Bit_length], N2_4[Bit_length], N3_4[Bit_length], N4_4[Bit_length]; // update를 위한 배열 선언
int DFF1, FB, CR, CON, CON2, HL_CHOP;
struct ADC_matrix
{
  int adc0;
  int adc1;
  int adc2;
  int adc3;
  int adc4;
}; // 이 과정은 두 개의 ADC를 하나의 함수에서 동시에 출력하기 위한 구조체 선언 과정임. 5x1 등의 선형회귀에서는 사용되지 않았지만, 앞으로 deep한 학습을 위해서는 필수일 것으로 보임
// 여기에 전역변수로 지정할 여러 define 값들이나 인수들을 정의해야 함. 확실히 정해질 때마다 추가할 것
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
  pinMode(43, OUTPUT);
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
  ADC->ADC_MR |= 0x80;  // Mode FREERUN
  ADC->ADC_CR = 2;      // Start converter
  ADC->ADC_CHER = 0xF8; // Enabling All channels
  Serial.begin(115200);
}

void loop()
{
  String D0_string, D1_string, D2_string, D3_string, D4_string, pulse_width_string, pre_enable_string, post_enable_string, zero_time_string, epoch_string;
  String Input_string;
  int index[VAR_NUM] = {
      0,
  }; // 이는 배열의 모든 요소를 0으로 초기화한다는 의미임

  for (int i = 0; i < Bit_length; i++)
  {
    N1_0[i] = 0;
    N1_1[i] = 0;
    N1_2[i] = 0;
    N1_3[i] = 0;
    N1_4[i] = 0;
    N2_0[i] = 0;
    N2_1[i] = 0;
    N2_2[i] = 0;
    N2_3[i] = 0;
    N2_4[i] = 0;
    N3_0[i] = 0;
    N3_1[i] = 0;
    N3_2[i] = 0;
    N3_3[i] = 0;
    N3_4[i] = 0;
    N4_0[i] = 0;
    N4_1[i] = 0;
    N4_2[i] = 0;
    N4_3[i] = 0;
    N4_4[i] = 0;
  }

  if (Serial.available() > 0)
  {
    // input을 받고 ',' 위치 저장
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
    pulse_width_string = Input_string.substring(index[4] + 1, index[5]);
    pre_enable_string = Input_string.substring(index[5] + 1, index[6]);
    post_enable_string = Input_string.substring(index[6] + 1, index[7]);
    zero_time_string = Input_string.substring(index[7] + 1, index[8]);
    epoch_string = Input_string.substring(index[8] + 1, index[9]);
    // 공백 제거

    // 정수값으로의 변환
    int D0 = D0_string.toInt();
    int D1 = D1_string.toInt();
    int D2 = D2_string.toInt();
    int D3 = D3_string.toInt();
    int D4 = D4_string.toInt();
    int pulse_width = pulse_width_string.toInt();
    int pre_enable_time = pre_enable_string.toInt();
    int post_enable_time = post_enable_string.toInt();
    int zero_time = zero_time_string.toInt();
    int epoch = epoch_string.toInt(); // 여기서부터 epoch 수를 지정해서 for문을 돌려야 할 것으로 보임

    for (int i = 0; i < epoch; i++)
    {
      Serial.print("epoch = ");
      Serial.print(i + 1);
      if (D0 = 10)
      {
        X0 = rand() % 2; // 0 또는 1의 값을 가짐. 아두이노의 rand 함수를 이용하여 구현 할 예정
        X1 = rand() % 2; // 0 또는 1의 값을 가짐. 아두이노의 rand 함수를 이용하여 구현 할 예정
        X2 = 1;          // bias 뉴런의 역할을 하기 때문에 고정된 값

        if (X0 == 0)
        {
          X0_real = 0;
        }
        else
        {
          X0_real = 1;
        }
        if (X1 == 0)
        {
          X1_real = 0;
        }
        else
        {
          X1_real = 1;
        }

        WL0 = round(6200 * X0_real); // X0_real과 X1_real은 0 대신 다른 input data를 인가할 상황에 대비하여 인수로 정의해둠.
        WL1 = round(6200 * X1_real); //
        WL2 = 6200 * 1;              // bias를 의미하는 고정값이 들어가야 함;
        WL3 = 0;
        WL4 = 0;
        target = X0 ^ X1; // XOR

        if (target == 0)
        {
          target_real = 0.25;
        }
        else
        {
          target_real = 0.75;
        }
      }
      // Serial.print(", X0 = ");
      // Serial.print(X0 );
      // Serial.print(", X1 = ");
      // Serial.print(X1 );
      Serial.print(", X0_real = ");
      Serial.print(X0_real);
      Serial.print(", X1_real = ");
      Serial.print(X1_real);
      // Serial.print(", WL0 = ");
      // Serial.print(WL0 );
      // Serial.print(", WL1 = ");
      // Serial.print(WL1 );
      // Serial.print(", WL2 = ");
      // Serial.print(WL2 );
      Serial.print(", target = ");
      Serial.print(target);
      Serial.print(", target_real = ");
      Serial.print(target_real);
      q0 = X0_real; // q0는 array의 첫 번째 N1, N3 line에 들어가는 펄스 발생확률
      q1 = X1_real; // q1은 array의 두 번째 N1, N3 line에 들어가는 펄스 발생확률
      q2 = X2;      // q2는 array의 세 번째 N1, N3 line에 들어가는 펄스 발생확률
      ADC_mid00 = 20;
      ADC_mid01 = 20;
      ADC_mid02 = 20;
      ADC_mid10 = 20;
      ADC_mid11 = 20;
      ADC_mid12 = 20;
      ADC_mid22 = 20;
      ADC_mid23 = 20;
      ADC_mid24 = 20;
      struct ADC_matrix feedforward1_N5; // 첫 번째 feedfowrd 과정을 통해서 얻은 ADC의 구조체를 feedforward1이라 정의
      PIOB->PIO_CODR = 1 << 26;
      PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
      int n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
      PIOC->PIO_SODR = n3; // N3 SET
      delayMicroseconds(read_set_time);
      feedforward1_N5 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n3;              // N3 Clear
      ADC_h1_N5 = feedforward1_N5.adc0; // 앞서 정의한 구조체의 멤버에 접근하는 과정
      ADC_h2_N5 = feedforward1_N5.adc1; // 앞서 정의한 구조체의 멤버에 접근하는 과정
      Serial.print(", ADC_h1_N5 = ");
      Serial.print(ADC_h1_N5);
      Serial.print(", ADC_h2_N5 = ");
      Serial.print(ADC_h2_N5);

      struct ADC_matrix feedforward1_N6;
      PIOB->PIO_CODR = 1 << 26;
      PIOB->PIO_SODR = 1 << 25; // digitalWrite(HL_CHOP,HIGH)
      int n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
      PIOC->PIO_SODR = n1; // N1 SET
      delayMicroseconds(read_set_time);
      feedforward1_N6 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n1;              // N1 Clear
      ADC_h1_N6 = feedforward1_N6.adc0; // 구조체 멤버에 접근하여 인수로 대입하는 과정, 제대로 인수가 대입됨을 확인
      ADC_h2_N6 = feedforward1_N6.adc1; // 구조체 멤버에 접근하여 인수로 대입하는 과정, 제대로 인수가 대입됨을 확인
      Serial.print(", ADC_h1_N6 = ");
      Serial.print(ADC_h1_N6);
      Serial.print(", ADC_h2_N6 = ");
      Serial.print(ADC_h2_N6);

      ADC_h1_N56 = ADC_h1_N5 - ADC_h1_N6;
      ADC_h2_N56 = ADC_h2_N5 - ADC_h2_N6;
      Serial.print(", ADC_h1_N56 = ");
      Serial.print(ADC_h1_N56);
      Serial.print(", ADC_h2_N56 = ");
      Serial.print(ADC_h2_N56);
      Activation_input_h1 = (32 * (double(ADC_h1_N56) - double(X0 * ADC_mid00) - double(X1 * ADC_mid01) - double(X2 * ADC_mid02))) / (double(ADC_standard_max1) - double(ADC_standard_min1)); // activation function의 input으로 들어가는 부분
      Activation_input_h2 = (32 * (double(ADC_h2_N56) - double(X0 * ADC_mid10) - double(X1 * ADC_mid11) - double(X2 * ADC_mid12))) / (double(ADC_standard_max2) - double(ADC_standard_min2)); // activation function의 input으로 들어가는 부분
      Activation_output_h1 = 1.0 / (1.0 + exp(-Activation_input_h1));
      Activation_output_h2 = 1.0 / (1.0 + exp(-Activation_input_h2));
      Serial.print(", Activation_input_h1 = ");
      Serial.print(Activation_input_h1, 4);
      Serial.print(", Activation_input_h2 = ");
      Serial.print(Activation_input_h2, 4);
      Serial.print(", Activation_output_h1 = ");
      Serial.print(Activation_output_h1, 4);
      Serial.print(", Activation_output_h2 = ");
      Serial.print(Activation_output_h2, 4);
      X2 = 1;
      X3 = Activation_output_h1;
      X4 = Activation_output_h2;
      WL0 = 0;                // 2nd layer에 들어가는 input data이기 때문에 0이 되어야 함
      WL1 = 0;                // 2nd layer에 들어가는 input data이기 때문에 0이 되어야 함
      WL2 = 6200;             // 동일하게 bias 뉴런에 들어가는 고정값이 인가가 됨
      WL3 = round(6200 * X3); // 첫 번째 layer에서 나온 값을 반영하여 값을 결정해주어야 함(실험 데이터가 들어가는 부분)
      WL4 = round(6200 * X4); // 첫 번째 layer에서 나온 값을 반영하여 값을 결정해주어야 함(실험 데이터가 들어가는 부분)
      Serial.print(", WL2 = ");
      Serial.print(WL2);
      Serial.print(", WL3 = ");
      Serial.print(WL3);
      Serial.print(", WL4 = ");
      Serial.print(WL4);
      q3 = X3; // q3은 array의 네 번째 N1, N3 line에 들어가는 펄스 발생확률
      q4 = X4; // q4는 array의 다섯 번째 N1, N3 line에 들어가는 펄스 발생확률

      PIOB->PIO_CODR = 1 << 26;
      PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
      n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
      PIOC->PIO_SODR = n3; // N3 SET
      delayMicroseconds(read_set_time);
      ADC_o1_N5 = feedforward_operation_2nd(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n3; // N3 Clear. 기존 코드에서 이 부분이 누락되어 있었는데 따라서 array 내 존재하는 모든 시냅스의 weight들이 weight 0으로 초기화 되었을 것으로 예상됨(N1과 N3가 동시에 켜져서)
      Serial.print(", ADC_o1_N5 = ");
      Serial.print(ADC_o1_N5);

      PIOB->PIO_CODR = 1 << 26;
      PIOB->PIO_SODR = 1 << 25; // digitalWrite(HL_CHOP,HIGH)
      n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
      PIOC->PIO_SODR = n1; // N1 SET
      delayMicroseconds(read_set_time);
      ADC_o1_N6 = feedforward_operation_2nd(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n1; // N1 Clear
      Serial.print(", ADC_o1_N6 = ");
      Serial.print(ADC_o1_N6);

      ADC_o1_N56 = ADC_o1_N5 - ADC_o1_N6;
      Serial.print(", ADC_o1_N56 = ");
      Serial.print(ADC_o1_N56);
      Activation_input_o1 = (32 * (double(ADC_o1_N56) - double(X2 * ADC_mid22) - double(X3 * ADC_mid23) - double(X4 * ADC_mid24))) / (double(ADC_standard_max3) - double(ADC_standard_min3)); // activation function의 input으로 들어가는 부분2
      /*if(Activation_input_o1 > 2.5){
        Activation_input_o1 = 2.5;
      }
      if(Activation_input_o1 < -2.5){
        Activation_input_o1 = -2.5;
      }*/
      Activation_output_o1 = 1.0 / (1.0 + exp(-Activation_input_o1));
      error = Activation_output_o1 - target_real; // error값을 (y-t)값으로 정의를 하였음
      loss = 100 * error * error;                 //  선형회귀에서 잘못 용어 정의한 것을 수정하여 loss라고 정의를 하였음
      Serial.print(", Activation_input_o1 = ");
      Serial.print(Activation_input_o1, 4);
      Serial.print(", Activation_output_o1 = ");
      Serial.print(Activation_output_o1, 4);
      Serial.print(", error = ");
      Serial.print(error, 4);
      Serial.print(", loss = ");
      Serial.print(loss, 4);
      Output_b1 = amplification_factor * (Activation_output_o1) * (1 - Activation_output_o1) * error; // 해당 값은 backpropagate되는 양을 의미함. 이 때 ADC(0~1023 10bit)의 범위를 맞추기 위해서 amplification_factor를 곱해준다.
      Output_b1_plus = abs(Output_b1);                                                                // 현재 하드웨어 상에서 측정 중간에 외부 전압을 바꿀 수 없어서 일단 절댓값을 걸어주는 방식을 사용
      Serial.print(", Output_b1 = ");
      Serial.print(Output_b1, 4);
      Serial.print(", Output_b1_plus = ");
      Serial.print(Output_b1_plus, 4);
      WL0 = 0;
      WL1 = 0;
      WL2 = round(6200 * Output_b1_plus);
      WL3 = 0;
      WL4 = 0; // 앞서 정의된 Output_b1의 값에서 소자의 값을 반영한 특정값을 곱해준 만큼 역전(backpropagation)을 시켜줌(특정값은 feedforward 과정에서 곱해준 일정한 상수와 일치를 시켜주어야 한다)

      struct ADC_matrix backpropagation_N5;
      PIOB->PIO_SODR = 1 << 26;
      PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
      n3 = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
      PIOC->PIO_SODR = n3; // N3 SET
      delayMicroseconds(read_set_time);
      backpropagation_N5 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n3;                 // N3 Clear
      ADC_b3_N5 = backpropagation_N5.adc3; // 앞서 정의한 구조체의 x성분을 ADC_current_h1에 대입하는 과정
      ADC_b4_N5 = backpropagation_N5.adc4; // 앞서 정의한 구조체의 y성분을 ADC_current_h2에 대입하는 과정
      Serial.print(", ADC_b3_N5 = ");
      Serial.print(ADC_b3_N5);
      Serial.print(", ADC_b4_N5 = ");
      Serial.print(ADC_b4_N5);

      struct ADC_matrix backpropagation_N6;
      PIOB->PIO_SODR = 1 << 26;
      PIOB->PIO_SODR = 1 << 25; // digitalWrite(HL_CHOP,HIGH)
      n1 = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
      PIOC->PIO_SODR = n1; // N1 SET
      delayMicroseconds(read_set_time);
      backpropagation_N6 = feedforward_operation(WL0, WL1, WL2, WL3, WL4);
      PIOC->PIO_CODR = n1; // N1 Clear
      ADC_b3_N6 = backpropagation_N6.adc3;
      ADC_b4_N6 = backpropagation_N6.adc4;
      Serial.print(", ADC_b3_N6 = ");
      Serial.print(ADC_b3_N6);
      Serial.print(", ADC_b4_N6 = ");
      Serial.print(ADC_b4_N6);

      ADC_b3_N56 = ADC_b3_N5 - ADC_b3_N6;
      ADC_b4_N56 = ADC_b4_N5 - ADC_b4_N6;
      Serial.print(", ADC_b3_N56 = ");
      Serial.print(ADC_b3_N56);
      Serial.print(", ADC_b4_N56 = ");
      Serial.print(ADC_b4_N56);

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
      Serial.print(", b3 = ");
      Serial.print(b3, 4);
      Serial.print(", b4 = ");
      Serial.print(b4, 4);
      Serial.print(", p0 = ");
      Serial.print(p0, 4);
      Serial.print(", p1 ");
      Serial.print(p1, 4);
      Serial.print(", p2 = ");
      Serial.print(p2, 4);

      learning_rate_partial = sqrt(learning_rate);
      Serial.print(", learning_rate_partial = ");
      Serial.print(learning_rate_partial, 4);

      P2 = round(6 * learning_rate_partial * 100000 * p2);
      Serial.print(", P2 ="); // P4 값 출력
      Serial.print(P2);

      Q2 = round(0.1667 * learning_rate_partial * 100000 * q2);
      Serial.print(", Q2 ="); // Q2값 출력
      Serial.print(Q2);

      Q3 = round(learning_rate_partial * 100000 * q3);
      Serial.print(", Q3="); // Q3값 출력
      Serial.print(Q3);

      Q4 = round(learning_rate_partial * 100000 * q4);
      Serial.print(", Q4=");
      Serial.println(Q4);

      if (P2 < 0)
      {
        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q2)
          {
            N1_2[i] = 1;
          }
          else
          {
            N1_2[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q3)
          {
            N1_3[i] = 1;
          }
          else
          {
            N1_3[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q4)
          {
            N1_4[i] = 1;
          }
          else
          {
            N1_4[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= -P2)
          {
            N2_2[i] = 1;
          }
          else
          {
            N2_2[i] = 0;
          }
        }
        Potentiation(N1_0, N1_1, N1_2, N1_3, N1_4, N2_0, N2_1, N2_2, N2_3, N2_4, pulse_width, pre_enable_time, post_enable_time, zero_time);
      }
      else
      {

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q2)
          {
            N3_2[i] = 1;
          }
          else
          {
            N3_2[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q3)
          {
            N3_3[i] = 1;
          }
          else
          {
            N3_3[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= Q4)
          {
            N3_4[i] = 1;
          }
          else
          {
            N3_4[i] = 0;
          }
        }

        for (int i = 0; i < Bit_length; i++)
        {
          if ((rand() % 100000) + 1 <= P2)
          {
            N4_2[i] = 1;
          }
          else
          {
            N4_2[i] = 0;
          }
        }

        Depression(N3_0, N3_1, N3_2, N3_3, N3_4, N4_0, N4_1, N4_2, N4_3, N4_4, pulse_width, pre_enable_time, post_enable_time, zero_time);
      }

      P0 = round(12 * learning_rate_partial * 100000 * p0);
      Serial.print(", P0 ="); // P0 값 출력
      Serial.print(P0);

      P1 = round(12 * learning_rate_partial * 100000 * p1);
      Serial.print(", P1 ="); // P1 값 출력
      Serial.print(P1);

      Q0 = round(0.1667 * learning_rate_partial * 100000 * q0);
      Serial.print(", Q0 ="); // Q0값 출력
      Serial.print(Q0);

      Q1 = round(0.1667 * learning_rate_partial * 100000 * q1);
      Serial.print(", Q1 ="); // Q1값 출력
      Serial.print(Q1);

      Q2 = round(0.1667 * learning_rate_partial * 100000 * q2);
      Serial.print(", Q2 ="); // Q2값 출력
      Serial.print(Q2);

      Q3 = 0;
      Q4 = 0;
      P2 = 0;

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q0)
        {
          N1_0[i] = 1;
        }
        else
        {
          N1_0[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q1)
        {
          N1_1[i] = 1;
        }
        else
        {
          N1_1[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q2)
        {
          N1_2[i] = 1;
        }
        else
        {
          N1_2[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q3)
        {
          N1_3[i] = 1;
        }
        else
        {
          N1_3[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q4)
        {
          N1_4[i] = 1;
        }
        else
        {
          N1_4[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= -P0)
        {
          N2_0[i] = 1;
        }
        else
        {
          N2_0[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= -P1)
        {
          N2_1[i] = 1;
        }
        else
        {
          N2_1[i] = 0;
        }
      }

      Potentiation(N1_0, N1_1, N1_2, N1_3, N1_4, N2_0, N2_1, N2_2, N2_3, N2_4, pulse_width, pre_enable_time, post_enable_time, zero_time);

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q0)
        {
          N3_0[i] = 1;
        }
        else
        {
          N3_0[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q1)
        {
          N3_1[i] = 1;
        }
        else
        {
          N3_1[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q2)
        {
          N3_2[i] = 1;
        }
        else
        {
          N3_2[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q3)
        {
          N3_3[i] = 1;
        }
        else
        {
          N3_3[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= Q4)
        {
          N3_4[i] = 1;
        }
        else
        {
          N3_4[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= P0)
        {
          N4_0[i] = 1;
        }
        else
        {
          N4_0[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= P1)
        {
          N4_1[i] = 1;
        }
        else
        {
          N4_1[i] = 0;
        }
      }

      for (int i = 0; i < Bit_length; i++)
      {
        if ((rand() % 100000) + 1 <= P2)
        {
          N4_2[i] = 1;
        }
        else
        {
          N4_2[i] = 0;
        }
      }

      Depression(N3_0, N3_1, N3_2, N3_3, N3_4, N4_0, N4_1, N4_2, N4_3, N4_4, pulse_width, pre_enable_time, post_enable_time, zero_time);
    }
  }
}
struct ADC_matrix feedforward_operation(int ds0, int ds1, int ds2, int ds3, int ds4)
{
  int d1 = 1;
  int d2 = 1 << 1;
  int d3 = 1 << 2;
  int d4 = 1 << 3;
  int d5 = 1 << 6;
  int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
  int c = d1 | d2 | d3 | d4 | d5;
  int set[Max_read];
  int WL_0[Max_read], WL_1[Max_read], WL_2[Max_read], WL_3[Max_read], WL_4[Max_read];
  struct ADC_matrix result; // 함수 안에서의 반환을 할 구조체를 따로 정의하는 과정

  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds0)
      WL_0[i] = d1;
    else
      WL_0[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds1)
      WL_1[i] = d2;
    else
      WL_1[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds2)
      WL_2[i] = d3;
    else
      WL_2[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds3)
      WL_3[i] = d4;
    else
      WL_3[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds4)
      WL_4[i] = d5;
    else
      WL_4[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    set[i] = WL_0[i] | WL_1[i] | WL_2[i] | WL_3[i] | WL_4[i];
  }
  PIOB->PIO_CODR = 1 << 14; // CON
  PIOA->PIO_SODR = 1 << 19; // DS
  PIOB->PIO_SODR = 1 << 21; // CR

  for (int i = 0; i < Max_read; i++)
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
  // Serial.print(", ADC0 Value =");
  // Serial.print(ADC_0 / 4 );
  // Serial.print(", ADC1 Value =");
  // Serial.print(ADC_1 / 4);
  // Serial.print(", ADC2 Value =");
  // Serial.print(ADC_2 / 4);
  // Serial.print(", ADC3 Value =");
  // Serial.print(ADC_3 / 4);
  // Serial.print(", ADC4 Value =");
  // Serial.print(ADC_4 / 4);
  result.adc0 = ADC_0 / 4;
  result.adc1 = ADC_1 / 4;
  result.adc2 = ADC_2 / 4;
  result.adc3 = ADC_3 / 4;
  result.adc4 = ADC_4 / 4;
  return result;
}

int feedforward_operation_2nd(int ds0, int ds1, int ds2, int ds3, int ds4)
{
  int d1 = 1;
  int d2 = 1 << 1;
  int d3 = 1 << 2;
  int d4 = 1 << 3;
  int d5 = 1 << 6;
  int ADC_1, ADC_2, ADC_3, ADC_4, ADC_0;
  int c = d1 | d2 | d3 | d4 | d5;
  int set[Max_read];
  int WL_0[Max_read], WL_1[Max_read], WL_2[Max_read], WL_3[Max_read], WL_4[Max_read];

  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds0)
      WL_0[i] = d1;
    else
      WL_0[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds1)
      WL_1[i] = d2;
    else
      WL_1[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds2)
      WL_2[i] = d3;
    else
      WL_2[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds3)
      WL_3[i] = d4;
    else
      WL_3[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    if (i < ds4)
      WL_4[i] = d5;
    else
      WL_4[i] = 0;
  }
  for (int i = 0; i < Max_read; i++)
  {
    set[i] = WL_0[i] | WL_1[i] | WL_2[i] | WL_3[i] | WL_4[i]; // 100% 이해는 아니지만 비트 이동 연산자에 대해서 좀 더 공부해보면 이해할 듯?
  }
  PIOB->PIO_CODR = 1 << 14; // CON
  PIOA->PIO_SODR = 1 << 19; // DS
  PIOB->PIO_SODR = 1 << 21; // CR

  for (int i = 0; i < Max_read; i++)
  {
    PIOD->PIO_SODR = set[i]; // D SET
    PIOA->PIO_SODR = 1 << 7; // DFF1 CLK HIGH
    PIOA->PIO_CODR = 1 << 7; // DFF1 CLK LOW
    PIOD->PIO_CODR = c;      // D clear
    delayMicroseconds(1);    // read 한 번 돌때의 단위로 추정됨. 이는 random한 input data를 가지고 실험할 때는 더 줄여야 함.
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
  // Serial.print(", ADC0 Value =");
  // Serial.print(ADC_0 / 4 );
  // Serial.print(", ADC1 Value =");
  // Serial.print(ADC_1 / 4);
  // Serial.print(", ADC2 Value =");
  // Serial.print(ADC_2 / 4);
  // Serial.print(", ADC3 Value =");
  // Serial.print(ADC_3 / 4);
  // Serial.print(", ADC4 Value =");
  // Serial.print(ADC_4 / 4);
  return ADC_2 / 4;
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
    PIOC->PIO_CODR = n2_clear; //// N2 clear
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
