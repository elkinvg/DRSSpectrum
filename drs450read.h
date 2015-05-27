#ifndef DRS450READ_H
#define DRS450READ_H

#include "drsreadn.h"

class Drs450Read : public DrsReadN
{
public:
    Drs450Read(string filename);
    virtual ~Drs450Read();
    Drs450Read(const Drs450Read&) = delete;
    Drs450Read& operator=(const Drs450Read&) = delete;

    virtual void useSafetyMode() override;
    void drsFileReadInfo() override;
    long int calcNumOfPulses() override;
    bool drsGetFrame(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short int mode) override;
    bool drsGetFrameSafety(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short &usedChannels) override;
    vector<unsigned long int> countNumOfPulses() override;

private:

    std::ios::pos_type pos_beginofdata;
    vector<float> timeBinData;
    void fillTimeBinData();
};

#endif // DRS450READ_H
