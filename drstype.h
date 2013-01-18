#ifndef DRSTYPE_H
#define DRSTYPE_H

const int numsampl =1024;

class DRS4
{
public:
    DRS4();

    char EHDR[5];// = "EHDR\0";
    int event_serial;
    unsigned short event_year, event_month, event_day, event_hour,
    event_minute, event_second, event_msec, event_reserved;
    float times[numsampl];
    unsigned short amplitudes[numsampl];
    unsigned short numofchannel;
    //double freq;
    float meanfreq;

    float getfreq(unsigned short *n_times);
//    unsigned short getsignal(unsigned short *n_amplitudes);
};

#endif // DRSTYPE_H
