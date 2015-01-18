#ifndef WHEELRPMS_H
#define WHEELRPMS_H
#include <iosfwd>
#include "shooter_status.h"
//#include "../util/maybe.h"

struct wheelcalib{
	Shooter_wheels::Status highgoal;
	Shooter_wheels::Status lowgoal;
	Shooter_wheels::Status overtruss;
	Shooter_wheels::Status passing;
};

bool operator==(wheelcalib,wheelcalib);
bool operator!=(wheelcalib,wheelcalib);
wheelcalib operator-(wheelcalib,wheelcalib);
std::ostream& operator<<(std::ostream&,wheelcalib);

void configfile();
void adddefaultrpms();
void writeconfig(wheelcalib);

wheelcalib readconfig();
wheelcalib rpmsdefault();
#if 0
wheelcalib nullvalues();
wheelcalib zerovalues();
#endif

#endif
