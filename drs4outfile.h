#ifndef DRS4OUTFILE_H
#define DRS4OUTFILE_H

class DRS4outfile
{
public:
    char EHDR = "EHDR\0";
    int event_serial;
    unsigned short event_year, event_month, event_day, event_hour,
    event_minute, event_second, event_msec, event_reserved;
    float times[1024];
    unsigned short amplitudes[1024];
    DRS4outfile();
};

#endif // DRS4OUTFILE_H
