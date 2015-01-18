#ifndef FIRE_CONTROL_H
#define FIRE_CONTROL_H

#include<iosfwd>
#include<vector>
#include "control_status.h"

namespace Fire_control{
	enum Target{NO_TARGET,HIGH,TRUSS,EJECT,AUTO_SHOT};
	std::ostream& operator<<(std::ostream&,Target);
	std::vector<Target> targets();
	Target target(Control_status::Control_status);

	enum Goal{PREP,FIRE,FIRE_WHEN_READY,DRIVE,OTHER};
	std::ostream& operator<<(std::ostream&,Goal);
	std::vector<Goal> goals();
	Goal goal(Control_status::Control_status);

	Control_status::Control_status generate_status(Target,Goal);

	Goal next_goal(Goal g,bool ready,bool fired,bool prep_button,bool fire_button,bool fire_when_ready_button);
	
	Control_status::Control_status next(Control_status::Control_status cs,bool ready,bool fired,bool prep_button,bool fire_button,bool fire_when_ready_button);
}
#endif
