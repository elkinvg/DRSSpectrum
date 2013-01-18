#include "drssignalproc.h"
#include <iostream>
using namespace std;

DRSSignalProc::DRSSignalProc()
{
    init();
}

float DRSSignalProc::getsignal(unsigned short *n_amplitudes, unsigned short *n_times)
{
    float amps[1024];
    int eventnum=0;
    float noise = 0;
    float signal = 0;
    for (int i=0;i<numsampl;i++)
    {
        amps[i] = factor*(n_amplitudes[i]/65535.- VoltMode);
    }
    for (int i=noise_min;i<noise_max;i++)
    {
        noise += amps[i];
        noise = noise / (noise_max - noise_min);
    }
    if (kuskoff_amplitude)
    {
        if(signal_max - signal_min < 50) {
                cerr << "Too small signal range\n";
                return 1;
            }
            double avg[50];
            int nums[50];
            for(int i = 0; i < 50; i++) {
                avg[i] = 0;
                nums[i] = 0;
            }
            for(int i=signal_min; i<signal_max; i++) {
                int idx = 50*(i-signal_min)/(signal_max - signal_min);
                avg[idx] += amps[i];
                nums[idx] += 1;
            }

            for(int i = 0; i < 50; i++) {
                avg[i] = avg[i] / nums[i] * 8;
            }

            double e = 0;
            double d = 0;
            double c = 0;
            double b = 0;
            double a = 0;
            double x0y = 0;
            double x1y = 0;
            double x2y = 0;

                int maxpos = 0;
                double maxval = avg[0];
                for(int i = 1; i < 50; i++)
                  if(avg[i] > maxval) {
                    maxpos = i;
                    maxval = avg[i];
                  }

                if((maxpos < 5) || (maxpos > 45))
                {
                    cout << "Frame " << eventnum << ": no reasonable max found" << endl;
                    return -1;
                }

                for(int i = max(0,maxpos - 8); i<min(50, maxpos + 9); i++) {
                double Xval = i/50.*(signal_max - signal_min)+signal_min;
                e += 1;
                d += Xval;
                c += Xval*Xval;
                b += Xval*Xval*Xval;
                a += Xval*Xval*Xval*Xval;

                x0y += 1*avg[i];
                x1y += Xval*avg[i];
                x2y += Xval*Xval*avg[i];
            }

            double demoninator = -a*c*e+a*d*d+b*b*e-2*b*c*d+c*c*c;

            double A = (d*d - c*e)*x2y + (b*e - c*d)*x1y  + (c*c-b*d) * x0y;
            double B = (b*e - c*d)*x2y + (c*c - a*e)*x1y  + (a*d-b*c) * x0y;
            double C = (c*c - b*d)*x2y + (a*d - b*c)*x1y  + (b*b-a*c) * x0y;
            A = A/demoninator;
            B = B/demoninator;
            C = C/demoninator;

            signal = -B*B/4/A + C - noise;

            //printf("Parabulum: %g %g %g\n", A, B, C);
    }
    else
    for (int i=signal_min; i<signal_max; i++)
        signal += (((amps[i]/2.+amps[i+1]/2.)- noise)*(n_times[i+1]-n_times[i]));
    return signal;
}

void DRSSignalProc::peak(unsigned short *n_amplitudes)
{
}

void DRSSignalProc::init()
{
    posorneg = -1; // positive = 1 ; negative = -1;
    VoltMode = 0.5; //-0.5 <> 0.5 V = 0.5 ; 0 <> 1 = 0;
    factor = 1;
    kuskoff_amplitude=false;
}
