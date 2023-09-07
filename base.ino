/*  int DFF1 = 31; //PA7 = D
 *  int DS = 42; //PA19

    int FB = 22; //PB26
    int CR = 52; //PB21
    int CON = 53; //PB14
    int HL_CHOP = 2; //PB25

    int D0 = 25; //PD0
    int D1 = 26; //PD1
    int D2 = 27; //PD2
    int D3 = 28; //PD3
    int D4 = 29; //PD6

    int UP0 = 51; //PC12
    int UP1 = 50; //PC13
    int UP2 = 49; //PC14
    int UP3 = 48; //PC15
    int UP4 = 47; //PC16
    /
    int UD0 = 46; //PC17
    int UD1 = 45; //PC18
    int UD2 = 44; //PC19
    int UD3 = 41; //PC9
    int UD4 = 40; //PC8

    int UP_EN0 = 39; //PC7
    int UP_EN1 = 38; //PC6
    int UP_EN2 = 37; //PC5
    int UP_EN3 = 36; //PC4
    int UP_EN4 = 35; //PC3

    int UD_EN0 = 34; //PC2
    int UD_EN1 = 33; //PC1
    int UD_EN2 = 7; //PC23
    int UD_EN3 = 6; //PC24
    int UD_EN4 = 5; //PC25

    int ADC_0 = A0;
    int ADC_1 = A1;
    int ADC_2 = A2;
    int ADC_3 = A3;
    int ADC_4 = A4;
*/

#define MAX 300

int UP0[MAX], UD0[MAX], UP_EN0[MAX], UD_EN0[MAX], D0;
int UP1[MAX], UD1[MAX], UP_EN1[MAX], UD_EN1[MAX], D1;
int UP2[MAX], UD2[MAX], UP_EN2[MAX], UD_EN2[MAX], D2;
int UP3[MAX], UD3[MAX], UP_EN3[MAX], UD_EN3[MAX], D3;
int UP4[MAX], UD4[MAX], UP_EN4[MAX], UD_EN4[MAX], D4;
int DFF1, FB, CR, CON, CON2, state, HL_CHOP;
String state_s, D_s0, D_s1, D_s2, D_s3, D_s4, UP_s0, UP_s1, UP_s2, UP_s3, UP_s4, UP_EN_s0, UP_EN_s1, UP_EN_s2, UP_EN_s3, UP_EN_s4, UD_s0, UD_s1, UD_s2, UD_s3, UD_s4, UD_EN_s0, UD_EN_s1, UD_EN_s2, UD_EN_s3, UD_EN_s4;

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
    ADC->ADC_CHER = 0xC0; // Enabling channels 6 and 7 (A0 and A1)
    Serial.begin(115200);
}

void loop()
{
    state = 0; // UP4[MAX] = {0, }; UD4[MAX] = {0, }; UP_EN4[MAX] = {0, }; UD_EN4[MAX] = {0, }; D4 = {0, }; // 초기화

    if (Serial.available() > 0)
    {
        state_s = Serial.readStringUntil(',');
        UP_s0 = Serial.readStringUntil(',');
        UP_s1 = Serial.readStringUntil(',');
        UP_s2 = Serial.readStringUntil(',');
        UP_s3 = Serial.readStringUntil(',');
        UP_s4 = Serial.readStringUntil(',');
        UD_s0 = Serial.readStringUntil(',');
        UD_s1 = Serial.readStringUntil(',');
        UD_s2 = Serial.readStringUntil(',');
        UD_s3 = Serial.readStringUntil(',');
        UD_s4 = Serial.readStringUntil(',');
        UP_EN_s0 = Serial.readStringUntil(',');
        UP_EN_s1 = Serial.readStringUntil(',');
        UP_EN_s2 = Serial.readStringUntil(',');
        UP_EN_s3 = Serial.readStringUntil(',');
        UP_EN_s4 = Serial.readStringUntil(',');
        UD_EN_s0 = Serial.readStringUntil(',');
        UD_EN_s1 = Serial.readStringUntil(',');
        UD_EN_s2 = Serial.readStringUntil(',');
        UD_EN_s3 = Serial.readStringUntil(',');
        UD_EN_s4 = Serial.readStringUntil(',');
        D_s0 = Serial.readStringUntil(',');
        D_s1 = Serial.readStringUntil(',');
        D_s2 = Serial.readStringUntil(',');
        D_s3 = Serial.readStringUntil(',');
        D_s4 = Serial.readStringUntil(',');

        state = state_s.toInt();
        D0 = D_s0.toInt();
        D1 = D_s1.toInt();
        D2 = D_s2.toInt();
        D3 = D_s3.toInt();
        D4 = D_s4.toInt();

        for (int i = 0; i < MAX; i++)
        {
            if (UP_s0[i] == '1')
                UP0[i] = 1;
            else
                UP0[i] = 0;
        }
        for (int i = 0; i < UP_s1.length(); i++)
        {
            if (UP_s1[i] == '1')
                UP1[i] = 1;
            else
                UP1[i] = 0;
        }
        for (int i = 0; i < UP_s2.length(); i++)
        {
            if (UP_s2[i] == '1')
                UP2[i] = 1;
            else
                UP2[i] = 0;
        }
        for (int i = 0; i < UP_s3.length(); i++)
        {
            if (UP_s3[i] == '1')
                UP3[i] = 1;
            else
                UP3[i] = 0;
        }
        for (int i = 0; i < UP_s4.length(); i++)
        {
            if (UP_s4[i] == '1')
                UP4[i] = 1;
            else
                UP4[i] = 0;
        }

        for (int i = 0; i < UD_s0.length(); i++)
        {
            if (UD_s0[i] == '1')
                UD0[i] = 1;
            else
                UD0[i] = 0;
        }
        for (int i = 0; i < UD_s1.length(); i++)
        {
            if (UD_s1[i] == '1')
                UD1[i] = 1;
            else
                UD1[i] = 0;
        }
        for (int i = 0; i < UD_s2.length(); i++)
        {
            if (UD_s2[i] == '1')
                UD2[i] = 1;
            else
                UD2[i] = 0;
        }
        for (int i = 0; i < UD_s3.length(); i++)
        {
            if (UD_s3[i] == '1')
                UD3[i] = 1;
            else
                UD3[i] = 0;
        }
        for (int i = 0; i < UD_s4.length(); i++)
        {
            if (UD_s4[i] == '1')
                UD4[i] = 1;
            else
                UD4[i] = 0;
        }

        for (int i = 0; i < UP_EN_s0.length(); i++)
        {
            if (UP_EN_s0[i] == '1')
                UP_EN0[i] = 1;
            else
                UP_EN0[i] = 0;
        }
        for (int i = 0; i < UP_EN_s1.length(); i++)
        {
            if (UP_EN_s1[i] == '1')
                UP_EN1[i] = 1;
            else
                UP_EN1[i] = 0;
        }
        for (int i = 0; i < UP_EN_s2.length(); i++)
        {
            if (UP_EN_s2[i] == '1')
                UP_EN2[i] = 1;
            else
                UP_EN2[i] = 0;
        }
        for (int i = 0; i < UP_EN_s3.length(); i++)
        {
            if (UP_EN_s3[i] == '1')
                UP_EN3[i] = 1;
            else
                UP_EN3[i] = 0;
        }
        for (int i = 0; i < UP_EN_s4.length(); i++)
        {
            if (UP_EN_s4[i] == '1')
                UP_EN4[i] = 1;
            else
                UP_EN4[i] = 0;
        }

        for (int i = 0; i < UD_EN_s0.length(); i++)
        {
            if (UD_EN_s0[i] == '1')
                UD_EN0[i] = 1;
            else
                UD_EN0[i] = 0;
        }
        for (int i = 0; i < UD_EN_s1.length(); i++)
        {
            if (UD_EN_s1[i] == '1')
                UD_EN1[i] = 1;
            else
                UD_EN1[i] = 0;
        }
        for (int i = 0; i < UD_EN_s2.length(); i++)
        {
            if (UD_EN_s2[i] == '1')
                UD_EN2[i] = 1;
            else
                UD_EN2[i] = 0;
        }
        for (int i = 0; i < UD_EN_s3.length(); i++)
        {
            if (UD_EN_s3[i] == '1')
                UD_EN3[i] = 1;
            else
                UD_EN3[i] = 0;
        }
        for (int i = 0; i < UD_EN_s4.length(); i++)
        {
            if (UD_EN_s4[i] == '1')
                UD_EN4[i] = 1;
            else
                UD_EN4[i] = 0;
        }

    } // 입력값 받기

    state_switch(state); // Update, Read, Backpropagation에 따른 연결 변화 여기를 보시면 실제 소자를 동작시키기 위해 PCB 및 상용소자에서 어떤 일이 일어나는지 잘 아실 것 같습니다.

    if (state == 1)
    {
        Serial.println("Update potential");
        update_potential_pulse(UP0, UP1, UP2, UP3, UP4, UP_EN0, UP_EN1, UP_EN2, UP_EN3, UP_EN4); // 고려대학교에서 potentiation을 potential로 표현하기는 해, 조금 혼동이 있을 것 같습니다. 어쨌든 최초 코드에서는 state가 1이면 potentiation을 수행하며, UP0 ~ UP_EN4까지 10개의 인수를 받아 함수가 실행됩니다.
        Serial.println("Done");
    }
    if (state == 2)
    {
        Serial.println("Update depression");
        update_depression_pulse(UD0, UD1, UD2, UD3, UD4, UD_EN0, UD_EN1, UD_EN2, UD_EN3, UD_EN4); // Depression도 마찬가지로 UD0부터 UDEN_4까지 10개의 인수를 받아 실행이 됩니다. 뒤에 설명이 있긴 하지만 UD는 N3를 의미하고 UD_EN는 N4를 의미하게 됩니다. 즉 10개의 인수는 N3_0~N4_4 line을 의미합니다.
        Serial.println("Done");
    }
    else if (state == 3 || state == 4)
    {
        Serial.println("Read operation start");
        int ud = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 9) | (1 << 8);
        PIOC->PIO_SODR = ud; // UD SET
        delay(5000);
        read_scaling_pulse(D0, D1, D2, D3, D4);
        PIOC->PIO_CODR = ud; // UD SET
        Serial.println("Done");
    }
    else if (state == 5 || state == 6)
    {
        Serial.println("Read operation start");
        int up = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);
        PIOC->PIO_SODR = up; // UP SET
        delay(5000);
        read_scaling_pulse(D0, D1, D2, D3, D4);
        PIOC->PIO_CODR = up; // UP SET
        Serial.println("Done");
    }
    else
    {
        delay(1);
    }
}

void state_switch(int state)
{ // Read 동작시에 CR핀 set할것, 조금씩 누적되는 경우 발생!
    if (state == 1)
    {                             // update potential
        PIOB->PIO_CODR = 1 << 26; // digitalWrite(FB,LOW)
        PIOB->PIO_CODR = 1 << 25; // digitalWrite(HL_CHOP,LOW)
    }
    else if (state == 2)
    { // update depression
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

void update_potential_pulse(int *UP0, int *UP1, int *UP2, int *UP3, int *UP4, int *UP_EN0, int *UP_EN1, int *UP_EN2, int *UP_EN3, int *UP_EN4)
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

void update_depression_pulse(int *UD0, int *UD1, int *UD2, int *UD3, int *UD4, int *UD_EN0, int *UD_EN1, int *UD_EN2, int *UD_EN3, int *UD_EN4)
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
        PIOC->PIO_SODR = set[i]; // ud set, 성원님 이걸 보니 SODR이 set 즉, off에서 on 시키는 과정으로 보이네요.
        delayMicroseconds(10);
        PIOC->PIO_SODR = seten[i]; // ud_en set
        delayMicroseconds(10);
        PIOC->PIO_CODR = seten[i]; // ud_en clear, 역시나 이걸 보니 CODR이 off시키는 과정으로 생각됩니다.
        delayMicroseconds(10);
        PIOC->PIO_CODR = set[i]; // ud clear
    }
}