#ifndef MAIN_H
#define MAIN_H

#include "force_interface.h"
#include "../util/posedge_toggle.h"
#include "../util/perf_tracker.h"
#include "../util/countdown_timer.h"
#include "../util/countup_timer.h"
#include "toplevel.h"
#include "../input/panel2015.h"

struct Main{
	#define MODES X(TELEOP) X(AUTO_MOVE) X(AUTO_GRAB) X(AUTO_BACK) X(AUTO_RELEASE)
	enum class Mode{
		#define X(NAME) NAME,
		MODES
		#undef X
	};
	Mode mode;
	
	Force_interface force;
	Perf_tracker perf;
	Toplevel toplevel;

	Countup_timer since_switch,since_auto_start;

	Posedge_trigger autonomous_start;

	//Move into PANEL.H when OI ready

	Posedge_toggle piston;
	
	enum class Sticky_can_goal{STOP,BOTTOM,TOP,UP_LEVEL,DOWN_LEVEL,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5,LEVEL6_NUDGE/*,KILL*/};
	Sticky_can_goal sticky_can_goal;
	
	enum class Sticky_tote_goal{STOP,BOTTOM,TOP,ENGAGE_KICKER,UP_LEVEL,DOWN_LEVEL,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5/*,KILL*/};
	Sticky_tote_goal sticky_tote_goal;
	Sticky_tote_goal pre_sticky_tote_goal;
	bool can_priority;
	
	struct Nudge{
		Posedge_trigger trigger;
		Countdown_timer timer;
	};
	Nudge nudges[6];//Left, Right, Forward, Backward, Clockwise, Counter-clockwise
	Nudge back_turns[2];//Backwards and left, Backwards and right
	
	Toplevel::Goal teleop(Robot_inputs const&,Joystick_data const&,Joystick_data const&,Panel const&,Toplevel::Status_detail&);
	Main();
	Robot_outputs operator()(Robot_inputs,std::ostream& = std::cerr);
};

std::ostream& operator<<(std::ostream&,Main::Mode);

bool operator==(Main,Main);
bool operator!=(Main,Main);
std::ostream& operator<<(std::ostream&,Main);

#endif
