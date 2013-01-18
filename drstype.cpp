#include "drstype.h"

DRS4::DRS4()
{
}

float DRS4::getfreq(unsigned short *n_times)
{
    float freq = 0;
    for (int i=1;i<numsampl;i++)
    {
        freq = freq + (n_times[i]-n_times[i-1]);
    }
    return freq/(numsampl-1);
}

//unsigned short DRS4::getsignal(unsigned short *n_amplitudes)
//{
//}
