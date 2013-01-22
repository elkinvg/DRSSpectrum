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
    ifstream DRSinput;
    long int Nevent;
    bool endfile;
    ios::pos_type mark;
    string FileDataName;

    /*virtual */void DRSFileReadStatusAndInfo(string outfilename);
    /*virtual */void DRSFileReadStatusAndInfo(string outfilename,int type);
    void DRSFileReadStatusAndInfo();
    void DRSFileSeekBegin();
    bool DRSGetFrame(unsigned short *n_amplitudes, float *n_times, short int nsamles=1024);
    void DRSFileEnd();
    int typeofDRS;
private:
    unsigned long int sizeoffile;
    bool ifupdatefile; // if file updated = true
    void DRS4read(string outfilename);
    void DRSStreamOpen(string filename);
    void DRSStreamClose();
};

#endif // DRSREAD_H
