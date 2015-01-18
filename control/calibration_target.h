#ifndef CALIBRATION_TARGET_H
#define CALIBRATION_TARGET_H

#include "fire_control.h"

struct Calibration_target{
	Fire_control::Target target;
	bool top;
	bool direct_mode;
	
	Calibration_target();
	Calibration_target(Fire_control::Target,bool top);
	
	static std::vector<Calibration_target> all();
};
std::ostream& operator<<(std::ostream&,Calibration_target);

#endif