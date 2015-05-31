#include "drsreadn.h"

DrsReadN::DrsReadN()
{
}

DrsReadN::~DrsReadN()
{
}

TypeData DrsReadN::checkTypeOfDrsData(string filename)
{
    /**
      \brief Статическая функция предназначенная для определения типа данных.
      После добавления новых типов следует править эту функцию и добавить новый тип в TypeData.

      \param filename имя входного файла

      \return Тип данных, либо определенный тип DRS, либо unknown

      */

    ifstream dataIn;
    dataIn.open(filename.c_str(), ios_base::binary);
    TypeData type;

    if (!dataIn)
    {
        cerr << RED_SH << " file doesn`t exist or error open " << ENDCOLOR << endl;
        dataIn.close();
        exit(1);
    }
    char ifDRSformat[5];
    dataIn.read((char*)&ifDRSformat,4*sizeof(char));
    ifDRSformat[4]='\0';

    if (strcmp(ifDRSformat,drs4Mark.c_str())==0)
    {
        type = TypeData::drs40;
    }
    else if (strcmp(ifDRSformat,drs450Mark.c_str())==0)
    {
        type = TypeData::drs450;
    }
    else
    {
        type = TypeData::unknown;
    }
    dataIn.close();
    return type;
}

void DrsReadN::useSafetyMode()
{
    /**
      \brief Включить безопасный режим. Использовать, если количество используемых каналов изменялось во время набора
      */
    cout << YELLOW_SH << "Safety mode " << ENDCOLOR << endl;
    cout << endl;
    nChannels = maxNumOfChannels;
    cout << " Used number of channels\t"<< BLUE_SH << nChannels << ENDCOLOR << endl;
    cout << "----------" << endl;
}

unsigned short DrsReadN::getNumOfChannels()
{
    /**
      \brief Возвращает число задействованных каналов
    */
    return nChannels;
}

unsigned short DrsReadN::getNumOfSamples()
{
    /**
      \brief Возвращает число сэмплов в единичном импульсе
    */
    return nSamples;
}

void DrsReadN::setMaxNumOfChannels(short nCh)
{
    maxNumOfChannels = nCh;
}

void DrsReadN::drsStreamOpen(string filename)
{
    drsInput.open(filename.c_str(), ios_base::binary);

    if (!drsInput)
    {
        cerr << RED_SH << " File doesn`t exist or error open " << ENDCOLOR << endl;
        drsInput.close();
        exit(1);
    }
    drsInput.seekg (0, ios::end);
    pos_mark_end = drsInput.tellg();
    drsInput.seekg(ios_base::beg);

}

void DrsReadN::drsStreamClose()
{
    cout << " File close " << endl;
    drsInput.close();
}

void DrsReadN::drsFileSeekBegin()
{
    if (!drsInput.good())
    {
        drsInput.clear();
    }
    drsInput.seekg(ios_base::beg);
    pos_mark = drsInput.tellg();
    isEndFile = false;
}

std::string DrsReadN::getNameOfDataFile()

{
    return fileDataName;
}

/*vector<long>*/std::pair<unsigned long,unsigned long> DrsReadN::getTimeStampsOfEvents()
{
    /**
      \brief Возвращает вектор TimeStamps событий. Записаны в секундах с 1.1.1970

      \return вектор TimeStamps событий
      */
    return timeStamps;
}

bool DrsReadN::updateFileInfo()
{
    struct stat buffer;
    stat(fileDataName.c_str(),&buffer);

    unsigned long size = buffer.st_size;
    if (size > sizeOfFile)
    {
//        cout << " file updated " << endl;
        sizeOfFile = size;
        isUpdatedfile = true;
//        cout << " file new size: " << sizeOfFile << endl;
    }
    else
    {
        isUpdatedfile = false;
    }
    return isUpdatedfile;
}

void DrsReadN::drsCheckFileStream()
{
    pos_mark = drsInput.tellg();
    if (!drsInput.good())
    {
        drsInput.clear();
    }
    drsInput.seekg(pos_mark,ios_base::beg);
}

void DrsReadN::readTimeInfo(DataMarker position)
{
    /* If isBegin = true is begin of reading of data
     * if isBegin = false is end
     */
//    struct tm timeinfo;
    short tYear,tMon,tDay,tHour,tMin,tSec,tmSec;


    if (position == DataMarker::begin || position == DataMarker::end)
    {
        drsInput.read((char*)&tYear,sizeof(short));
        drsInput.read((char*)&tMon,sizeof(short));
        drsInput.read((char*)&tDay,sizeof(short));
        drsInput.read((char*)&tHour,sizeof(short));
        drsInput.read((char*)&tMin,sizeof(short));
        drsInput.read((char*)&tSec,sizeof(short));
        drsInput.read((char*)&tmSec,sizeof(short));
        drsInput.seekg(sizeof(short int),ios::cur);

        timeinfo.tm_year = (int)(tYear - 1900);
        timeinfo.tm_mon = (int)(tMon - 1);
        timeinfo.tm_mday = (int)tDay;
        timeinfo.tm_hour = (int)tHour;
        timeinfo.tm_min = (int)tMin;
        timeinfo.tm_sec = (int)tSec;
    }
    if (position == DataMarker::body)
    {
        drsInput.seekg(8*sizeof(short int),ios::cur);
    }

    if (position == DataMarker::begin)
    {
        time_t date = mktime(&timeinfo);
        //date =  mktime(timeinfo);
        timeStamps.first = date;//*1000+tmSec;
    }
    if (position == DataMarker::end)
    {
        time_t date = mktime(&timeinfo);
        timeStamps.second = (unsigned long)date;//*1000+tmSec;
    }

}
