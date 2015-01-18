#ifndef CONTROL_STATUS_H
#define CONTROL_STATUS_H

#include <iosfwd>
#include<vector>
#include "../util/maybe.h"

namespace Control_status{
	enum Control_status{
		AUTO_SPIN_UP,
		AUTO_FIRE,
		AUTO_TO_COLLECT,
		AUTO_COLLECT,
		AUTO_SPIN_UP2,
		AUTO_FIRE2,

		A2_SPIN_UP,
		A2_FIRE,
		A2_TO_COLLECT,
		A2_COLLECT,
		A2_SPIN_UP2,
		A2_FIRE2,
		A2_MOVE,

		//many of these are the same as things in "Mode"
		DRIVE_W_BALL,DRIVE_WO_BALL,
		COLLECT,
		SHOOT_HIGH_PREP,SHOOT_HIGH,SHOOT_HIGH_WHEN_READY,
		TRUSS_TOSS_PREP,TRUSS_TOSS,TRUSS_TOSS_WHEN_READY,
		EJECT_PREP,EJECT,EJECT_WHEN_READY,
		AUTO_SHOT_PREP,AUTO_SHOT,AUTO_SHOT_WHEN_READY,
		CATCH, //SHOOT_LOW
	};
	std::ostream& operator<<(std::ostream&,Control_status);
	std::vector<Control_status> all();
	Maybe<Control_status> parse(std::string const&);
	bool autonomous(Control_status);
	bool teleop(Control_status);
}
#endif
