#ifndef CYBORG_JOYSTICK_H
#define CYBORG_JOYSTICK_H

#include<iosfwd>

class Joystick_data;

struct Cyborg_joystick{
	double x,y,theta,throttle;
	double hat_x,hat_y;

	static const unsigned BUTTONS=6;
	bool button[BUTTONS];

	Cyborg_joystick();
};

std::ostream& operator<<(std::ostream&,Cyborg_joystick);

Cyborg_joystick as_cyborg_joystick(Joystick_data);

#endif
