#include "drsspectrumproc.h"
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef unix
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TROOT.h>
#endif

//struct Spectr {
//    short noisemin;
//    short noisemax;
//};

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
    graphflag=false;
    onlinemode = false;
    fileopenflag = false;
    resdir="res/";
}

DRSSpectrumProc::~DRSSpectrumProc()
{
#ifndef __MINGW32__
    if (HistSpectrflag) {HistSpectr->Delete();HistSpectrflag=false;}
    if (canvasflag) {canvas->Destructor();canvasflag=false;}
    if (graphflag) {graph->Delete();graphflag=false;}
#endif
}

void DRSSpectrumProc::SetOutFileName(string name)
{
    autonameOutFile=false;
    if(name.find_last_of("/")==name.npos) OutFileName = resdir+name;
    else OutFileName = name;
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
    if (!fileopenflag)
    {
        DRSFileReadStatusAndInfo(filename,type);
        fileopenflag = true;
        num_channels = GetNumberOfChannels();
    }
#ifndef __MINGW32__
    HistSpectr = new TH1F;
#endif
    unsigned short amp[numsampl];
    float times[numsampl];
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
                if (num_channels==1)
                {
                    endframe = DRSGetFrame(&amp[0],&times[0]);
                    /*if (autodetect) */autoSignalDetectKusskoff(&amp[0],endfile);
                }
                if (endframe) continue;
            }
        }
        if (onlydetect)
        {
            cout << noise_min << " " << noise_max<< " " <<signal_min<< " " <<signal_max << /*" " <<signal_peak <<*/ endl;
            return;
        }
    }
    else
    {
        if(type==DRS4)
        {
            DRSFileSeekBegin();
            DetectPolarityOfSignal();
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
            if(tmpsignal!=-1111)
            {
                signalval.push_back(tmpsignal);
                if (tmpI==0) tmpmaxsignal = tmpminsignal = tmpsignal;

                else
                {
                    if (tmpsignal>tmpmaxsignal) tmpmaxsignal = tmpsignal;
                    if (tmpsignal<tmpminsignal) tmpminsignal = tmpsignal;
                }
                tmpI++;
            }

            if (endframe) continue;
        }
    }
    SetMinMaxValOfSignal(tmpminsignal,tmpmaxsignal);
#ifdef DEBUG
    cout << "Value of Integral min: " <<  tmpminsignal << " max: " << tmpmaxsignal << endl;
    cout <<"noise_min=" << noise_min << "  noise_max=" << noise_max<< "  signal_min=" <<signal_min<< "  signal_max=" <<signal_max << endl;
#endif

    string InFileName;
    string tmpdirect;
    if(autonameOutFile)
    {
        tmpdirect = filename.substr(0,filename.find_last_of("/")+1);
        resdir = tmpdirect+resdir;
#ifdef unix
        if (access(&resdir.c_str()[0],0)!=0) mkdir(&resdir.c_str()[0],0777);
#endif
        size_t of = filename.find_last_of(".",filename.npos);
        size_t fi = filename.find_last_of("/",filename.npos)+1;
        size_t len = of-fi;
        if (of<=fi) OutFileName = resdir+"tmp.root";
        else
        {
            InFileName = filename.substr(fi,len);
            OutFileName = resdir+InFileName+".root";
        }
    }
    //int Tmpsize = signalval.size();
    //cout << Tmpsize << " <<<<Tmpsize" << endl;

#ifndef __MINGW32__
    gROOT->ProcessLine("#include <vector>");
    vector<float> SummarySignal;
    getIntegralSignal(SummarySignal);
    TFile fData((resdir+InFileName+"-tree.root").c_str(),"RECREATE");
    TTree *SignalData = new TTree("DRSSignal","Signal DRS");
    SignalData->Branch("SummarySignal",&SummarySignal);
    SignalData->Branch("Sigfortree",&signalval);
    SignalData->Branch("Noise_min",&noise_min,"noise_min/s");
    SignalData->Branch("Noise_max",&noise_max,"noise_max/s");
    SignalData->Branch("Signal_min",&signal_min,"signal_min/s");
    SignalData->Branch("Signal_max",&signal_max,"signal_max/s");

    SignalData->Fill();
    SignalData->Write();
    fData.Close();
    //delete sigfortree;
#endif
    CreateSimpleHist(signalval);
    if (fileopenflag) DRSFileEnd();
}

void DRSSpectrumProc::CreateSimpleHist(std::vector<float> &signal)
{

    pair<float,float> ValuesOfBorders;
    GetMinMaxValOfSignal(ValuesOfBorders);

    if (ValuesOfBorders.first>=ValuesOfBorders.second) return;
    float minBorder = ValuesOfBorders.first;
    float maxBorder = ValuesOfBorders.second;


#ifndef __MINGW32__

    if (!canvasflag) { canvas = new TCanvas("DRS","DRS",800,600); canvasflag =true;}

    if(!HistSpectrflag) {HistSpectr = new TH1F("DRS-hist","DRS-hist",NBins,minBorder,maxBorder); HistSpectrflag=true;}


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

void DRSSpectrumProc::CreatIntegralGraph(string filename, int type)
{
    if (!fileopenflag)
    {
        DRSFileReadStatusAndInfo(filename,type);
        fileopenflag = true;
    }
    float IntegralSignal[numsampl];
    unsigned short amp[numsampl];
    float times[numsampl];
    bool endframe;
    while(!endfile)
    {
        if(type==DRS4)
        {
            endframe = DRSGetFrame(&amp[0],&times[0]);
            /*if (autodetect) */autoSignalDetectKusskoff(&amp[0],endfile);
            if (endframe) continue;
        }
    }
    DRSFileSeekBegin();
    autodetect = false;

    getIntegralSignal(&IntegralSignal[0]);

    string outgraphfile;
    string tmpdirect;
    float N[numsampl];
#ifndef __MINGW32__
    if (!canvasflag) { canvas = new TCanvas("DRS","DRS",800,600); canvasflag =true;}
    if (!graphflag)
    {
        tmpdirect = filename.substr(0,filename.find_last_of("/")+1);
        resdir = tmpdirect+resdir;

        if (access(&resdir.c_str()[0],0)!=0) mkdir(&resdir.c_str()[0],0777);

        size_t of = filename.find_last_of(".",filename.npos);
        size_t fi = filename.find_last_of("/",filename.npos)+1;
        size_t len = of-fi;
        outgraphfile = resdir+filename.substr(fi,len) + "-integral.png";
        for (int i=0;i<numsampl;i++) N[i]=(float)i;
        graph = new TGraph(numsampl,N,IntegralSignal);
        graph->Draw("ALP");
        canvas->SaveAs(outgraphfile.c_str());
    }
#endif
    if(fileopenflag)
    {
        DRSFileEnd();
        fileopenflag=false;
    }
}

void DRSSpectrumProc::DetectPolarityOfSignal()
{
    float amp;
    unsigned short int k_amplitudes[numsampl];
    float times[numsampl];
    float localSumAmp[numsampl];
    int localNumEvent=0;
    float integrall=0;
    bool endframe;
    while(!endfile)
    {

            if (num_channels==1)
            {
                endframe = DRSGetFrame(&k_amplitudes[0],&times[0]);
                for ( int i = 0; i < numsampl; i++ )
                {
                    if (localNumEvent==0) localSumAmp[i]=0;
                    amp =  (float)k_amplitudes[i]/65535.-VoltMode;
                    localSumAmp[i] += amp;
                }
                localNumEvent++;
                if (!endfile) continue;
                for(int i=16;i<numsampl-16;i++)
                {
                    integrall += localSumAmp[i]/localNumEvent;
                }
                if(integrall < 0) posorneg=-1;
                else posorneg=1;

            }
        }
}

