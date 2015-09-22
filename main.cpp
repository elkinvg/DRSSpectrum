#include <iostream>
#include <sstream>
#include <getopt.h>
#include <signal.h>

#ifdef USETHREADAPP
#include <thread>
#endif

#include "drstype.h"
#include "drssignalprocn.h"

#ifndef DEBROOT
#include <root/TApplication.h>
#else
#include <TApplication.h>
#endif

TApplication *myapp;
void ctrlplusc(int sig);

void UseApp();
void RunApp();
DrsReadN *drs;
DrsSignalProcN *signalProc;

using std::cout;
using std::endl;

//Flags
bool onlydetect = false;
bool inputparameters = false;
bool singleChannelMode = true;
bool rootapplicationflag=true;
bool isoutpng=false;
bool amplitudekuskoffmode=false;
bool isnewoutdir=false;
bool safetymode=false;
bool voltmode=false;
bool onlinemode=false;

unsigned short modes = 0;

unsigned short int get_noise_min,get_noise_max,get_signal_min, get_signal_max;
int chMode = 0; // Channel mode
float getfactor=1;     //
float getfactorB=0;    //  y = factor*x + factorB
float timerNsec; // for timer (onlinemode)

short int maxNCh;

void online()
{
//    UseApp();
    // нужно для онлайн режима. на некоторых компьютерах, если не запустить
    // RunApp() не отображаются гистограммы, поэтому пришлось запустить в отдельном потоке
    // не уверен, что это правильное решение, но пока единственное найденное.
    sleep(2);
    RunApp();
}

int fromStr(const std::string aS)
{
    std::istringstream _iss(aS);
    int _res;
    _iss >> _res;
    if (_res==0) return -1;
    return _res;
}

static const char *optString = "khdrosvm:a:b:n:c:e:";

static const struct option longOpts[] = {
    {"help", 0, 0, 'h'},
    {"only-detect", 0, 0, 'd'},
    {"without-root-app", 0, 0, 'r'},
    {"amplitude", 0, 0, 'k'},
    {"out-png", 0, 0, 'o'},
    {"safety", 0, 0, 's'},
    {"volt-mode", 0, 0, 'v'},
    {"channel-mode",1, 0,'m'},
    {"max-num", 1, 0,'c'},
    {"a-factor", 1, 0, 'a'},
    {"b-shift", 1, 0, 'b'},
    {"outdir", 1, 0, 'n'},
    {"online", 1, 0, 'e'},
    {"version",0,0,0},
    {0, 0, 0, 0}
};

void help()
{
    cout << "Version 2.3" << endl;
    cout << "Usage:    drsspectrum\tINPUTFILE ";
    cout << "[noise_min noise max signal_min signal_max] " << endl;
    cout << "\t\t\t[-d | --only-detect] [-h | --help] [[-m|--channel-mode] Working channels]" << endl;
    cout << "\t\t\t[-r|--without-root-application] [[-a|--a-factor] factor] [[-b|--b-shift] shift]" << endl;
    cout << "\t\t\t[-o|--out-png] [-k|--amplitude] [[-n|--outdir] newoutdir] [-s|safety]" << endl;
    cout << "\t\t\t[[-c]--max-num] max num of chs] [-v|--volt-mode] [[-e|--online] sec]" << endl;
    cout << endl;
    cout << "   [noise_min noise max signal_min signal_max] Установка локализации шума и сигнала для всех каналов" << endl;
    cout << "   -d,\t--only-detect\tВычислить noise_min noise max signal_min signal_max и выйти" << endl;
    cout << "   -m\t--channel-mode\t"<< BOLD_SH<<"<channels>"<< ENDCOLOR << " Выбрать рабочие каналы из используемых [4ch 3ch 2ch 1ch]. Записывать десятичное число. Пример: -m 4 для 3го канала (0b0100)" << endl;
    cout << "   -r,\t--without-root-app\tЗапустить без root aplication" << endl;
    cout << "   -a,\t--a-factor\t"<< BOLD_SH<<"<factor>"<< ENDCOLOR << " установить множитель" << endl;
    cout << "   -b,\t--b-shift\t"<< BOLD_SH<<"<shift>"<< ENDCOLOR << " установить сдвиг" << endl;
    cout << "   -o,\t--out-png\tВывести гистограммы в png" << endl;
    cout << "   -k,\t--amplitude\tЗапустить в амплитудном режиме." << endl;
    cout << "   -n,\t--outdir\t"<< BOLD_SH <<"<dir>"<< ENDCOLOR << "Имя новой output директории. Новая директория будет в директории res" << endl;
    cout << "   -s,\t--safety\tБезопасный режим. Если во время набора менялось число каналов." << endl;
    cout << "   -v,\t--volt-mode\tИзменить volt mode. По умолчанию диапазон от -0,5 до 0,5 V, изменить на 0 - 1 V" << endl;
    cout << "   -c,\t--max-num\t" << BOLD_SH << "<num>" << ENDCOLOR << " установить максимальное число возможных каналов. Используется в безопасном режиме. По умолчанию 4" << endl;
    cout << "   -e,\t--online\t"<< BOLD_SH << "<sec>" << ENDCOLOR << " online-режим. Задаётся такт таймера в секундах" <<endl;

    exit(0);
}

float tmpf;
string tmpOutDir;
int main(int argc, char** argv)
{
    int opt = 0;
    int longIndex = 0;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex);
    while (opt != -1) {
        switch (opt) {
        case 'h':
            help();
            break;
        case 'd':
            onlydetect = true;
            break;
        case 'm':
            chMode = atoi(argv[optind-1]);
            break;
        case 's':
            safetymode = true;
            break;
        case 'v':
            voltmode = true;
            break;
        case 'c':
            maxNCh = atoi(argv[optind-1]);
            break;
        case 'a':
            tmpf = atof(argv[optind-1]);
            if (tmpf==0) cout << argv[optind-1] <<" factor a must be float " << endl;
            getfactor = getfactor*tmpf;
            break;
        case 'k':
            amplitudekuskoffmode=true;
            break;
        case 'b':
            tmpf = atof(argv[optind-1]);
            if (tmpf==0) cout << argv[optind-1] <<" factor b must be float " << endl;
            getfactorB = getfactorB+tmpf;
            break;
        case 'n':
            isnewoutdir = true;
            tmpOutDir = string(argv[optind-1]);
            break;
        case 'e':
            onlinemode = true;
            timerNsec = atof(argv[optind-1]);
            if (timerNsec<1) timerNsec = 1;
            break;
        case 'r':
            rootapplicationflag=false;
            break;
        case 'o':
            isoutpng=true;
            break;
        case 0:
            if (strcmp("version",longOpts[longIndex].name) == 0)
            {
                cout << "DrsSpectrum Version 2.2" << endl;
                exit(0);
            }
            exit(0);
        default:
            help();
            break;
        }
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    }

    if (argc-optind==0) { cout << "No input file specified\nUse -h for help" << endl; exit(0);}
    if ((argc-optind>1 )&&(argc-optind!=5) ) {cout << " many arguments! " << endl; exit(0);}

    if (argc-optind==5)
    {
        if(onlydetect)
        {
            cerr << RED_SH << " с параметром -d не должно быть дополнительных аргументов" << ENDCOLOR << endl;
            onlydetect = false;
        }

        int tmpval;
        tmpval = fromStr(string(argv[optind+1]));
        get_noise_min = (unsigned short int)tmpval;

        tmpval = fromStr(argv[optind+2]);
        get_noise_max = (unsigned short int)tmpval;

        tmpval = fromStr(argv[optind+3]);
        get_signal_min = (unsigned short int)tmpval;

        tmpval = fromStr(argv[optind+4]);
        get_signal_max = (unsigned short int)tmpval;

        if (get_noise_min<16) get_noise_min=16;
        inputparameters=true;
    }

    string nameInputFile = argv[optind];

    TypeData drsType = DrsReadN::checkTypeOfDrsData(argv[optind]);

    cout << endl;

    if (drsType == TypeData::drs40)
    {
        cout << " Type of data is DRS40 " << endl;
        drs = new Drs4Read(nameInputFile);
    }
    else if (drsType == TypeData::drs450)
    {
        cout << " Type of data is DRS450 " << endl;
        drs = new Drs450Read(nameInputFile);
    }
    else if (drsType == TypeData::unknown)
    {
        cerr << RED_SH << " File format error! (unknown format)" << ENDCOLOR << endl;
        exit(1);
    }

    if (safetymode)
    {
        modes = (modes | 0b10);
        if(maxNCh>0) drs->setMaxNumOfChannels(maxNCh);
        drs->useSafetyMode();
    }
    if (voltmode) modes = (modes | 0b100);
    if (onlydetect) modes = (modes | 0b1);


    if(inputparameters)
    {
        if (singleChannelMode)
            signalProc = new DrsSignalProcN(drs,get_noise_min,get_noise_max,get_signal_min,get_signal_max,modes);
    }
    else
    {
        if(onlydetect || safetymode || voltmode)
        {
            signalProc = new DrsSignalProcN(drs,modes);
        }
        else
        {
            signalProc = new DrsSignalProcN(drs);
        }
    }

//    if (safetymode)
//    {
//        signalProc->setSafetyMode();
//    }

    if (chMode>0 && !safetymode) signalProc->setWorkingChannelsMode(chMode);
    if(getfactor!=0 && (getfactor!=1 || getfactorB!=0)) signalProc->setFactorAndShift(getfactor,getfactorB);

    if (amplitudekuskoffmode) signalProc->setAmplitudeKuskoffMode(true);

    if (onlinemode)
    {
        UseApp();

        //signal(SIGINT,ctrlplusconeline);
#ifdef USETHREADAPP
        std::thread func_thread(online);
#endif
        signalProc->getSpectumOnline(timerNsec);
        //RunApp();
    }
    auto spectr = signalProc->getSpectumOffline();

    if (onlydetect)
    {
        delete signalProc;
        delete drs;
        return 0;
    }

    if(rootapplicationflag) UseApp();
    if (isnewoutdir) signalProc->setOutputDirectory(tmpOutDir);
    if (isoutpng) signalProc->setOutPngFileFlag();
    signalProc->rootProc(spectr);
    if(rootapplicationflag)
    {
        RunApp();
        signal(SIGINT,ctrlplusc);
    }



    delete signalProc;
    delete drs;
}

void ctrlplusc(int sig)
{
    cout << endl;
    cout << "exit" << endl;
    if(rootapplicationflag || onlinemode) {delete signalProc; delete drs; exit(0);}
}


void UseApp()
{
    signal(SIGINT,ctrlplusc);
    myapp = new TApplication("App",0,0);
    cout << "\e[1;33m \e[40m Use CTRL+C to exit!!! \E[0m" << endl;

}

void RunApp()
{
    myapp->Run();
}
