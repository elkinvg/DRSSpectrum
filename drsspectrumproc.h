#ifndef DRSSPECTRUMPROC_H
#define DRSSPECTRUMPROC_H
#include "drssignalproc.h"
#include "drsread.h"

#ifndef __MINGW32__
#ifdef QTCREATOR
#include <root/TH1F.h>
#include <root/TCanvas.h>
#include <root/TGraph.h>
#else
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraph.h>
#endif
#endif
class DRSSpectrumProc: public DRSread, public DRSSignalProc
{
public:
    DRSSpectrumProc();
    DRSSpectrumProc(bool bl_onlydetect);
//    DRSSpectrumProc(bool bl_autodetect ,bool bl_onlydetect);
    DRSSpectrumProc(unsigned int get_noise_min, unsigned int get_noise_max, unsigned int get_signal_min, unsigned int get_signal_max);
    ~DRSSpectrumProc();

#ifndef __MINGW32__
    TH1F *HistSpectr;
    TCanvas *canvas;
    TGraph *graph;
#endif
    void SetOutFileName(string);
    void SetNumberOfBins(unsigned int);
    void GetSpectumOffline(string filename , int type = DRS4);
    void CreateSimpleHist(std::vector<float>& signal);
    void CreatIntegralGraph(string filename, int type = DRS4);
    void DetectPolarityOfSignal();

    void SetFactor(float SetFactor, float SetShift);
    void GetFactor(float &GetFactor, float &GetShift);
    void setChannelMode(int chMode);
private:
    bool autodetect;
    bool onlydetect;
    bool autonameOutFile;
    int num_channels;
    int work_channel;
    short int work_ch_mode; // 0b0001 default - 0b[4ch 3ch 2ch 1ch]
    bool onlinemode;
    bool fileopenflag;
    float factor,shift;
    string resdir;
    void spectrinit();
    bool canvasflag,HistSpectrflag,graphflag;
    string OutFileName;
    string OutFileNameHist;
    string InputFileName;
    int NBins;
};

#endif // DRSSPECTRUMPROC_H
