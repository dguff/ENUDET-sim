#include "event/SLArEventCRT.hh"

ClassImp(SLArEventCRT);

SLArEventCRT::SLArEventCRT() : fCRTNo(0),
                               fPDG(0),
                               fTime(0),
                               fEkin(0),
                               fLocalPos(0,0,0),
                               fGlobalPos(0,0,0),
                               fDirection(0,0,0)
{
}

SLArEventCRT::~SLArEventCRT()
{
}

void SLArEventCRT::SetLocalPos(double x, double y, double z)
{
    fLocalPos.SetX(x);
    fLocalPos.SetY(y);
    fLocalPos.SetZ(z);
}

void SLArEventCRT::SetGlobalPos(double x, double y, double z)
{
    fGlobalPos.SetX(x);
    fGlobalPos.SetY(y);
    fGlobalPos.SetZ(z);
}

void SLArEventCRT::SetDir(double dx, double dy, double dz)
{
    fDirection.SetX(dx);
    fDirection.SetY(dy);
    fDirection.SetZ(dz);
}