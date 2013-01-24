#include <iostream>
#include <getopt.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <signal.h>


#include "drsspectrumproc.h"

#ifndef __MINGW32__
#include <root/TApplication.h>
#endif
using namespace std;

// Flags
bool onlydetect=false;
bool inputparameters=false;
bool outputfileflag=false;
bool rootapplicationflag=true;
bool graphintegralsignal=false;





string NameOutputFile;
bool amplitudekuskoffmode=false; // amplitude = true; charge = false
unsigned short int get_noise_min,get_noise_max,get_signal_min, get_signal_max;
bool oneline_mode = false;

istringstream iss;
string tmpvalue;
void ctrlplusc(int sig);
void UseApp();
void RunApp();

float getfactor=1;     //
float getfactorB=0;    //  y = factor*x + factorB
float tmpf;
unsigned int NumOfBins=0;
const int NumSamples=1024;

#ifndef __MINGW32__
TApplication *myapp;
#endif
DRSSpectrumProc *spectrum;

//template < typename T >
int fromStr(const std::string aS)
{
        std::istringstream _iss(aS);
        int _res;
        _iss >> _res;
        if (_res==0) return -1;
        return _res;
}

static const char *optString = "mho:a:b:n:dkrg";
static const struct option longOpts[] = {
    {"help", 0, 0, 'h'},
    {"oneline-mode", 0, 0, 'm'},
    {"outfile", 1, 0, 'o'},
    {"a-factor", 1, 0, 'a'},
    {"b-shift", 1, 0, 'b'},
    {"number-of-canal", 1, 0, 'n'},
    {"only-detect", 0, 0, 'd'},
    {"amplitute", 0, 0, 'k'},
    {"without-root-application", 0, 0, 'r'},
    {"graph-integral",0, 0,'g'},
    {0, 0, 0, 0}
};
void help()
{
    cout << "Usage:\t drsspectrum INPUTFILE [noise_min noise max signal_min signal_max]" <<"\n\t [[-o|--outfile] outfile] " /*<< endl*/;
    cout << /*"[[-n|--number-of-canal] Number_of_canal]*/ "[[-a|--a-factor] factor] [[-b|--b-shift] shift]" << endl;
    cout<< "\t [-d|--only-detect] [-k|--amplitute] [-r|--without-root-application]"  << endl;
    cout << endl;
    cout << " -a -b: Integral = factor*X + shift; factor is -a shift is -b " << endl;
    cout << " -d detect noise_min noise max signal_min signal_max and exit " << endl;
    cout << " -k run with amplitude mode. Default mode is charge" << endl;
    cout << " -r run without root aplication " << endl;
    cout << " -g plot of the integral signal " << endl;


    exit(0);
}


int main(int argc, char** argv)
{
     // offline or oneline
    int opt = 0;
    int longIndex = 0;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    while( opt != -1 ) {
        switch( opt ) {
        case 'h':
            help();
            break;
        case 'o':
            cout << " outputfile " << /*optind <<*/ argv[optind-1] <<endl;
            NameOutputFile = (string)(argv[optind-1]);
            outputfileflag = true;
            break;
        case 'm':
            cout << "oneline mode " << endl;
            cout << "***********************************" << endl;
            oneline_mode = true;
            break;
        case 'a':
            tmpf = atof(argv[optind-1]);
            if (tmpf==0) cout << argv[optind-1] <<" factor a must be float " << endl;
            getfactor = getfactor*tmpf;
            break;

        case 'b':
            tmpf = atof(argv[optind-1]);
            if (tmpf==0) cout << argv[optind-1] <<" factor b must be float " << endl;
            getfactorB = getfactorB+tmpf;
            break;

        case 'n':
            NumOfBins = atoi(argv[optind-1]);
            break;

        case 'k':
            amplitudekuskoffmode=true;
            break;

        case 'r':
            rootapplicationflag=false;
            break;
        case 'g':
            graphintegralsignal = true;
            break;

        case 'd':
            onlydetect = true;
            break;
        case 0:
//            if (strcmp("longopts",longOpts[longIndex].name) == 0)

            exit(0);
        default:
            help();
            exit(0);
        }
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    }


    if (argc-optind==0) { cout << "No input file specified" << endl; exit(0);}
    if ((argc-optind>1 )&&(argc-optind!=5) ) {cout << " many arguments! " << endl; exit(0);}

    if (argc-optind==5)
    {
        if(onlydetect)
        {
            cerr << " с параметром -d не должно быть дополнительных аргументов" << endl;
            onlydetect = false;
            graphintegralsignal = false;
        }

//        tmpvalue=string(argv[optind]);
        int tmpval;
        tmpval = fromStr(string(argv[optind+1]));

        if(tmpval<0 || tmpval>NumSamples)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_noise_min = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+2]);
        if(tmpval<0 || tmpval>NumSamples)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_noise_max = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+3]);
        if(tmpval<0 || tmpval>NumSamples)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_signal_min = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+4]);
        if(tmpval<0 || tmpval>NumSamples)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_signal_max = (unsigned short int)tmpval;
        if (get_noise_min<16) get_noise_min=16;
        if (get_noise_max <= get_noise_min )
        {
            cerr << " error parameters: noise_max < noise_min " << endl;
            exit(1);
        }
        if (get_signal_max <= get_signal_min )
        {
            cerr << " error parameters: signal_max <= signal_min " << endl;
            exit(1);
        }
        if (get_noise_min>=get_signal_min)
        {
            cerr << " error parameters: noise_min>=signal_min " << endl;
            exit(1);
        }
        if (get_noise_max>get_signal_min) get_noise_max=get_signal_min;
        inputparameters=true;
    }


    if(inputparameters)
    {
        spectrum = new DRSSpectrumProc(get_noise_min,get_noise_max,get_signal_min,get_signal_max);
    }
    else
    {
        if(onlydetect)
        {
            spectrum = new DRSSpectrumProc(onlydetect);
            if(!graphintegralsignal) rootapplicationflag = false;
        }
        else
        {
            spectrum = new DRSSpectrumProc();
        }
    }

    if (graphintegralsignal)
    {
        if(outputfileflag) spectrum->SetOutFileName(NameOutputFile);
        if (amplitudekuskoffmode) spectrum->SetModeIntegral(amplitudekuskoffmode);
        if(NumOfBins>0) spectrum->SetNumberOfBins(NumOfBins);
        if(rootapplicationflag) UseApp();
        spectrum->CreatIntegralGraph(argv[optind]);
        if(rootapplicationflag) RunApp();
    }
    if(!oneline_mode)
    {
        if(outputfileflag) spectrum->SetOutFileName(NameOutputFile);
        if(getfactor!=0 || getfactorB!=0) spectrum->SetFactor(getfactor,getfactorB);
        if (amplitudekuskoffmode) spectrum->SetModeIntegral(amplitudekuskoffmode);
        if(NumOfBins>0) spectrum->SetNumberOfBins(NumOfBins);
        if(rootapplicationflag) UseApp();
        spectrum->GetSpectumOffline(argv[optind]);
        if(rootapplicationflag) RunApp();
    }

    delete spectrum;

    return 0;
}


void ctrlplusc(int sig)
{
    cout << "..." << endl;
    if(rootapplicationflag) delete spectrum;
    exit(0);
}

void UseApp()
{
    signal(SIGINT,ctrlplusc);
#ifndef __MINGW32__
    myapp = new TApplication("App",0,0);
    cout << "\e[1;33m \e[40m Use CTRL+C to exit!!! \E[0m" << endl;

#endif
}

void RunApp()
{
#ifndef __MINGW32__
     myapp->Run();
#endif
}
