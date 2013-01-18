#include "drsread.h"
#include "string.h"
#include <iostream>
#include <stdlib.h>

//DRSread::DRSread()
//{
//}

DRSread::DRSread(int type)
{
    typeofDRS = type;
}

void DRSread::DRS4read(string outfilename)
{
    DRSinput.open(outfilename.c_str(),ios_base::binary);
    char ifDRSformat[5];
    DRSinput.read((char*)&ifDRSformat[0],5*sizeof(char));
    if (strcmp(ifDRSformat,"EHDR")!=0)
    {
        cerr << "File format error! (Not DRS4 format)" << endl;
        DRSinput.close();
        exit(1);
    }
    DRSinput.seekg(ios_base::beg);
}
