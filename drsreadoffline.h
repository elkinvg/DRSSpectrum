#ifndef DRSREADOFFLINE_H
#define DRSREADOFFLINE_H

#include "drsread.h"


class DRSReadOffline : public DRSread
{
public:
    DRSReadOffline();
    DRSReadOffline(int type);
    void ReadDRSFile(string filename);
};

#endif // DRSREADOFFLINE_H
