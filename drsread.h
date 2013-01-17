#ifndef DRSREAD_H
#define DRSREAD_H
#include <fstream>
#include "drs4outfile.h"

using namespace std;

class DRSread : public DRS4outfile
{
public:
    DRSread();
    ifstream DRSinput;
    long int Nevent;
};

#endif // DRSREAD_H
