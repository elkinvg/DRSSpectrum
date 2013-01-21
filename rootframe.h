#ifndef ROOTFRAME_H
#define ROOTFRAME_H

#include <root/TGFrame.h>
#include <root/TGClient.h>

class RootFrame
{
public:
    RootFrame();
    RootFrame(const TGWindow *p, UInt_t w, UInt_t h);
    ~RootFrame();

    TGMainFrame        *frameMain;
};

#endif // ROOTFRAME_H
