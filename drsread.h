#ifndef DRSREAD_H
#define DRSREAD_H
#include <fstream>
//#include "drstype.h"

using namespace std;
enum DRSTYPE {DRS3=3,DRS4=4,DRS4i=-4};

class DRSread
{
public:
    DRSread();
    /*virtual */~DRSread();

    long int Nevent;
    bool endfile;


    void DRSFileReadStatusAndInfo(string outfilename);
    void DRSFileReadStatusAndInfo(string outfilename,int type);
    void DRSFileReadStatusAndInfo();
    void DRSFileSeekBegin();
    int GetNumberOfChannels();

    bool DRSGetFrame(unsigned short *n_amplitudes, float *n_times, bool readflag=true);
    bool DRSGetFrameFromOneCh(unsigned short *n_amplitudes, float *n_times, short working_channel, bool readflag=true);
    void DRSFileEnd();

private:
    int NumberOfChannels;
    unsigned short int nsamles;
    int typeofDRS;
    ios::pos_type mark;
    ifstream DRSinput;
    string FileDataName;
    unsigned long int sizeoffile;
    bool ifupdatefile; // if file updated = true
    bool isAlwaysCheckNumOfCh; // if allways check of number of channel (all iterations). Default is false
    short int checkNumOfCh();
    void DRS4read(string outfilename);
    void DRSStreamOpen(string filename);
    void DRSStreamClose();
    void drsGetFrameVoltage(unsigned short *n_amplitudes, bool readflag);
};

#endif // DRSREAD_H
