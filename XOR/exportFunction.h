#ifndef _EXPORT_FUNCTION_H_
#define _EXPORT_FUNCTION_H_

#include <vector>
#include <math.h>
#include <cmath>
#include <iostream>
#include <fstream>

//**************************************************************************************************************//
class exportFunction
{
    /*---------------------fields-----------------*/
public:
    /*---------------------methods----------------*/
public:
    exportFunction();
    ~exportFunction();

    void exportLossToTXT(std::vector<double> lossRecorder);
};
//**************************************************************************************************************//
#endif