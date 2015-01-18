#ifndef SHOOTER_WHEELS_H
#define SHOOTER_WHEELS_H

#include"shooter_status.h"
#include"wheelrpms.h"
#include "../util/jag_interface.h"
#include "calibration_target.h"
#include "../util/posedge_trigger.h"
#include "../input/panel2014.h"

namespace Shooter_wheels{
	enum High_level_goal{
		HIGH_GOAL,TRUSS,STOP,X,AUTO_SHOT,
		HIGH_GOAL_NONBLOCK, //go to the high goal speed but always read 'ready'
		AUTO_SHOT_NONBLOCK
	};
	std::ostream& operator<<(std::ostream&,High_level_goal);
	
	struct Output{
		static const int FEEDBACK=0,OPEN_LOOP=1;
		Jaguar_output top[2],bottom[2];
	};
	std::ostream& operator<<(std::ostream& o,Output);
	
	struct Goal{
		High_level_goal high_level;
		Status speed;
		PID_coefficients pid;

		Goal();
		Goal(High_level_goal,Status,PID_coefficients);
	};
	std::ostream& operator<<(std::ostream&,Goal);
	Output control(Goal);

	typedef std::pair<wheelcalib,PID_coefficients> Calibration;

	struct Calibration_manager{
		wheelcalib calib;
		Posedge_trigger learn;
		
		PID_coefficients pid;
		Posedge_trigger pidadjust;

		Calibration_manager();
		
		//adjust wheel in 0-3.3
		Calibration update(bool learn,double adjust_wheel,Calibration_target,Panel::PIDselect,bool pid_adjust);
	};
	bool operator==(Calibration_manager,Calibration_manager);
	std::ostream& operator<<(std::ostream&,Calibration_manager);
	
	Goal convert_goal(Calibration,High_level_goal);
	Output control(Status,Goal);
	bool ready(Status,Goal);

	RPM free_speed();
}

#endif
