#include "drssignalproc.h"
#include <iostream>
#include <math.h>

using namespace std;

DRSSignalProc::DRSSignalProc()
{
    init();
}

void DRSSignalProc::init()
{
    posorneg = -1; // positive = 1 ; negative = -1;
    VoltMode = 0.5; //-0.5 <> 0.5 V = 0.5 ; 0 <> 1 = 0;
    factor = 1.;
    kuskoff_amplitude=false;
    EventSN = 0;
    MinValOfSignal=0;
    MaxValOfSignal=0;
    numsampl = 1024;
    sumamp.clear();
    for (int i=0;i<numsampl;i++) sumamp.push_back(0);
}

void DRSSignalProc::SetModeIntegral(bool SetMode)
{
    kuskoff_amplitude = SetMode;
}


float DRSSignalProc::getsignal(unsigned short *n_amplitudes, float *n_times)
{
    float amps[numsampl];
    int eventnum=0;
    float noise = 0;
    float signal = 0;
    EventSN++;
    for (int i=0;i<numsampl;i++)
    {
        amps[i] = /*factor**/(n_amplitudes[i]/65535.- VoltMode)/* + factorB*/;
        //amps[i] = factor*amps[i] + factorB;
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
#ifdef DEBUG
                    cout << "Frame " << EventSN << ": no reasonable max found" << endl;
#endif
                    return -1111;
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

            signal = B*B/4/A + C + noise;

            //printf("Parabulum: %g %g %g\n", A, B, C);
    }
    else
    for (int i=signal_min; i<signal_max; i++)
        signal += (((amps[i]/2.+amps[i+1]/2.)- noise)*(n_times[i+1]-n_times[i]));
    signal = posorneg*factor*signal + /*factor**/factorB;
    return signal;
}

void DRSSignalProc::getIntegralSignal(float *int_signal)
{
    for(int i=0;i<numsampl;i++) int_signal[i] = sumamp[i];
}

void DRSSignalProc::getIntegralSignal(vector<float> &SummarySignal)
{
    SummarySignal = sumamp;
}

void DRSSignalProc::SetFactor(float SetFactorAValue, float SetFactorBValue)
{
    // y = factorA*x + factorB;
    factor = SetFactorAValue;
    factorB = SetFactorBValue;

}

void DRSSignalProc::GetFactor(float &GetFactorAValue, float &GetFactorBValue)
{
    GetFactorAValue = factor;
    GetFactorBValue = factorB;
}

void DRSSignalProc::GetMinMaxValOfSignal(std::pair<float, float> &minmax)
{
    minmax.first = MinValOfSignal;
    minmax.second = MaxValOfSignal;
}

void DRSSignalProc::SetMinMaxValOfSignal(float min, float max)
{
    MinValOfSignal= min;
    MaxValOfSignal = max;
}

void DRSSignalProc::SetNumOfSamples(int numofsampes)
{
    if (numofsampes!=numsampl)
    {
        sumamp.clear();
        numsampl=numofsampes;
        for(int i=0;i<numsampl;i++) sumamp.push_back(0);
    }
}


void DRSSignalProc::autoSignalDetectKusskoff(const unsigned short *k_amplitudes,bool endfile)
{
    //int eventnum;
    int  ready_to_overflow = 0, overflow = 0;
    for ( int i = 0; i < numsampl; i++ )
    {
        float amp =  (float)k_amplitudes[i]/65535.-VoltMode;
        if(overflow) {
            if(amp > 0) {
                overflow = 0;
            }
            ready_to_overflow = (amp > 0.35);
        } else
            if(ready_to_overflow) {
                if(amp < -0.25) {
                    overflow = 1;
                } else if (amp < 0.35)
                    ready_to_overflow = 0;
            } else
                if(amp > 0.35)
                    ready_to_overflow = 1;
        if(overflow)
            amp += 1.;

        sumamp[i] += amp/*/numsampl*/;


    }EventSN++;

    if (!endfile) return;

    autoSignalDetectKusskoffProc(EventSN);
    EventSN=0;
}

void DRSSignalProc::autoSignalDetectKusskoffProc(int eventnum)
{
    for (int i=0; i<1024; i++)
        sumamp[i]= sumamp[i]/eventnum;
    float x[124];
    float y[124];

    for(int i=0;i<124;i++) {
      x[i] = i;
      y[i] = 0;
    }

    float integrall = 0;

    for(int i=16;i<1024-16;i++) {
      y[(i-16)/8] += sumamp[i];
      integrall += sumamp[i];
    }

    double p[5];
    p[1] = 0;
    for(int i = 0; i < 10; i++)
      p[1] += y[i];
    p[1] = p[1]/10;

    p[0]= (y[123]-p[1])/124;



    float max = 0;

    for(int i = 0; i<124;i++)
      if(integrall*max <= integrall*y[i]) {
        max = y[i];
      }

    double half_height = (max-p[1])/sqrt(2.718)+p[1];
    int half_height_min = 0, half_height_max = 0;

    for(int i = 0; i<124;i++)
      if(integrall*half_height <= integrall*y[i]) {
        half_height_min = i;
        break;
      }

    for(int i = half_height_min; i<124;i++)
      if(integrall*half_height > integrall*y[i]) {
        half_height_max = i;
        break;
      }

    p[3] = (half_height_max+half_height_min)/2.;
    p[4] = (half_height_max-half_height_min)/2.;

    p[2] = (integrall-p[1]*124-p[0]*(124*124/2))/(2.507*p[4]);

    //FIT???

    signal_min = p[3] - p[4]*2.6;
    signal_max = p[3] + p[4]*2.6;
    noise_min = 0;
    noise_max = p[3] - p[4]*3.3;

    signal_min = 16+8*signal_min;
    signal_max = 16+8*signal_max;
    noise_min = 16+8*noise_min;
    noise_max = 16+8*noise_max;


    if(integrall < 0) posorneg=-1;
    else posorneg=1;
    //factor = posorneg*factor;
}
