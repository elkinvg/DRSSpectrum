#ifndef DRSREAD_H
#define DRSREAD_H
#include <fstream>
#include "drs4outfile.h"

using namespace std;

class DRSread : public DRS4outfile
{
public:
    explicit DRSread(int type);
    ifstream DRSinput;
    long int Nevent;
    void DRS4read(string outfilename);
private:
    int typeofDRS;
};

#endif // DRSREAD_H
