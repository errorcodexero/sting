#ifndef MAIN_H
#define MAIN_H

#include "force_interface.h"
#include "gyro_tracker.h"
#include "../util/posedge_toggle.h"
#include "../util/perf_tracker.h"
#include "../util/bound_tracker.h"
#include "control_status.h"
#include "../util/countup_timer.h"
#include "toplevel.h"

struct Main{
	Force_interface force;
	Perf_tracker perf;
	Gyro_tracker gyro;
	Toplevel::Estimator est;

	Control_status::Control_status control_status;
	Countup_timer since_switch,since_auto_start;

	Posedge_toggle ball_collecter;
	Posedge_trigger print_button;

	Posedge_toggle field_relative;
	Posedge_trigger autonomous_start;

	// Keeps track of light status
	Posedge_toggle light;
		
	Shooter_wheels::Calibration_manager wheel_calibration;
	
	Main();
	Robot_outputs operator()(Robot_inputs,std::ostream& = std::cerr);
};

bool operator==(Main,Main);
bool operator!=(Main,Main);
std::ostream& operator<<(std::ostream&,Main);

#endif
