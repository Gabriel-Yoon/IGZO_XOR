#include "exportFunction.h"

//--------------------------------------------------------------
exportFunction ::exportFunction()
{
}
//--------------------------------------------------------------
exportFunction ::~exportFunction()
{
}
//--------------------------------------------------------------
void exportFunction ::exportLossToTXT(std::vector<double> lossRecorder)
{
    // Open a file named loss.txt for writing
    std::ofstream outFile("loss.txt");

    // Check if the file is successfully opened
    if (outFile.is_open())
    {
        // Iterate through the vector and write each value to the file
        for (size_t i = 0; i < lossRecorder.size(); ++i)
        {
            outFile << lossRecorder[i] << "\n";
        }

        // Close the file
        outFile.close();

        std::cout << "loss.txt generated successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error opening file: loss.txt" << std::endl;
    }
}