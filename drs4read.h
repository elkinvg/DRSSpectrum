#ifndef DRS4READ_H
#define DRS4READ_H

#include "drsreadn.h"

class Drs4Read : public DrsReadN
{
public:
    explicit Drs4Read(string filename);
    virtual ~Drs4Read();
    Drs4Read(const Drs4Read&) = delete;
    Drs4Read& operator=(const Drs4Read&) = delete;

    bool drsGetFrame(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short int mode) override;
    bool drsGetFrameSafety(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short &usedChannels);
    void drsFileReadInfo() override;
    void setMaxNumOfChannels(short int nCh) override;
    unsigned short int getNumOfChannels() override;
    unsigned short int getNumOfSamples() override;
    long calcNumOfPulses() override;
    vector<unsigned long int> countNumOfPulses() override;

private:


};

#endif // DRS4READ_H
