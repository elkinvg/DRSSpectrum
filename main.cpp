#include <iostream>
#include <getopt.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <signal.h>

//#include "drsreadoffline.h"
#include "drsspectrumproc.h"

#include <root/TRint.h>

using namespace std;

bool onlydetect=false;
bool inputparameters=false;
bool outputfileflag=false;
string NameOutputFile;
bool amplitudekuskoffmode=false; // amplitude = true; charge = false
unsigned short int get_noise_min,get_noise_max,get_signal_min, get_signal_max;
bool oneline_mode = false;
istringstream iss;
string tmpvalue;
void fg(int sig);
float getfactor=0;
unsigned int NumOfBins=0;

//template < typename T >
int fromStr(const std::string aS)
{
        std::istringstream _iss(aS);
        int _res;
        _iss >> _res;
        if (_res==0) return -1;
        return _res;
}

static const char *optString = "mho:f:dab:";
static const struct option longOpts[] = {
    {"help", 0, 0, 'h'},
    {"oneline-mode", 0, 0, 'm'},
    {"outfile", 1, 0, 'o'},
    {"factor", 1, 0, 'f'},
    {"number-of-bins", 1, 0, 'b'},
    {"only-detect", 0, 0, 'd'},
    {"amplitute", 0, 0, 'a'},
    {0, 0, 0, 0}
};
void help();

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
        case 'f':
            getfactor = atof(argv[optind-1]);
            if (getfactor==0) cout << argv[optind-1] <<" factor must be float " << endl;
            break;
        case 'b':
            NumOfBins = atoi(argv[optind-1]);
            break;

        case 'a':
            amplitudekuskoffmode=true;

        case 'd':
            onlydetect = true;
            break;
        case 0:
//            if (strcmp("ddd",longOpts[longIndex].name) == 0)
//            cout << "ddd " << endl;
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
    DRSSpectrumProc *spectrum;

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
        if(getfactor!=0) spectrum->SetFactor(getfactor);
        if (amplitudekuskoffmode) spectrum->SetModeIntegral(amplitudekuskoffmode);
        if(NumOfBins>0) spectrum->SetNumberOfBins(NumOfBins);
        TApplication *myapp = new TApplication("h",0,0);
        //TRint *myapp = new TRint("h",0,0);
        spectrum->GetSpectumOffline(argv[optind]);
        myapp->Run();
        cout << " ADD!" << endl;
        //myapp->Terminate(1000);
        //myapp->Terminate(0);
    }

    delete spectrum;

    return 0;
}

void help()
{
    cout << "Usage:\t drsspectrum INPUTFILE [noise_min noise max signal_min signal_max]" <<"\n\t [[-o|--outfile] outfile] " /*<< endl*/;
    cout << "[[-n|--number-of-bins] Number_of_bins] [[-f|--factor] factor]" << endl;
    cout<< "\t [-d|--only-detect] [-a|--amplitute]"  << endl;

    exit(0);
}

