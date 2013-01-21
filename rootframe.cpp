#include "rootframe.h"

RootFrame::RootFrame()
{
}


RootFrame::RootFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
    frameMain = new TGMainFrame(p,w,h);
    gClient->WaitFor(frameMain);
}

RootFrame::~RootFrame()
{
    delete frameMain;
}
