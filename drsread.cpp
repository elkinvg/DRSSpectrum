#include "drsread.h"
#include "string.h"
#include <iostream>
#include <stdlib.h>

//DRSread::DRSread()
//{
//}

DRSread::DRSread()
{
}


DRSread::~DRSread()
{
}

void DRSread::DRSFileRead(string outfilename)
{
    DRSStreamOpen(outfilename);
    char ifDRSformat[5];
    DRSinput.read((char*)&ifDRSformat[0],5*sizeof(char));
    if (strcmp(ifDRSformat,"EHDR")==0) typeofDRS=DRS4;
    else
    {
        cerr << "File format error! (Not DRS4 format)" << endl;
        DRSinput.close();
        exit(1);
    }
    DRSinput.seekg(ios_base::beg);
    if (typeofDRS==DRS4) DRS4read(outfilename);
}

void DRSread::DRSFileRead(string outfilename, int type)
{
    DRSStreamOpen(outfilename);
    if (type==DRS4)
    {
        typeofDRS = DRS4i;
        DRS4read(outfilename);
    }
}

void DRSread::DRS4read(string outfilename)
{
    char ifDRSformat[5];
    if (typeofDRS==DRS4i)
    {
        DRSinput.read((char*)&ifDRSformat[0],5*sizeof(char));
        if (strcmp(ifDRSformat,"EHDR")!=0)
        {
            cerr << "File format error! (Not DRS4 format)" << endl;
            DRSinput.close();
            exit(1);
        }
        DRSinput.seekg(ios_base::beg);
        typeofDRS==DRS4;
    }
}

void DRSread::DRSStreamOpen(string filename)
{
    DRSinput.open(filename.c_str(),ios_base::binary);

    if(!DRSinput)
    {
        cerr << " file doesn`t exist or error open " << endl;
        DRSinput.close();
        exit(1);
    }
}

void DRSread::DRSStreamClose()
{
    DRSinput.close();
}
