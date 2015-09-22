#ifndef DRSREADN_H
#define DRSREADN_H

#include <fstream>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <sys/stat.h>


using std::ifstream;
using std::string;
using std::ios_base;
using std::ios;
using std::endl;
using std::cout;
using std::cerr;
using std::vector;

#include "common.h"

enum class TypeData {drs40, drs450, unknown};

class DrsReadN
{
public:
    DrsReadN();
    virtual ~DrsReadN();

    static TypeData checkTypeOfDrsData(string filename);

    virtual void useSafetyMode();
    virtual bool drsGetFrame(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short int mode) = 0;
    virtual bool drsGetFrameSafety(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short& usedChannels) = 0;
    virtual void drsFileReadInfo() = 0;
    virtual unsigned short int getNumOfChannels();
    virtual unsigned short int getNumOfSamples();
    virtual long int calcNumOfPulses() = 0;
    virtual void setMaxNumOfChannels(short int nCh);
    virtual vector<unsigned long int> countNumOfPulses() = 0;

    virtual void drsFileSeekBegin();
    virtual string getNameOfDataFile();
    virtual std::pair<unsigned long, unsigned long> getTimeStampsOfEvents();

    virtual bool updateFileInfo();
    virtual vector<unsigned short> getChannelsInBoard();


//    virtual void DRSFileEnd();
protected:
    enum class DataMarker {begin,end,body};
    struct tm timeinfo;
    ifstream drsInput;


    virtual void drsStreamOpen(string filename);
    virtual void drsStreamClose();

    virtual void drsCheckFileStream();

    virtual void readTimeInfo(DataMarker position);

    unsigned short int nSamples;
    unsigned short int nChannels;
    unsigned long int sizeOfFile;
    unsigned long int nPulses;

    bool isUpdatedfile; // if file updated = true
    bool isEndFile; // if end of file = true

    unsigned short int maxNumOfChannels;
    string fileDataName;
    string drsMark; // first symbols in the binary data
    string channelMark; // first symbols for new channel
    std::ios::pos_type pos_mark; // stream position
    std::ios::pos_type pos_mark_end; // stream position
    std::pair<unsigned long, unsigned long> timeStamps;
};

#endif // DRSREADN_H
