#include "drs4read.h"
#include <sys/stat.h>
#include <algorithm>
#include <iterator>
#include <ctime>

using std::copy;
using std::istream_iterator;

Drs4Read::Drs4Read(string filename)
{
    fileDataName = filename;
    drsMark = drs4Mark;
    nSamples = 1024;
    nChannels = 0;
    nPulses = 0;
    channelMark = drs4chMark;
    maxNumOfChannels = 4;
    timeStamps.first = 0;
    timeStamps.second = 0;
    drsStreamOpen(fileDataName);
    drsFileReadInfo();
}

Drs4Read::~Drs4Read()
{
    if(drsInput.is_open()) drsStreamClose();
}

bool Drs4Read::drsGetFrame(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short mode)
{
    /**
     * \brief Читает данные единичного фрейма
     *
     * \param v_amplitudes Вектор данных (16 битных) показаний осциллографа. Размер вектора пропорционален количеству используемых каналов
     *
     * \param v_times Вектор временнЫх поправок
     *
     * \param mode Режим чтения. Бины mode соответствуют используемым каналам. Данные будут считываться только с используемых каналов
     *
     * \return true если последний блок в фрейме
     * false если конец файла, или не весь фрейм прочитан (следующий канал)
     */

    char buffer[5];
    int event_serial;
    struct tm timeinfo;
    short tYear,tMon,tDay,tHour,tMin,tSec,tmSec;

    drsCheckFileStream();
    if (v_times.size()<nSamples) v_times.resize(nSamples);
    if (v_amplitudes.size()<nSamples*nChannels) v_amplitudes.resize(nSamples*nChannels);
    char buffer2[4];

    std::ios::pos_type tmppos = drsInput.tellg();
    bool isbegin = false;
    if (tmppos==0) isbegin = true;

    drsInput.read(&buffer[0],4*sizeof(char));
    buffer[4] = '\0';
    buffer2[3] = '\0';
    char symbolChNum[1];
    unsigned short curNChannels;

    curNChannels = nChannels;

    if (strcmp(buffer,drsMark.c_str()) != 0)
    {
        cerr << RED_SH <<"File format error! (Not DRS4 format or data error)" << ENDCOLOR << endl;
        drsStreamClose();
        exit(1);
    }
    drsInput.read((char*)&event_serial,sizeof(int));

    drsInput.read((char*)&tYear,sizeof(short));
    drsInput.read((char*)&tMon,sizeof(short));
    drsInput.read((char*)&tDay,sizeof(short));
    drsInput.read((char*)&tHour,sizeof(short));
    drsInput.read((char*)&tMin,sizeof(short));
    drsInput.read((char*)&tSec,sizeof(short));
    drsInput.read((char*)&tmSec,sizeof(short));

    if (isbegin)
    {
        struct tm timeinfo2;
        time_t date2 = mktime(&timeinfo2);
        date2 =  mktime(&timeinfo2);
        // нужно для правильной интерпретации времени. Первое обращение даёт неправильный результат
        // поэтому обращаемся здесь
    }

    drsInput.seekg(sizeof(short int),ios::cur);
    drsInput.read((char*)&v_times[0],nSamples*sizeof(float));

    unsigned short tmpCheckMode = 1;
    unsigned short nCh;

    for (int i=0;i<curNChannels;i++)
    {
        drsInput.read((char*)&buffer2,3*sizeof(char));
        drsInput.read((char*)&symbolChNum,sizeof(char));
        nCh = atoi(symbolChNum);

        if (mode & tmpCheckMode)
            drsInput.read((char*)&v_amplitudes[i*nSamples],nSamples*sizeof(short));
        else
            drsInput.seekg(nSamples*sizeof(short),ios::cur);
        tmpCheckMode = tmpCheckMode << 1;
    }


    if (isbegin)
    {
        timeinfo.tm_year = (int)(tYear - 1900);
        timeinfo.tm_mon = (int)(tMon - 1);
        timeinfo.tm_mday = (int)tDay;
        timeinfo.tm_hour = (int)tHour;
        timeinfo.tm_min = (int)tMin;
        timeinfo.tm_sec = (int)tSec;

        time_t date = mktime(&timeinfo);
        date =  mktime(&timeinfo);
        timeStamps.first = date;//*1000+tmSec;
    }


    pos_mark = drsInput.tellg();
    drsInput.read((char*)&buffer[0],4*sizeof(char));
    if (!drsInput)
    {
        timeinfo.tm_year = tYear - 1900;
        timeinfo.tm_mon = tMon - 1;
        timeinfo.tm_mday = tDay;
        timeinfo.tm_hour = tHour;
        timeinfo.tm_min = tMin;
        timeinfo.tm_sec = tSec;

        time_t date = mktime(&timeinfo);
        timeStamps.second = (unsigned long)date;//*1000+tmSec;
        isEndFile = true;
        return isEndFile;
    }

    isEndFile = false;
    drsInput.seekg(pos_mark);
    return isEndFile;
}

bool Drs4Read::drsGetFrameSafety(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short& usedChannels)
{
    /**
     * \brief Читает данные единичного фрейма.
     *
     * Используется безопасный режим. То есть данные считываются в не зависимости от того, менялось ли число каналов во время набора. Данный режим более медленный. Применять только при необходмости
     *
     * \param v_amplitudes Вектор данных (16 битных) показаний осциллографа. Размер вектора пропорционален общему количеству каналов
     *
     * \param v_times Вектор временнЫх поправок
     *
     * \param maxNumOfChannels Максимальное число возможных каналов. По умолчанию 4.
     *
     * \param usedChannels Записываются флаги используемых каналов. 0b0110 для 2го и 3го.
     *
     * \return true если последний блок в фрейме
     * false если конец файла, или не весь фрейм прочитан (следующий канал)
     */


    char buffer[5];
    char buffer2[4];
    int event_serial;
    struct tm timeinfo;
    short tYear,tMon,tDay,tHour,tMin,tSec,tmSec;
    usedChannels = 0;

    std::ios::pos_type tmppos = drsInput.tellg();
    bool isbegin = false;
    if (tmppos==0) isbegin = true;

    drsCheckFileStream();

//    unsigned short curNChannels;
//    curNChannels = maxNumOfChannels;


    if (v_times.size()<nSamples) v_times.resize(nSamples);
    if (v_amplitudes.size()<nSamples*maxNumOfChannels) v_amplitudes.resize(nSamples*maxNumOfChannels);

    drsInput.read(&buffer[0],4*sizeof(char));
    buffer[4] = '\0';
    buffer2[3] = '\0';
    char symbolChNum[1];




    if (strcmp(buffer,drsMark.c_str()) != 0)
    {
        cerr << RED_SH << "File format error! (Not DRS4 format or data error)" << ENDCOLOR << endl;
        drsStreamClose();
        exit(1);
    }
    drsInput.read((char*)&event_serial,sizeof(int));

    drsInput.read((char*)&tYear,sizeof(short));
    drsInput.read((char*)&tMon,sizeof(short));
    drsInput.read((char*)&tDay,sizeof(short));
    drsInput.read((char*)&tHour,sizeof(short));
    drsInput.read((char*)&tMin,sizeof(short));
    drsInput.read((char*)&tSec,sizeof(short));
    drsInput.read((char*)&tmSec,sizeof(short));

    if (isbegin)
    {
        struct tm timeinfo2;
        time_t date2 = mktime(&timeinfo2);
        date2 =  mktime(&timeinfo2);
        // нужно для правильной интерпретации времени. Первое обращение даёт неправильный результат
        // поэтому обращаемся здесь
    }

    drsInput.seekg(sizeof(short int),ios::cur);
    drsInput.read((char*)&v_times[0],nSamples*sizeof(float));

    short nCh;

    unsigned short tmpCheckMode;
    while (true) {
        tmpCheckMode = 1;
        drsInput.read((char*)&buffer2,3*sizeof(char));
        drsInput.read((char*)&symbolChNum,sizeof(char));
        nCh = atoi(symbolChNum);
        drsInput.read((char*)&v_amplitudes[(nCh-1)*nSamples],nSamples*sizeof(short));
        tmpCheckMode = tmpCheckMode << (nCh-1);
        usedChannels = (usedChannels | tmpCheckMode);


        tmppos =  drsInput.tellg();
        drsInput.read(&buffer[0],4*sizeof(char));
        buffer[4] = '\0';
        if(!drsInput) {isEndFile = true;break;}

        if(strcmp(buffer,drsMark.c_str())==0)
        {
            drsInput.seekg(tmppos);
            break;
        }
        drsInput.seekg(tmppos);
    }


    if (isbegin)
    {
        timeinfo.tm_year = (int)(tYear - 1900);
        timeinfo.tm_mon = (int)(tMon - 1);
        timeinfo.tm_mday = (int)tDay;
        timeinfo.tm_hour = (int)tHour;
        timeinfo.tm_min = (int)tMin;
        timeinfo.tm_sec = (int)tSec;

        time_t date = mktime(&timeinfo);
        date =  mktime(&timeinfo);
        timeStamps.first = date;//*1000+tmSec;
    }

    pos_mark = drsInput.tellg();
    drsInput.read((char*)&buffer[0],4*sizeof(char));
    if (!drsInput)
    {
        timeinfo.tm_year = tYear - 1900;
        timeinfo.tm_mon = tMon - 1;
        timeinfo.tm_mday = tDay;
        timeinfo.tm_hour = tHour;
        timeinfo.tm_min = tMin;
        timeinfo.tm_sec = tSec;

        time_t date = mktime(&timeinfo);
        timeStamps.second = date;
        isEndFile = true;
        return isEndFile;
    }

    isEndFile = false;
    drsInput.seekg(pos_mark);
    return isEndFile;
}

void Drs4Read::drsFileReadInfo()
{
    /**
     \brief Информация о файле данных: число каналов, тип

     После получения информации ставит указатель ввода в начало файла
     */
    char mark[5];
    mark[4] = '\0';

    drsInput.seekg(4*sizeof(char));

    drsInput.seekg((6+nSamples+nSamples/2)*sizeof(float),ios_base::cur);
    drsInput.read((char*)&mark,4*sizeof(char));
    mark[4] = '\0';
    if (strcmp(mark,drsMark.c_str())==0) nChannels=1;
    else
    {
        drsInput.seekg((nSamples/2)*sizeof(float),ios_base::cur);
        drsInput.read((char*)&mark,4*sizeof(char));
        mark[4] = '\0';
        if (strcmp(mark,drsMark.c_str())==0) nChannels=2;
        else
        {
            drsInput.seekg((nSamples/2)*sizeof(float),ios_base::cur);
            drsInput.read((char*)&mark,4*sizeof(char));
            mark[4] = '\0';
            if (strcmp(mark,drsMark.c_str())==0) nChannels=3;
            else
            {
                drsInput.seekg((nSamples/2)*sizeof(float),ios_base::cur);
                drsInput.read((char*)&mark,4*sizeof(char));
                mark[4] = '\0';
                if (strcmp(mark,drsMark.c_str())==0) nChannels=4;
                else cerr << RED_SH <<" Channels more than 4 "  << ENDCOLOR << endl;
            }
        }
    }

    drsFileSeekBegin();

    struct stat buffer;
    stat(fileDataName.c_str(),&buffer);

    sizeOfFile = buffer.st_size;

    cout << endl;
    cout << "----------"<< endl;
    cout << YELLOW_SH <<"DRS File Info:" << ENDCOLOR << endl;
    cout << " Name of input file\t\t" << BLUE_SH <<fileDataName << ENDCOLOR << endl;
    cout << " Type\t\t\t\t" << BLUE_SH  << "DRS4" << ENDCOLOR << endl;
    cout << " Calculated number of channels\t" << BLUE_SH << nChannels << ENDCOLOR << endl;
    cout << " Calculated number of pulses\t" << BLUE_SH << calcNumOfPulses() << ENDCOLOR << endl;
    cout << endl;
    cout << " Size: " << GREEN_SH << sizeOfFile << ENDCOLOR << " Bytes " << GREEN_SH << (float)sizeOfFile/1024. << ENDCOLOR << " KiB" << endl;
    cout << "----------"<< endl;
}

void Drs4Read::setMaxNumOfChannels(short nCh)
{
    maxNumOfChannels = nCh;
}

unsigned short Drs4Read::getNumOfChannels()
{
    /**
      \brief Возвращает число задействованных каналов
      */
    return nChannels;
}

unsigned short Drs4Read::getNumOfSamples()
{
    /**
      \brief Возвращает число сэмплов в единичном импульсе
      */
    return nSamples;
}

long Drs4Read::calcNumOfPulses()
{
    /**
      \brief Возвращает рассчитанное число фреймов (импульсов). Совпадает с реальным, если в процессе записи не менялось число каналов
      */
    int onePulse,onePulseHead,onePulseCh;
    if (!nPulses)
    {
        onePulseHead = 4*sizeof(char) + sizeof(int) + 8*sizeof(short int) + nSamples*sizeof(float);
        onePulseCh = 0;

        for (int i=0;i<nChannels;i++) onePulseCh += ( 4*sizeof(char) + nSamples*sizeof(short int));

        onePulse = onePulseCh + onePulseHead;
        nPulses = sizeOfFile/onePulse;
    }

    return nPulses;
}

vector<unsigned long> Drs4Read::countNumOfPulses()
{
    /**
      \brief Считает число фреймов (импульсов). Применяется в безопасном режиме.
      */
    char buffer[5];
    drsCheckFileStream();

    nChannels = maxNumOfChannels;
    unsigned long int numofpulses = 0;
    short int maxnumofch,minnuofch,tmpnumofch = 0;
    bool b = true;

    vector<unsigned long> chPulses;
    chPulses.resize(nChannels);
    char n[1];

    while (b) {
        tmpnumofch = 0;
        if (!numofpulses)
        {
            drsInput.read(&buffer[0],4*sizeof(char));
            buffer[4] = '\0';
            if (strcmp(buffer,drsMark.c_str()) != 0)
            {
                cerr << RED_SH << "File format error! (Not DRS4 format or data error)" << ENDCOLOR << endl;
                drsStreamClose();
                exit(1);
            }
        }
        numofpulses++;

        drsInput.seekg(5*sizeof(float),ios::cur);
        drsInput.seekg(nSamples*sizeof(float),ios::cur);

        while (true) {
            drsInput.read(&buffer[0],4*sizeof(char));
            if (!drsInput) {;b=false;break;}

            if (strcmp(buffer,drsMark.c_str()) != 0)
            {
                n[0] = buffer[3];
                buffer[3]='\0';
                if (strcmp(buffer,channelMark.c_str()) !=0)
                {
                    cerr << RED_SH << "File format error! (Not DRS4 format or data error)" << ENDCOLOR << endl;
                    drsStreamClose();
                    exit(1);
                }
                int tmpi = atoi(n);
                if (tmpi>nChannels)
                {
                    cerr << RED_SH <<"File format error! (Not DRS4 format or data error)" << ENDCOLOR << endl;
                    drsStreamClose();
                    exit(1);
                }
                chPulses[tmpi-1]++;
            }
            else {
                break;
            }
            drsInput.seekg(nSamples*sizeof(short),ios::cur);
        }
    }
    drsFileSeekBegin();
    return chPulses;
}
