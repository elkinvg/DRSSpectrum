#ifndef DRSREAD_H
#define DRSREAD_H
#include <fstream>
#include "drstype.h"

using namespace std;
enum DRSTYPE {DRS3=3,DRS4=4,DRS4i=-4};

class DRSread
{
public:
    /*explicit */
    DRSread();
    ~DRSread();
    ifstream DRSinput;
    long int Nevent;
    void DRSFileRead(string outfilename);
    void DRSFileRead(string outfilename,int type);

    int typeofDRS;
private:
    void DRS4read(string outfilename);
    void DRSStreamOpen(string filename);
    void DRSStreamClose();
};

#endif // DRSREAD_H
