#ifndef FSR_H
#define FSR_H

#include <string>
#include <map>
#include <vector>


typedef uint32_t AU1;


struct FSRConstants {
    AU1 const0[4];
    AU1 const1[4];
    AU1 const2[4];
    AU1 const3[4];

    AU1 const0RCAS[4];

    uint32_t input_width;
    uint32_t input_height;
    uint32_t output_width;
    uint32_t output_height;
};

void initFSR(FSRConstants* fsrData, float sharpness);// sharpness in range [0-2], 0 is sharpest

uint32_t createFSRComputeProgramEAUS(const std::string& baseDir);
uint32_t createFSRComputeProgramRCAS(const std::string& baseDir);
uint32_t createBilinearComputeProgram(const std::string& baseDir);

#endif /* FSR_H */
