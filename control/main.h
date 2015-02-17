#ifndef MAIN_H
#define MAIN_H

#include "force_interface.h"
#include "../util/posedge_toggle.h"
#include "../util/perf_tracker.h"
#include "../util/bound_tracker.h"
#include "control_status.h"
#include "../util/countdown_timer.h"
#include "../util/countup_timer.h"
#include "toplevel.h"
#include "drivebase.h"
#include "lift.h"
#include "../input/util.h"

struct Main{
	enum class Mode{TELEOP,AUTO_MOVE};
	Mode mode;
	
	Force_interface force;
	Perf_tracker perf;
	Toplevel::Estimator est;

	Control_status::Control_status control_status;
	Countup_timer since_switch,since_auto_start;

	Posedge_toggle ball_collecter;
	Posedge_trigger print_button;

	Posedge_toggle field_relative;
	Posedge_trigger autonomous_start;

	Shooter_wheels::Calibration_manager wheel_calibration;
	Drivebase drivebase;
	
	Lift lift_can;
	Lift lift_tote;

	enum class Sticky_can_goal{STOP,HEIGHT};
	Sticky_can_goal sticky_can_goal;
	
	enum class Sticky_tote_goal{STOP,MIN,MAX,HEIGHT};
	Sticky_tote_goal sticky_tote_goal;
	
	struct Nudge{
		Posedge_trigger trigger;
		Countdown_timer timer;
		bool start;
	};
	Nudge nudges[6];//Left, Right, Forward, Backward, Clockwise, Counter-clockwise
	
	Main();
	Robot_outputs operator()(Robot_inputs,std::ostream& = std::cerr);
};

bool operator==(Main,Main);
bool operator!=(Main,Main);
std::ostream& operator<<(std::ostream&,Main);

#endif
