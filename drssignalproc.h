#ifndef DRSSIGNALPROC_H
#define DRSSIGNALPROC_H
#include <vector>
#include <utility>
//
//const int numsampl = 1024;
class DRSSignalProc
{
public:
    DRSSignalProc();
    int numsampl;

    int posorneg; // positive = 1 ; negative = -1;
    float VoltMode;// = 0.5; //-0.5 <> 0.5 V = 0.5 ; 0 <> 1 = 0;
    unsigned int noise_min,noise_max,signal_min,signal_max,signal_peak;


    void SetModeIntegral(bool);
    void SetFactor(float SetFactorAValue, float SetFactorBValue);
    void GetMinMaxValOfSignal(std::pair<float,float>&);
    void SetMinMaxValOfSignal(float min,float max);
    void SetNumOfSamples(int numofsampes);

    float getsignal(unsigned short *n_amplitudes, float *n_times);
    void getIntegralSignal(float *int_signal);
    void getIntegralSignal(std::vector<float>&);
    void autoSignalDetectKusskoff(const unsigned short *k_amplitudes, bool endfile); // if end = true

private:
    float factor,factorB;
    float MinValOfSignal,MaxValOfSignal;
    bool kuskoff_amplitude; // true is amplitude mode; false is charge mode
    void init();
    //float sumamp[numsampl]; // sum of the amplitudes
    std::vector<float> sumamp;
    int EventSN; // serial number of the event
    void autoSignalDetectKusskoffProc(int eventnum);
};

#endif // DRSSIGNALPROC_H
