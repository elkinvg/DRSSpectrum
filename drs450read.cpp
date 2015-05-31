#include "drs450read.h"

Drs450Read::Drs450Read(string filename)
{
    fileDataName = filename;
    drsMark = drs450newHeadMark;
    nSamples = 1024;
    nChannels = 0;
    nPulses = 0;
    channelMark = drs450chMark;
    maxNumOfChannels = 4;
    timeStamps.first = 0;
    timeStamps.second = 0;
    drsStreamOpen(fileDataName);
    drsFileReadInfo();
}

void Drs450Read::useSafetyMode()
{
    /**
      * Безопасный режим для данных DRS450. Используется стандартный режим для первого канала.
      */
    cerr << RED_SH <<"DRS450 does not have a safety mode. Use standart mode " << ENDCOLOR << endl;
}

Drs450Read::~Drs450Read()
{
    if(drsInput.is_open()) drsStreamClose();
}

void Drs450Read::drsFileReadInfo()
{
    /**
     \brief Информация о файле данных: число каналов, тип

     После получения информации ставит указатель ввода в начало файла
     */
    char mark[5];
    mark[4] = '\0';
    unsigned short int boardNum;
    std::ios::pos_type tmppo;

    drsInput.seekg(6*sizeof(char));
    drsInput.read((char*)&boardNum,sizeof(short));
    drsInput.seekg((nSamples+1)*sizeof(float),ios_base::cur);tmppo=drsInput.tellg();
    drsInput.read((char*)&mark,4*sizeof(char));

//    drsInput.seekg((6+nSamples+nSamples/2)*sizeof(float),ios_base::cur);
//    drsInput.read((char*)&mark,4*sizeof(char));
//    mark[4] = '\0';
    if (strcmp(mark,drsMark.c_str())==0) nChannels=1;
    else
    {
        drsInput.seekg((nSamples)*sizeof(float),ios_base::cur);tmppo=drsInput.tellg();
        drsInput.read((char*)&mark,4*sizeof(char));
        mark[4] = '\0';
        if (strcmp(mark,drsMark.c_str())==0) nChannels=2;
        else
        {
            drsInput.seekg((nSamples)*sizeof(float),ios_base::cur);tmppo=drsInput.tellg();
            drsInput.read((char*)&mark,4*sizeof(char));
            mark[4] = '\0';
            if (strcmp(mark,drsMark.c_str())==0) nChannels=3;
            else
            {
                drsInput.seekg((nSamples)*sizeof(float),ios_base::cur);tmppo=drsInput.tellg();
                drsInput.read((char*)&mark,4*sizeof(char));
                mark[4] = '\0';
                if (strcmp(mark,drsMark.c_str())==0) nChannels=4;
                else cerr << RED_SH <<" Channels more than 4 "  << ENDCOLOR << endl;
            }
        }
    }
    pos_beginofdata = tmppo;

    drsFileSeekBegin();

    struct stat buffer;
    stat(fileDataName.c_str(),&buffer);

    sizeOfFile = buffer.st_size;

    cout << endl;
    cout << "----------"<< endl;
    cout << YELLOW_SH <<"DRS File Info:" << ENDCOLOR << endl;
    cout << " Name of input file\t\t" << BLUE_SH <<fileDataName << ENDCOLOR << endl;
    cout << " Type\t\t\t\t" << BLUE_SH  << "DRS450" << ENDCOLOR << endl;
    cout << " Calculated number of channels\t" << BLUE_SH << nChannels << ENDCOLOR << endl;
    cout << " Calculated number of pulses\t" << BLUE_SH << calcNumOfPulses() << ENDCOLOR << endl;
    cout << endl;
    cout << " Size: " << GREEN_SH << sizeOfFile << ENDCOLOR << " Bytes " << GREEN_SH << (float)sizeOfFile/1024. << ENDCOLOR << " KiB" << endl;
    cout << "----------"<< endl;
}

long Drs450Read::calcNumOfPulses()
{
    /**
      \brief Возвращает рассчитанное число фреймов (импульсов). Совпадает с реальным, если в процессе записи не менялось число каналов
      */
    unsigned long int onePulse,onePulseHead,onePulseCh;
    unsigned long int sizeOfHead;

    sizeOfHead = 8 + nChannels*(4+nSamples*sizeof(float));


    if (!nPulses || isUpdatedfile)
    {
        onePulseHead = 4*sizeof(char) + sizeof(int) + 12*sizeof(short int);
        onePulseCh = 0;

        for (int i=0;i<nChannels;i++) onePulseCh += ( 4*sizeof(char) + nSamples*sizeof(short int));

        onePulse = onePulseCh + onePulseHead;
        nPulses = (sizeOfFile-sizeOfHead)/onePulse;
    }
    return nPulses;
}

bool Drs450Read::drsGetFrame(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short mode)
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
     *
     * Во избежание многоразового копирования, данные в v_times записываются только один раз,
     * так как временнЫе данные по сэмплам в DRS450 хранятся в одном месте - в заголовке общих
     * бинарных данных.
     * Для поддержания стабильности кода, не рекомендуется изменять содержимое v_times вне класса.
     */

    char buffer[5];
    int event_serial;
    std::ios::pos_type tmppos;

    drsCheckFileStream();
    if (v_times.size()<nSamples) v_times.resize(nSamples);
    if (v_amplitudes.size()<nSamples*nChannels) v_amplitudes.resize(nSamples*nChannels);
    char buffer2[4];


    DataMarker position;

    //tmp
    if (!drsInput) // for online mode
    {
        drsInput.close();
        drsInput.open(fileDataName.c_str(),ios_base::binary);
        drsInput.seekg (0, ios::end);
        ios::pos_type pos_marktmp = drsInput.tellg();
        drsInput.seekg(ios_base::beg);
        drsInput.seekg(pos_mark_end);
        pos_mark_end = pos_marktmp;
    }
    //tmp


    drsInput.read(&buffer[0],4*sizeof(char));
    buffer[4] = '\0';
    buffer2[3] = '\0';
    char symbolChNum[1];
    unsigned short curNChannels;


    if (strcmp(buffer,drs450Mark.c_str()) == 0)
    {
        fillTimeBinData();
        drsInput.seekg(pos_beginofdata);
        drsInput.read(&buffer[0],4*sizeof(char));
    }


    if (strcmp(buffer,drsMark.c_str()) != 0)
    {
        cerr << RED_SH << "File format error! (Not DRS format or data error)" << ENDCOLOR << endl;
        drsStreamClose();
        exit(1);
    }

    curNChannels = nChannels;

    drsInput.read((char*)&event_serial,sizeof(int));

    if (event_serial == 1) position = DataMarker::begin;
    else if (event_serial == nPulses) position = DataMarker::end;
    else position = DataMarker::body;

    readTimeInfo(position);
    drsInput.seekg(8*sizeof(char),ios::cur);

    if (event_serial == 1) v_times = timeBinData;


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



    pos_mark = drsInput.tellg();
    drsInput.read((char*)&buffer[0],4*sizeof(char));
    if (!drsInput)
    {
        isEndFile = true;
        return isEndFile;
    }

    isEndFile = false;
    drsInput.seekg(pos_mark);
    return isEndFile;


    //v_times = timeBinData[0];
}

bool Drs450Read::drsGetFrameSafety(vector<unsigned short> &v_amplitudes, vector<float> &v_times, unsigned short &usedChannels)
{
    /**
      * Безопасный режим для данных DRS450. Используется стандартный режим для первого канала.
      */

    drsGetFrame(v_amplitudes,v_times,1);
}

vector<unsigned long> Drs450Read::countNumOfPulses()
{
    vector<unsigned long> chPulses;
    chPulses.resize(nChannels);
    for (int i=0;i<nChannels;i++) chPulses[i] = nPulses;
    return chPulses;
}

void Drs450Read::fillTimeBinData()
{
    if (timeBinData.size() < nChannels*nSamples) timeBinData.resize(nChannels*nSamples);

    vector<float> tmpFloat; tmpFloat.resize(nSamples);

    drsInput.seekg(0);
    drsInput.seekg(4*2*sizeof(char),ios::cur);

    for (int i=0;i<nChannels;i++)
    {
        drsInput.seekg(4*sizeof(char),ios::cur);
        drsInput.read((char*)&tmpFloat[0],nSamples*sizeof(float));
        for (int j=0;j<nSamples;j++)
        {
            if(!j) timeBinData[i*nSamples + j] = 0;
            else
            {
                timeBinData[i*nSamples + j] = timeBinData[i*nSamples + j -1] + tmpFloat[j];
            }
        }
    }
}
