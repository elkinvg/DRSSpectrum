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

void DrsReadN::drsStreamOpen(string filename)
{
    drsInput.open(filename.c_str(), ios_base::binary);

    if (!drsInput)
    {
        cerr << RED_SH << " File doesn`t exist or error open " << ENDCOLOR << endl;
        drsInput.close();
        exit(1);
    }

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
      \brief Возвращает вектор TimeStamps событий. Записаны в миллисекундах с 1.1.1970

      \return вектор TimeStamps событий
      */
    return timeStamps;
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
