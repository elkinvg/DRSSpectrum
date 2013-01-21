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




string NameOutputFile;
bool amplitudekuskoffmode=false; // amplitude = true; charge = false
unsigned short int get_noise_min,get_noise_max,get_signal_min, get_signal_max;
bool oneline_mode = false;

istringstream iss;
string tmpvalue;
void ctrlplusc(int sig);
void UseApp();
void RunApp();

float getfactor=0;     //
float getfactorB=0;    //  y = factor*x + factorB
unsigned int NumOfBins=0;

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

static const char *optString = "mho:1:2:dab:r";
static const struct option longOpts[] = {
    {"help", 0, 0, 'h'},
    {"oneline-mode", 0, 0, 'm'},
    {"outfile", 1, 0, 'o'},
    {"factora", 1, 0, '1'},
    {"factorb", 1, 0, '2'},
    {"number-of-bins", 1, 0, 'b'},
    {"only-detect", 0, 0, 'd'},
    {"amplitute", 0, 0, 'a'},
    {"root-application", 0, 0, 'r'},
    {0, 0, 0, 0}
};
void help()
{
    cout << "Usage:\t drsspectrum INPUTFILE [noise_min noise max signal_min signal_max]" <<"\n\t [[-o|--outfile] outfile] " /*<< endl*/;
    cout << "[[-n|--number-of-bins] Number_of_bins] [[-1|--factora] factora] [[-2|--factorb] factorb]" << endl;
    cout<< "\t [-d|--only-detect] [-a|--amplitute] [-r|--root-application]"  << endl;
    cout << endl;
    cout << " -1 -2 Integral = factora*X + factorb; factora is -1 factor b is -2 " << endl;
    cout << " -d detect noise_min noise max signal_min signal_max and exit " << endl;
    cout << " -a run with amplitude mode" << endl;
    cout << " -r run without root aplication " << endl;

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
            cout << " help: " << endl;
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
        case '1':
            getfactor = atof(argv[optind-1]);
            if (getfactor==0) cout << argv[optind-1] <<" factor a must be float " << endl;
            break;

        case '2':
            getfactorB = atof(argv[optind-1]);
            if (getfactorB==0) cout << argv[optind-1] <<" factor b must be float " << endl;
            break;

        case 'b':
            NumOfBins = atoi(argv[optind-1]);
            break;

        case 'a':
            amplitudekuskoffmode=true;
            break;

        case 'r':
            rootapplicationflag=false;
//            cout << "\e[1;33m \e[40m Use CTRL+C to exit!!! \E[0m" << endl;
            break;

        case 'd':
            onlydetect = true;
            break;
        case 0:
//            if (strcmp("longopts",longOpts[longIndex].name) == 0)

            exit(0);
        default:
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
        }

//        tmpvalue=string(argv[optind]);
        int tmpval;
        tmpval = fromStr(string(argv[optind+1]));

        if(tmpval<0 || tmpval>numsampl)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_noise_min = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+2]);
        if(tmpval<0 || tmpval>numsampl)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_noise_max = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+3]);
        if(tmpval<0 || tmpval>numsampl)
        {
            cerr << " error parameters " << endl;
            exit(1);
        }
        get_signal_min = (unsigned short int)tmpval;
        tmpval = fromStr(argv[optind+4]);
        if(tmpval<0 || tmpval>numsampl)
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
        }
        else
        {
            spectrum = new DRSSpectrumProc();
        }
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
