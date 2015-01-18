#ifndef PUMP_H
#define PUMP_H

//This is meant to control the pump for the pneumatic system.
//It may seem a little bit overkill, but this is here basically because there are some fancier things we might want to do later like know when we have at least 60psi.  

#include<iosfwd>
#include "../util/maybe.h"

namespace Pump{
	enum Goal{GOAL_AUTO,GOAL_OFF};
	std::ostream& operator<<(std::ostream&,Goal);

	enum Output{OUTPUT_ON,OUTPUT_OFF};
	std::ostream& operator<<(std::ostream&,Output);

	enum Status{FULL,NOT_FULL};
	std::ostream& operator<<(std::ostream&,Status);
	Maybe<Status> parse_status(std::string const&);

	Output control(Status,Goal);
}

#endif
