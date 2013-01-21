#include "drsspectrumproc.h"
#include <iostream>
#include <vector>

DRSSpectrumProc::DRSSpectrumProc()
{
    spectrinit();
    autodetect = true;
    onlydetect = false;
}

DRSSpectrumProc::DRSSpectrumProc(bool bl_onlydetect)
{
    spectrinit();
    autodetect = true;
    onlydetect = bl_onlydetect;
}

//DRSSpectrumProc::DRSSpectrumProc(bool bl_autodetect, bool bl_onlydetect)
//{
//    autodetect = bl_autodetect;
//    if (autodetect) autodetect = bl_onlydetect;
//}

DRSSpectrumProc::DRSSpectrumProc(unsigned int get_noise_min, unsigned int get_noise_max, unsigned int get_signal_min, unsigned int get_signal_max)
{
    spectrinit();
    autodetect = false;
    onlydetect = false;
    noise_min = get_noise_min;
    noise_max = get_noise_max;
    signal_min = get_signal_min;
    signal_max = get_signal_max;
}

void DRSSpectrumProc::spectrinit()
{
    num_channels = work_channel = 1;
    NBins=1000;
    autonameOutFile=true;
    canvasflag=false;
    HistSpectrflag=false;
}

DRSSpectrumProc::~DRSSpectrumProc()
{
#ifndef __MINGW32__
    if (HistSpectrflag) {HistSpectr->Delete();HistSpectrflag=false;}
    if (canvasflag) {canvas->Destructor();canvasflag=false;}
#endif
}

void DRSSpectrumProc::SetOutFileName(string name)
{
    autonameOutFile=false;
    OutFileName = name;
}

void DRSSpectrumProc::SetNumberOfBins(unsigned int N)
{
    NBins = N;
}

void DRSSpectrumProc::GetSpectumOffline(string filename , int type)
{
    /**
     * @brief DRSFileReadStatusAndInfo
     * Считает спектр по одному выбранному каналу. Всего каналов может быть
     * от одного до четырёх.
     * (нет пока реализации ... для  n channel)
     */
    DRSFileReadStatusAndInfo(filename,type);
#ifndef __MINGW32__
    HistSpectr = new TH1F;
#endif
    unsigned short amp[1024];
    float times[1024];
    bool endframe;
    vector<float> signalval;
    float tmpsignal;
    float tmpmaxsignal,tmpminsignal;

    if (autodetect)
    {
        while(!endfile)
        {
            if(type==DRS4)
            {
                endframe = DRSGetFrame(&amp[0],&times[0]);
                /*if (autodetect) */autoSignalDetectKusskoff(&amp[0],endfile);
                if (endframe) continue;
            }
        }
        if (onlydetect)
        {
            cout << noise_min << " " << noise_max<< " " <<signal_min<< " " <<signal_max << /*" " <<signal_peak <<*/ endl;
            return;
        }
    }
    DRSFileSeekBegin();
    int tmpI=0;
    while(!endfile)
    {
        if(type==DRS4)
        {
            endframe = DRSGetFrame(&amp[0],&times[0]);
            tmpsignal = getsignal(&amp[0],&times[0]);
            signalval.push_back(tmpsignal);
            if (tmpI==0) tmpmaxsignal = tmpminsignal = tmpsignal;
            else
            {
                if (tmpsignal>tmpmaxsignal) tmpmaxsignal = tmpsignal;
                if (tmpsignal<tmpminsignal) tmpminsignal = tmpsignal;
            }
            tmpI++;
            if (endframe) continue;
        }
    }
    SetMinMaxValOfSignal(tmpminsignal,tmpmaxsignal);
#ifdef DEBUG
    cout << "Value of Integral min: " <<  tmpminsignal << " max: " << tmpmaxsignal << endl;
    cout <<"noise_min=" << noise_min << "  noise_max=" << noise_max<< "  signal_min=" <<signal_min<< "  signal_max=" <<signal_max << endl;
#endif

    string InFileName;
    if(autonameOutFile)
    {
        InFileName = filename.substr(0,filename.find_last_of("."));
        OutFileName = InFileName+".root";
    }
    CreateSimpleHist(signalval);
}

void DRSSpectrumProc::CreateSimpleHist(std::vector<float> &signal)
{

    pair<float,float> ValuesOfBorders;
    GetMinMaxValOfSignal(ValuesOfBorders);

    if (ValuesOfBorders.first>=ValuesOfBorders.second) return;
    float minBorder = ValuesOfBorders.first;
    float maxBorder = ValuesOfBorders.second;


#ifndef __MINGW32__

    canvas = new TCanvas("DRS","DRS",800,600);

    HistSpectr = new TH1F("DRS-hist","DRS-hist",NBins,minBorder,maxBorder);

    canvasflag=true;HistSpectrflag=true;

    int N = signal.size();
    for (int i=0;i<N;i++)
    {
        HistSpectr->Fill(signal[i]);
    }
    HistSpectr->Draw();
    canvas->SaveAs(OutFileName.c_str());
    //HistSpectr->SaveAs(OutFileName);

#endif
    cout << "Hist save as " << OutFileName << endl;
}

