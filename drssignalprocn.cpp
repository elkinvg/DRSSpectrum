#include "drssignalprocn.h"
#include <math.h>
#include <chrono>

//#pragma link C++ class vector<vector<float > >+;

DrsSignalProcN::DrsSignalProcN(DrsReadN *drsDataRead)
{
    drsRead = drsDataRead;
    safetymode = false;
    voltModeN_ = false;

    init();
    spectrinit();
    autodetect = true;
    onlydetect = false;
    single_ch_mode = false;
}

DrsSignalProcN::DrsSignalProcN(DrsReadN *drsDataRead,unsigned short int modes)
{
    /**
      \brief 1st bin - is onlydetect mode 2nd bin - is safety mode
      */

    if (modes & 1) onlydetect = true;
    if (modes & 2) safetymode = true;
    if (modes & 4) voltModeN_ = true; else voltModeN_ = false;

    drsRead = drsDataRead;
    init();
    spectrinit();
    autodetect = true;
    single_ch_mode = false;
}

DrsSignalProcN::DrsSignalProcN(DrsReadN *drsDataRead, unsigned int noise_min, unsigned int noise_max, unsigned int signal_min, unsigned int signal_max, unsigned short modes)
{
    drsRead = drsDataRead;

    if (modes & 2) safetymode = true; else safetymode = false;
    if (modes & 4) voltModeN_ = true; else voltModeN_ = false;

    init();
    spectrinit();
    autodetect = false;
    single_ch_mode = false;
    if (noise_min > numsampl || noise_max > numsampl || signal_min > numsampl || signal_max >numsampl )
    {
        cerr << RED_SH << " Values of more than number of samples(" << numsampl << ") " << ENDCOLOR << endl;
        exit(1);
    }
    if (noise_min >= noise_max || signal_min >= signal_max || noise_max > signal_min)
    {
        cerr << RED_SH << " Incorrect values" << ENDCOLOR << endl;
        exit(1);
    }
    for (int i=0;i<numofch;i++)
    {
        noise_min_[i]=noise_min;
        noise_max_[i]=noise_max;
        signal_min_[i]=signal_min;
        signal_max_[i]=signal_max;
    }
}

void DrsSignalProcN::init()
{
    posorneg = -1;

    voltMode_ = 0.5;
    kuskoff_amplitude = false;

    eventSN = 0;

    numsampl = drsRead->getNumOfSamples();
    numofch = drsRead->getNumOfChannels();

    minValOfSignal.resize(numofch);
    maxValOfSignal.resize(numofch);
    sumAmp.resize(numsampl*numofch);

    string tmpNameFile = drsRead->getNameOfDataFile();
    string tmpDir;

    tmpDir = tmpNameFile.substr(0,tmpNameFile.find_last_of("/")+1);

    size_t of = tmpNameFile.find_last_of(".",tmpNameFile.npos);
    size_t fi = tmpNameFile.find_last_of("/",tmpNameFile.npos)+1;
    size_t len = of-fi;
    if (len<=0) inputFileName = "output";
    else
    {
        inputFileName = tmpNameFile.substr(fi,len);
    }
    if(!tmpDir.size()) tmpDir = "./";

    resdir = "res/";
    resdirT = resdir;
    workdir = tmpDir;

    //cout << "tmpDir:" << tmpDir << endl;


    if (access(&(workdir+resdir).c_str()[0],0)!=0) mkdir(&(workdir+resdir).c_str()[0],0777);
}

void DrsSignalProcN::spectrinit()
{
    factor_.resize(numofch);
    shift_.resize(numofch);
    for (int i=0;i<numofch;i++) factor_[i] = 1.;
    for (int i=0;i<numofch;i++) shift_[i] = 0.;
    nBins = 1000;
    //autonameOutFile = true;
    onlinemode = false;

    work_ch_mode_ = 1; // 0b0001

    noise_min_.resize(numofch);
    noise_max_.resize(numofch);
    signal_min_.resize(numofch);
    signal_max_.resize(numofch);
    histSpectr = new TH1F*[numofch];
    canvas = new TCanvas*[numofch];
    canvasflag = false;
    fdataflag = false;
    outpngflag = false;
    //fileopenflag = true;
}

void DrsSignalProcN::rootProc(const vector<vector<float> > &signal)
{
    string outfile;

    outfile = workdir + resdir + inputFileName  + ".root";

    fData = new TFile(outfile.c_str(),"RECREATE");
    if (fData->IsWritable()) fdataflag = true;
    else return;

    createTree(signal,fData);
    createHist(signal,fData);
    createSumHist(fData);
}

void DrsSignalProcN::createHist(const vector<vector<float> > &signal, TFile *tFile)
{
    if(!tFile->IsOpen()) return;

    unsigned short tmpCheckMode = 1;

    if (canvasflag) return;

    for (int i=0;i<numofch;i++)
    {
        if(!safetymode) if (!(tmpCheckMode & work_ch_mode_)) {tmpCheckMode = tmpCheckMode << 1;continue;}
        string pCh = "_" + std::to_string(i)+"ch";
        canvas[i] = new TCanvas(("DRS-Signal"+pCh).c_str(),("DRS-Signal "+pCh).c_str(),800,600);
        histSpectr[i] = new TH1F((inputFileName+pCh).c_str(),(inputFileName+pCh).c_str(),nBins,(minValOfSignal[i]*factor_[i]) + shift_[i],maxValOfSignal[i]*factor_[i] + shift_[i]);

        for (int j=0;j<signal[i].size();j++)
        {
            histSpectr[i]->Fill(factor_[i]*signal[i][j]+shift_[i]);
        }
        histSpectr[i]->Draw();
        canvas[i]->Write();
        if (outpngflag) canvas[i]->SaveAs((workdir + resdir + inputFileName + pCh + ".png").c_str());
        if(!safetymode) tmpCheckMode = tmpCheckMode << 1;
    }
    canvasflag = true;
}

void DrsSignalProcN::deleteHist()
{
    short int tmpCheckMode = 1;
    for (int i=0;i<numofch;i++)
    {
        if (!(tmpCheckMode & work_ch_mode_)) {tmpCheckMode = tmpCheckMode << 1;continue;}
        histSpectr[i]->Delete();
        canvas[i]->Destructor();
        tmpCheckMode = tmpCheckMode << 1;
    }
    canvasflag = false;
}

void DrsSignalProcN::createSumHist(TFile *tFile)
{
    if(!tFile->IsOpen()) return;
    short int tmpCheckMode = 1;

    for (int i=0;i<numofch;i++)
    {
        if(!safetymode) if (!(tmpCheckMode & work_ch_mode_)) {tmpCheckMode = tmpCheckMode << 1;continue;}

        string pCh = "_" + std::to_string(i)+"ch";

        TCanvas *tCan = new TCanvas(("Summed"+pCh).c_str(),("Summed"+pCh).c_str(),800,600);
        TH1F *histSummedSignal = new TH1F((inputFileName+"_summed" + pCh).c_str(),(inputFileName+"_summed" + pCh).c_str(),numsampl,0,numsampl);

        int tmpI = 0;
        float tmpF = 0;
        if (voltModeN_) tmpF = 0.5;
        for (int j=numsampl*i;j<(numsampl*i+numsampl);j++)
        {
            tmpI++;
            histSummedSignal->SetBinContent(tmpI,sumAmp[j] + tmpF);
        }

        histSummedSignal->Draw();
        tCan->Write();
        if (outpngflag) tCan->SaveAs((workdir + resdir + inputFileName + "_summed" + pCh + ".png").c_str());

        histSummedSignal->Delete();
        tCan->Destructor();
        if(!safetymode) tmpCheckMode = tmpCheckMode << 1;
    }
}


void DrsSignalProcN::createTree(const vector<vector<float> > &signal, TFile *tFile)
{
    if(!tFile->IsOpen()) return;
    TTree *signalDataRootTree;
    gROOT->ProcessLine("#include <vector>");

    unsigned short tmpCheckMode = 1;

    for (int i=0; i<numofch; i++)
    {
        if(!safetymode) if (!(tmpCheckMode & work_ch_mode_)) {tmpCheckMode = tmpCheckMode << 1;continue;}

        string pCh = "_" + std::to_string(i)+"ch";
        vector<float> signalPtr = signal[i];

        signalDataRootTree = new TTree(("drsSignal"+pCh).c_str(), ("DRS data " + pCh).c_str());
        signalDataRootTree->Branch("Charge",&signalPtr);
        signalDataRootTree->Branch(("Noise_min_"+pCh).c_str(),&noise_min_[i],"noise_min/s");
        signalDataRootTree->Branch(("Noise_max_"+pCh).c_str(),&noise_max_[i],"noise_max/s");
        signalDataRootTree->Branch(("Signal_min_"+pCh).c_str(),&signal_min_[i],"signal_min/s");
        signalDataRootTree->Branch(("Signal_max_"+pCh).c_str(),&signal_max_[i],"signal_max/s");
        signalDataRootTree->Branch(("Factor_"+pCh).c_str(),&factor_[i],"factor/F");
        signalDataRootTree->Branch(("Shift_"+pCh).c_str(),&shift_[i],"shift/F");

        signalDataRootTree->Fill();
        signalDataRootTree->Write();

        if(!safetymode) tmpCheckMode = tmpCheckMode << 1;
    }
}

void DrsSignalProcN::createTreeTest(const vector<vector<float> > &signal, string outfile, TFile *tFile)
{
    TTree *signalDataRootTree;
    gROOT->ProcessLine("#include <vector>");

    unsigned short tmpCheckMode = 1;

    for (int i=0; i<numofch; i++)
    {
        if (!(tmpCheckMode & work_ch_mode_)) {tmpCheckMode = tmpCheckMode << 1;continue;}

        string pCh = "_" + std::to_string(i)+"ch";
        vector<float> signalPtr = signal[i];

        signalDataRootTree = new TTree(("drsSignal"+pCh).c_str(), ("DRS data " + pCh).c_str());
        signalDataRootTree->Branch("Charge",&signalPtr);
        signalDataRootTree->Branch(("Noise_min_"+pCh).c_str(),&noise_min_[i],"noise_min/s");
        signalDataRootTree->Branch(("Noise_max_"+pCh).c_str(),&noise_max_[i],"noise_max/s");
        signalDataRootTree->Branch(("Signal_min_"+pCh).c_str(),&signal_min_[i],"signal_min/s");
        signalDataRootTree->Branch(("Signal_max_"+pCh).c_str(),&signal_max_[i],"signal_max/s");
        signalDataRootTree->Branch(("Factor_"+pCh).c_str(),&factor_[i],"factor/F");
        signalDataRootTree->Branch(("Shift_"+pCh).c_str(),&shift_[i],"shift/F");

        signalDataRootTree->Fill();
        signalDataRootTree->Write();

        tmpCheckMode = tmpCheckMode << 1;
    }
}

void DrsSignalProcN::setVoltMode(float voltMode)
{
    /**
      \brief
    */

    voltMode_ = voltMode;
}

void DrsSignalProcN::setWorkingChannelsMode(unsigned short work_ch_mode)
{
    work_ch_mode_ = work_ch_mode;
}

void DrsSignalProcN::autoSignalDetectKusskoff(const vector<unsigned short> &n_amplitudes, unsigned short chNum, bool isNewCh, bool endfile)
{
    countSumAmpKusskoff(n_amplitudes,chNum);
    if (isNewCh && !endfile) eventSN++;

    if (!endfile) return;

    autoSignalDetectKusskoffProc(++eventSN, chNum);
    //eventSN=0;
}

vector<float> DrsSignalProcN::getIntegralSignal()
{
    /**
      \brief Получить интегральный сигнал
    */

    return sumAmp;
}

vector<std::pair<float, float> > DrsSignalProcN::getMinMaxValOfSignal()
{
    /**
      \brief Получить Максимальные и минимальные значения рассчитанного сигнала.

      \return Вестор пар значений (min,max)
    */

    vector<std::pair<float, float> > tmpVec;
    tmpVec.reserve(numofch);
    for (int i=0;i<numofch;i++)
    {
        tmpVec.push_back(std::pair<float, float>(minValOfSignal[i],maxValOfSignal[i]));
    }

    return tmpVec;
}

void DrsSignalProcN::setFactorAndShift(float factor, float shift)
{
    /**
      \brief Установка множителя и сдвига для всех каналов

      \param factor Множитель

      \param shift Сдвиг
    */

//    if (chNum+1 > numofch) cerr << RED_SH << " Error of set factor & shift. Channel number more than number of channel" << ENDCOLOR << endl;
    for (int i=0;i<numofch;i++)
    {
        factor_[i] = factor;
        shift_[i] = shift;
    }
}

void DrsSignalProcN::getFactorAndShift(vector<float> &factor, vector<float> &shift)
{
    /**
      \brief Получение множителя и сдвига для всех каналов

      \param &factor Ссылка на вектор множителей

      \param &shift Ссылка на вектор сдвигов
    */

    factor = factor_;
    shift = shift_;
}

void DrsSignalProcN::setAmplitudeKuskoffMode(bool isAmp)
{
    kuskoff_amplitude = isAmp;
}

void DrsSignalProcN::setSafetyMode()
{
    safetymode = true;
}

void DrsSignalProcN::setOutputDirectory(string outDir)
{
    string tmpNameFile = outDir;
    size_t of = tmpNameFile.find_last_of(".",tmpNameFile.npos);
    size_t fi = tmpNameFile.find_last_of("/",tmpNameFile.npos)+1;
    size_t len = of-fi;
    if (len<=0) outDir = "tmp/";
    else outDir = tmpNameFile.substr(fi,len);

    resdir = resdirT + outDir + "/";

    if (access(&(workdir+resdir).c_str()[0],0)!=0) mkdir(&(workdir+resdir).c_str()[0],0777);
}


vector<vector<float> > DrsSignalProcN::getSpectumOffline()
{
    /**
      \brief Считает спектр для выбранных каналов

      \return Вектор векторов значений рассчитанного спектра. Размерность внешнего вектора соответствует количеству использованных каналов.
    */

    bool endFile = false;
    vector<float> times;

    std::chrono::time_point<std::chrono::system_clock> startDetect,startRun,end;

    vector<unsigned short> amplitudes;
    vector<vector<float>> signalValues;
    vector<unsigned long> nP;
    unsigned short tmpUsedCh;

    std::pair<unsigned long,unsigned long> timestamps;
    long ntime;
    struct tm *m;
    long nPulses = 0;
    long tmpS = 0; // for count


    cout << YELLOW_SH << "Spectr Info:" << ENDCOLOR << endl;
    cout << endl;

    unsigned short tmpCheckMode = 1;
    bool tmpNewCh;
    if(!onlydetect) startDetect = std::chrono::system_clock::now();

    if (safetymode)
    {
        nP = drsRead->countNumOfPulses();
        cout << "SIZE N " <<  nP.size() << endl;
        for (int i=0;i<nP.size();i++)
        {
            cout << "SIZE " << i  << " " << nP[i] << endl;
        }
        cout << endl;
    }
    else nPulses = drsRead->calcNumOfPulses();
    if (autodetect)
    {
        if (safetymode)
        {
            while (!endFile) {
                endFile = drsRead->drsGetFrameSafety(amplitudes,times,tmpUsedCh);
                for (int i=0;i<numofch;i++)
                {
                    if (!i) tmpNewCh = true;
                    autoSignalDetectKusskoff(amplitudes,i,tmpNewCh,endFile);
                    tmpNewCh = false;
                } tmpS++;
            }
            nPulses = tmpS;
        }
        else
        {
            while (!endFile) {
                endFile = drsRead->drsGetFrame(amplitudes,times,work_ch_mode_);
                for (int i=0;i<numofch;i++)
                {
                    if (!i) tmpNewCh = true;

                    if (tmpCheckMode & work_ch_mode_ )
                    {
                        autoSignalDetectKusskoff(amplitudes,i,tmpNewCh,endFile);
                        tmpNewCh = false;
                    }
                    tmpCheckMode = tmpCheckMode << 1;
                }
                tmpCheckMode = 1;
            }
        }
    }

    //vector<unsigned long int>

    if (onlydetect)
    {
        timestamps = drsRead->getTimeStampsOfEvents();

        ntime = (timestamps.second - timestamps.first);//1000;
        m = gmtime(&ntime);

        cout << " Acquisition time : "  << BLUE_SH  <<  m->tm_mday-1 << ENDCOLOR <<" days " << BLUE_SH << m->tm_hour << ENDCOLOR << " hours " << BLUE_SH << m->tm_min << ENDCOLOR << " min   " << BLUE_SH << m->tm_sec << ENDCOLOR << " sec" << endl;
        cout << " Average frequency: "<< BLUE_SH  <<  (float)nPulses/(float)ntime << ENDCOLOR <<" Hz" << endl;

        for (int i=0;i<numofch;i++)
        {
            cout << endl;
            cout << " Channel " << i+1 << endl;
            cout << " Noise min:\t" << BLUE_SH << noise_min_[i] << ENDCOLOR << endl;
            cout << " Noise max:\t" << BLUE_SH << noise_max_[i] << ENDCOLOR << endl;
            cout << " Signal min\t" << BLUE_SH << signal_min_[i] << ENDCOLOR << endl;
            cout << " Signal max\t" << BLUE_SH << signal_max_[i] << ENDCOLOR << endl;

            if ((times.size()/numsampl)==(numofch))
                cout << " Sampling frequency:"<< BLUE_SH  << 1/(times[i*numsampl+10] - times[i*numsampl+9]) << ENDCOLOR <<"GS/s" << endl;
            else
                cout << " Sampling frequency:"<< BLUE_SH  << 1/(times[10] - times[9]) << ENDCOLOR <<"GS/s" << endl;
        }
        return signalValues;
    }

    startRun = std::chrono::system_clock::now();


    signalValues.resize(numofch);
    if (!safetymode) for (int i=0;i<numofch;i++) signalValues[i].reserve(drsRead->calcNumOfPulses());
    else for (int i=0;i<numofch;i++) signalValues[i].reserve(nP[i]);

    drsRead->drsFileSeekBegin();
    endFile =false;

    float tmpSignal;
    vector<float> tmpmaxsignal,tmpminsignal;
    tmpmaxsignal.resize(numofch);
    tmpminsignal.resize(numofch);

    vector<bool> tmpI;
    tmpI.resize(numofch);

    tmpCheckMode = 1;
    if (safetymode)
    {
        while  (!endFile)
        {
            endFile = drsRead->drsGetFrameSafety(amplitudes,times,tmpUsedCh);
            for (int i=0;i<numofch;i++)
            {
                if(!(tmpCheckMode & tmpUsedCh))
                {
                    tmpCheckMode = tmpCheckMode << 1;
                    continue;
                }
                tmpSignal = getSignalWithKuskoffMethod(amplitudes,times,i);
                if(!autodetect) countSumAmpKusskoff(amplitudes,i);
                if(tmpSignal!=-1111)
                {
                    signalValues[i].push_back(tmpSignal);
                    if (!tmpI[i]) {tmpmaxsignal[i] = tmpminsignal[i] = tmpSignal; tmpI[i]=!tmpI[i];}
                    else
                    {
                        if (tmpSignal>tmpmaxsignal[i]) tmpmaxsignal[i] = tmpSignal;
                        if (tmpSignal<tmpminsignal[i]) tmpminsignal[i] = tmpSignal;
                    }
                }
                tmpCheckMode = tmpCheckMode << 1;
            }
            tmpCheckMode = 1;
            if (!autodetect) tmpS++;
        }
        if (!autodetect) nPulses = tmpS;
    }
    else
    {
        while  (!endFile)
        {
            endFile = drsRead->drsGetFrame(amplitudes,times,work_ch_mode_);

            for (int i=0;i<numofch;i++)
            {
                if (tmpCheckMode & work_ch_mode_ )
                {
                    tmpSignal = getSignalWithKuskoffMethod(amplitudes,times,i);
                    if(!autodetect) countSumAmpKusskoff(amplitudes,i);
                    if(tmpSignal!=-1111)
                    {
                        signalValues[i].push_back(tmpSignal);
                        if (!tmpI[i]) {tmpmaxsignal[i] = tmpminsignal[i] = tmpSignal; tmpI[i]=!tmpI[i];}
                        else
                        {
                            if (tmpSignal>tmpmaxsignal[i]) tmpmaxsignal[i] = tmpSignal;
                            if (tmpSignal<tmpminsignal[i]) tmpminsignal[i] = tmpSignal;
                        }
                    }
                }
                tmpCheckMode = tmpCheckMode << 1;
            }
            tmpCheckMode = 1;
        }
    }

    minValOfSignal = tmpminsignal;
    maxValOfSignal = tmpmaxsignal;

    timestamps = drsRead->getTimeStampsOfEvents();

    ntime = (timestamps.second - timestamps.first);//1000;
    m = gmtime(&ntime);

    cout << " Acquisition time : "  << BLUE_SH  <<  m->tm_mday-1 << ENDCOLOR <<" days " << BLUE_SH << m->tm_hour << ENDCOLOR << " hours " << BLUE_SH << m->tm_min << ENDCOLOR << " min   " << BLUE_SH << m->tm_sec << ENDCOLOR << " sec" << endl;
    cout << " Average frequency: "<< BLUE_SH  <<  (float)nPulses/(float)ntime << ENDCOLOR <<" Hz" << endl;

    for (int i=0;i<numofch;i++)
    {
        cout << endl;
        cout << " Channel " << i+1 << endl;
        cout << " Noise min:\t" << BLUE_SH << noise_min_[i] << ENDCOLOR << endl;
        cout << " Noise max:\t" << BLUE_SH << noise_max_[i] << ENDCOLOR << endl;
        cout << " Signal min\t" << BLUE_SH << signal_min_[i] << ENDCOLOR << endl;
        cout << " Signal max\t" << BLUE_SH << signal_max_[i] << ENDCOLOR << endl;
        cout << " values size: " << BLUE_SH << signalValues[i].size() << ENDCOLOR;
        cout << " Max: " << BLUE_SH << maxValOfSignal[i] << ENDCOLOR;
        cout << " Min: " << BLUE_SH << minValOfSignal[i] << ENDCOLOR << endl;
        if ((times.size()/numsampl)==(numofch))
            cout << " Sampling frequency:"<< BLUE_SH  << 1/(times[i*numsampl+10] - times[i*numsampl+9]) << ENDCOLOR <<" GS/s" << endl;
        else
            cout << " Sampling frequency:"<< BLUE_SH  << 1/(times[10] - times[9]) << ENDCOLOR <<" GS/s" << endl;
    }
    cout << "----------"<< endl;
    cout << endl;

    end = std::chrono::system_clock::now();
    cout << " 1st step runtime (autodetect): " << std::chrono::duration_cast<std::chrono::microseconds>(startRun-startDetect).count()/1e6 << "s" << endl;
    cout << " 2nd step runtime (run): " <<  std::chrono::duration_cast<std::chrono::microseconds>(end-startRun).count()/1e6 << "s" << endl;
    cout << " Total runtime: " << std::chrono::duration_cast<std::chrono::microseconds>(end-startDetect).count()/1e6 << "s" << endl;
    return signalValues;//std::move(signalValues);
}

void DrsSignalProcN::setOutPngFileFlag()
{
    outpngflag = true;
}

void DrsSignalProcN::autoSignalDetectKusskoffProc(int eventnum, unsigned short chNum)
{
    for (int i=chNum*numsampl; i<(chNum*numsampl + numsampl); i++)
        sumAmp[i]= sumAmp[i]/eventnum;
    //float x[124];
    float y[124];

    for(int i=0;i<124;i++) {
      //x[i] = i;
      y[i] = 0;
    }

    float integrall = 0;

    int tmpI = 16;
    for(int i= (chNum*numsampl +16);i<(chNum*numsampl + numsampl-16);i++) {
      y[(tmpI-16)/8] += sumAmp[i];
      integrall += sumAmp[i];
      tmpI++;
    }

    double p[5];
    p[1] = 0;
    for(int i = 0; i < 10; i++)
      p[1] += y[i];
    p[1] = p[1]/10;

    p[0]= (y[123]-p[1])/124;



    float max = 0;

    for(int i = 0; i<124;i++)
      if(integrall*max <= integrall*y[i]) {
        max = y[i];
      }

    double half_height = (max-p[1])/sqrt(2.718)+p[1];
    int half_height_min = 0, half_height_max = 0;

    for(int i = 0; i<124;i++)
      if(integrall*half_height <= integrall*y[i]) {
        half_height_min = i;
        break;
      }

    for(int i = half_height_min; i<124;i++)
      if(integrall*half_height > integrall*y[i]) {
        half_height_max = i;
        break;
      }

    p[3] = (half_height_max+half_height_min)/2.;
    p[4] = (half_height_max-half_height_min)/2.;

    p[2] = (integrall-p[1]*124-p[0]*(124*124/2))/(2.507*p[4]);

    //FIT???

    signal_min_[chNum] = p[3] - p[4]*2.6;
    signal_max_[chNum] = p[3] + p[4]*2.6;
    noise_min_[chNum] = 0;
    noise_max_[chNum] = p[3] - p[4]*3.3;

    signal_min_[chNum] = 16+8*signal_min_[chNum];
    signal_max_[chNum] = 16+8*signal_max_[chNum];
    noise_min_[chNum] = 16+8*noise_min_[chNum];
    noise_max_[chNum] = 16+8*noise_max_[chNum];


    if(integrall < 0) posorneg=-1;
    else posorneg=1;
    //factor = posorneg*factor;
}

float DrsSignalProcN::getSignalWithKuskoffMethod(const vector<unsigned short> &n_amplitudes, const vector<float> &n_times, unsigned short chNum)
{
    //float amps[numsampl];
    vector<float> amps(numsampl);
    int eventnum=0;
    float noise = 0;
    float signal = 0;
    eventSN++;
    for (int i=0;i<numsampl;i++)
    {
        amps[i] = /*factor**/(n_amplitudes[chNum*numsampl+i]/65535.- voltMode_)/* + factorB*/;
        //amps[i] = factor*amps[i] + factorB;
    }
    for (int i=noise_min_[chNum];i<noise_max_[chNum];i++)
    {
        noise += amps[i];
        noise = noise /(noise_max_[chNum] - noise_min_[chNum]);
    }
    if (kuskoff_amplitude)
    {
        if(signal_max_[chNum] - signal_min_[chNum] < 50) {
                cerr << RED_SH <<"Too small signal range\n" << ENDCOLOR;
                return 1;
            }
            double avg[50];
            int nums[50];
            for(int i = 0; i < 50; i++) {
                avg[i] = 0;
                nums[i] = 0;
            }
            for(int i=signal_min_[chNum]; i<signal_max_[chNum]; i++) {
                int idx = 50*(i-signal_min_[chNum])/(signal_max_[chNum] - signal_min_[chNum]);
                avg[idx] += amps[i];
                nums[idx] += 1;
            }

            for(int i = 0; i < 50; i++) {
                avg[i] = avg[i] / nums[i] * 8;
            }

            double e = 0;
            double d = 0;
            double c = 0;
            double b = 0;
            double a = 0;
            double x0y = 0;
            double x1y = 0;
            double x2y = 0;

                int maxpos = 0;
                double maxval = avg[0];
                for(int i = 1; i < 50; i++)
                  if(avg[i] > maxval) {
                    maxpos = i;
                    maxval = avg[i];
                  }

                if((maxpos < 5) || (maxpos > 45))
                {
#ifdef DEBUG
                    cout << "Frame " << eventSN << ": no reasonable max found" << endl;
#endif
                    return -1111;
                }

                for(int i = std::max(0,maxpos - 8); i<std::min(50, maxpos + 9); i++) {
                double Xval = i/50.*(signal_max_[chNum] - signal_min_[chNum])+signal_min_[chNum];
                e += 1;
                d += Xval;
                c += Xval*Xval;
                b += Xval*Xval*Xval;
                a += Xval*Xval*Xval*Xval;

                x0y += 1*avg[i];
                x1y += Xval*avg[i];
                x2y += Xval*Xval*avg[i];
            }

            double demoninator = -a*c*e+a*d*d+b*b*e-2*b*c*d+c*c*c;

            double A = (d*d - c*e)*x2y + (b*e - c*d)*x1y  + (c*c-b*d) * x0y;
            double B = (b*e - c*d)*x2y + (c*c - a*e)*x1y  + (a*d-b*c) * x0y;
            double C = (c*c - b*d)*x2y + (a*d - b*c)*x1y  + (b*b-a*c) * x0y;
            A = A/demoninator;
            B = B/demoninator;
            C = C/demoninator;

            signal = B*B/4/A + C + noise;

            //printf("Parabulum: %g %g %g\n", A, B, C);
    }
    else
    for (int i=signal_min_[chNum]; i<signal_max_[chNum]; i++)
    {
        if (n_times.size()<(numsampl+1)) {
            if (i<numsampl-1) signal += (((amps[i]/2.+amps[i+1]/2.)- noise)*(n_times[i+1]-n_times[i]));
        }
        else {
            if (i<numsampl-1) signal += (((amps[i]/2.+amps[i+1]/2.)- noise)*(n_times[numsampl*chNum + i+1]-n_times[numsampl*chNum + i]));
        }
    }
    signal = posorneg/**factor*/*signal/* + factorB*/;
    return signal;
}

void DrsSignalProcN::countSumAmpKusskoff(const vector<unsigned short> &n_amplitudes, unsigned short chNum)
{
    int  ready_to_overflow = 0, overflow = 0;
    for ( int i = numsampl*chNum; i < (numsampl*chNum + numsampl); i++ )
    {
        float amp =  (float)n_amplitudes[i]/65535.-voltMode_;
        if(overflow) {
            if(amp > 0) {
                overflow = 0;
            }
            ready_to_overflow = (amp > 0.35);
        } else
            if(ready_to_overflow) {
                if(amp < -0.25) {
                    overflow = 1;
                } else if (amp < 0.35)
                    ready_to_overflow = 0;
            } else
                if(amp > 0.35)
                    ready_to_overflow = 1;
        if(overflow)
            amp += 1.;

        sumAmp[i] += amp/*/numsampl*/;
    }
}
