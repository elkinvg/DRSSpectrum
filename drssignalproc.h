#ifndef DRSSIGNALPROC_H
#define DRSSIGNALPROC_H
//
//const int numsampl = 1024;
class DRSSignalProc
{
public:
    DRSSignalProc();
    bool kuskoff_amplitude;
    float factor;
    int posorneg; // positive = 1 ; negative = -1;
    float VoltMode;// = 0.5; //-0.5 <> 0.5 V = 0.5 ; 0 <> 1 = 0;
    unsigned int noise_min,noise_max,signal_min,signal_max,signal_peak;
    float getsignal(unsigned short *n_amplitudes, unsigned short *n_times);
    void peak(unsigned short *n_amplitudes);
    void autoSignalDetectKusskoff(unsigned short *k_amplitudes,int eventnum);
private:
    void init();
    float sumamp[numsampl]; // sum of the amplitudes
    int EventSN; // serial number of the event
    void autoSignalDetectKusskoffProc(int eventnum);
};

#endif // DRSSIGNALPROC_H
