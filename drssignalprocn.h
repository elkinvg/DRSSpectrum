#ifndef DRSSIGNALPROCN_H
#define DRSSIGNALPROCN_H
#include <vector>
#include "drstype.h"

#include <sys/stat.h>

#ifdef DEBROOT
#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraph.h>
#else
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TROOT.h>
#include <root/TH1F.h>
#include <root/TCanvas.h>
#include <root/TGraph.h>
#endif

class DrsSignalProcN
{
public:
    DrsSignalProcN(DrsReadN* drsDataRead);
    DrsSignalProcN(DrsReadN* drsDataRead, unsigned short modes);
    DrsSignalProcN(DrsReadN *drsDataRead, unsigned int noise_min, unsigned int noise_max, unsigned int signal_min, unsigned int signal_max, unsigned short modes);

    void setVoltMode(float voltMode);
    void setWorkingChannelsMode(unsigned short work_ch_mode);

    void autoSignalDetectKusskoff(const vector<unsigned short> &n_amplitudes, unsigned short chNum, bool isNewCh, bool endfile);
    vector<float> getIntegralSignal();

    vector<std::pair<float, float> > getMinMaxValOfSignal();

    void setFactorAndShift(float factor, float shift);
    void getFactorAndShift(vector<float> &factor, vector<float>& shift);

    void setAmplitudeKuskoffMode(bool isAmp);
    void setSafetyMode();
    void setOutputDirectory(string outDir);

    // old drsSpectrumProc
    vector<vector<float> > getSpectumOffline();
    void setOutPngFileFlag();
    void rootProc(const vector<vector<float>>& signal);

private:
    void init();
    void spectrinit();

    void createHist(const vector<vector<float> > &signal, TFile *tFile);
    void deleteHist();
    void createTree(const vector<vector<float> > &signal, TFile *tFile);
    void createTreeTest(const vector<vector<float> > &signal, string outfile, TFile *tFile);
    void createSumHist(TFile *tFile);

    DrsReadN *drsRead;

    TH1F **histSpectr;
    TCanvas **canvas;
    TGraph **graph;
    TFile *fData;
    int nBins; // number of bins of histograms


    void autoSignalDetectKusskoffProc(int eventnum, unsigned short chNum);
    float getSignalWithKuskoffMethod(const vector<unsigned short> &n_amplitudes, const vector<float> &n_times, unsigned short chNum);
    void countSumAmpKusskoff(const vector<unsigned short> &n_amplitudes, unsigned short chNum);
    //void countSumSimple(const vector<unsigned short> &n_amplitudes, unsigned short chNum);

    float voltMode_; // =0.5 // -0.5 <> 0.5 V = 0.5 // 0 <> 1 = 0;
    bool voltModeN_; // thanks to kusskoff
    int posorneg; // positive = 1 ; negative = -1;
    vector<unsigned int> noise_min_,noise_max_,signal_min_,signal_max_,signal_peak_;
    unsigned short int numsampl;
    unsigned short int numofch;

    vector<float> minValOfSignal,maxValOfSignal;
    bool kuskoff_amplitude; // true is amplitude mode; false is charge mode
    int eventSN; // serial number of the event
    std::vector<float> sumAmp;

    bool autodetect;
    bool onlydetect;
    //bool autonameOutFile;
    bool single_ch_mode;
    //short int mask_use_ch; // 0b0001 for 1 0b0011 for 2 etc..
    short int work_ch_mode_; // 0b0001 default - 0b[4ch 3ch 2ch 1ch]
    bool onlinemode;
    bool safetymode;
    //bool fileopenflag;
    vector<float> factor_,shift_;

    bool canvasflag;
    bool fdataflag;
    bool outpngflag;

    string resdir;
    string resdirT;
    string workdir;
    string inputFileName;
    string outputFileName;
};

#endif // DRSSIGNALPROCN_H
