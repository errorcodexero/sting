#ifndef PANEL2014_H
#define PANEL2014_H

#include "../control/fire_control.h"
#include "../util/maybe.h"
#include "../control/collector.h"
#include "../control/collector_tilt.h"
#include "../control/injector.h"
#include "../control/injector_arms.h"
#include "../control/ejector.h"
#include "../control/calibration_target.h"

//these will probably be the things that light up.
struct Mode_buttons{
	bool drive_wo_ball,drive_w_ball,collect,shoot_high,truss_toss,eject,catch_mode,auto_shot;

	Mode_buttons();
};
std::ostream& operator<<(std::ostream&,Mode_buttons);

struct Panel{
	enum Auto_mode{
		DO_NOTHING,ONE_BALL,TWO_BALL,MOVE
	};
	Auto_mode auto_mode;
	enum PIDselect{
		P,I,D,PID_NONE
	};
	PIDselect pidselect;
	//presets
	Mode_buttons mode_buttons;
	bool fire,pass_now,pidadjust;

	//shooting override section
	Calibration_target target;
	double speed;	
	bool learn;

	//standard override section
	
	Maybe<Collector_mode> collector;
	Maybe<Collector_tilt::Output> collector_tilt;
	Maybe<Injector::Output> injector;
	//Maybe<Injector_arms::Output> injector_arms;
	Maybe<Ejector::Output> ejector;
	
	bool force_wheels_off;
	
	Panel();
};
std::ostream& operator<<(std::ostream&,Panel::Auto_mode);
std::ostream& operator<<(std::ostream&,Panel::PIDselect);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret(Driver_station_input);

#endif
