#include "drsread.h"
#include "string.h"
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>

//DRSread::DRSread()
//{
//}

DRSread::DRSread()
{
    sizeoffile = 0;
    FileDataName = "NULL";
}


DRSread::~DRSread()
{
}

void DRSread::DRSFileReadStatusAndInfo(string outfilename)
{
    DRSStreamOpen(outfilename);
    char ifDRSformat[5];
    DRSinput.read((char*)&ifDRSformat[0],4*sizeof(char));
    if (strcmp(ifDRSformat,"EHDR")==0) typeofDRS=DRS4;
    else
    {
        cerr << "File format error! (Not DRS4 format)" << endl;
        DRSStreamClose();
        exit(1);
    }
    DRSinput.seekg(ios::beg);
    endfile = false;
    if (typeofDRS==DRS4) DRS4read(outfilename);
}

void DRSread::DRSFileReadStatusAndInfo(string outfilename, int type)
{
    DRSStreamOpen(outfilename);
    if (type==DRS4)
    {
        typeofDRS = DRS4i;
        DRS4read(outfilename);
        endfile = false;
    }
    else
    {
        cerr << "File format error! (Not DRS4 format)" << endl;
        DRSStreamClose();
        exit(1);
    }
}

void DRSread::DRSFileReadStatusAndInfo()
{
    if (FileDataName=="NULL")
    {
        cerr << " Name of file is unknown ";
        exit(1);
    }
    DRS4read(FileDataName);
    if (ifupdatefile) endfile=false;
}

void DRSread::DRSFileSeekBegin()
{
    if (!DRSinput.good())
    {
        DRSinput.clear();
    }
    DRSinput.seekg(ios::beg);
    mark = DRSinput.tellg();
    endfile = false;
}

bool DRSread::DRSGetFrame(unsigned short *n_amplitudes, float *n_times, short nsamles)
{
    // return true если последний блок в фрейме
    // return false если конец файла, или не весь фрейм прочитан (следующий канал)
    char buffer[5];
    int event_serial;
    if (!DRSinput.good())
    {
        DRSinput.clear();
        DRSinput.seekg(mark,ios::beg);
    }

    DRSinput.read(&buffer[0],4*sizeof(char));
    buffer[4] = '\0';
#ifdef DEBUG_STRUCT
    cout << "B1 " << buffer  /*<< endl*/;
#endif
    if (strcmp(buffer, "EHDR") == 0)
    {
        DRSinput.read((char*)&event_serial,sizeof(int));
        DRSinput.seekg(8*sizeof(short int),ios::cur);
        DRSinput.read((char*)&n_times[0],nsamles*sizeof(float));
        DRSinput.read(&buffer[0],4*sizeof(char));
#ifdef DEBUG_STRUCT
        cout << " B2 " << buffer /*<< flush <<  endl*/;
#endif
        Nevent = event_serial+1;
    }

#ifdef DEBUG_STRUCT
    cout << " event_serial " << event_serial/* << endl*/;
#endif
    DRSinput.read((char*)&n_amplitudes[0],nsamles*sizeof(short int));
    mark = DRSinput.tellg();
    DRSinput.read((char*)&buffer[0],4*sizeof(char));
    if (!DRSinput)
    {
        endfile = true;
#ifdef DEBUG
        cout << " EndFile" << endl;
#endif
    }
    buffer[4] = '\0';
    if (strcmp(buffer, "EHDR") == 0)
    {
        DRSinput.seekg(-4*sizeof(char),ios::cur);
#ifdef DEBUG_STRUCT
        cout << " B3 " << buffer/* << flush*/ << endl;
#endif
        return true;
    }
    else
    {
        DRSinput.seekg(-4*sizeof(char),ios::cur);
        return false;
    }
}

void DRSread::DRSFileEnd()
{
    DRSStreamClose();
}

void DRSread::DRS4read(string outfilename)
{
    char ifDRSformat[5];
    ifDRSformat[4] = '\0';
    if (typeofDRS==DRS4i)
    {
        DRSinput.read((char*)&ifDRSformat[0],4*sizeof(char));
        if (strcmp(ifDRSformat,"EHDR")!=0)
        {
            cerr << "File format error! (Not DRS4 format)" << endl;
            DRSStreamClose();
            exit(1);
        }
        DRSinput.seekg(ios::beg);
        typeofDRS==DRS4;
    }
    struct stat buffer;
    stat(outfilename.c_str(),&buffer);

    if (sizeoffile > buffer.st_size)
    {
        cerr << " FILE ERROR! (file downscale!)" << endl;
        DRSStreamClose();
        exit(2);
    }
    if (sizeoffile < buffer.st_size) ifupdatefile = true;
    if (sizeoffile == buffer.st_size) ifupdatefile = false;
    sizeoffile = buffer.st_size;
    FileDataName = outfilename;
#ifdef DEBUG
    cout << " size: " << sizeoffile << " Bytes " << (float)sizeoffile/1024. << " KiB" << endl;
#endif
//    buffer
}

void DRSread::DRSStreamOpen(string filename)
{
    DRSinput.open(filename.c_str(),ios::binary);

    if(!DRSinput)
    {
        cerr << " file doesn`t exist or error open " << endl;
        DRSStreamClose();
        exit(1);
    }
#ifdef DEBUG
    cout << " file open " << endl;
#endif
}

void DRSread::DRSStreamClose()
{
#ifdef DEBUG
    cout << " file close " << endl;
#endif
    DRSinput.close();
}
