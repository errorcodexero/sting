#ifndef BIST_H
#define BIST_H

#include "toplevel.h"

struct BIST{
	unsigned state;
	Countup_timer timer;
	bool auto_mode;

	BIST();

	unsigned next()const;
	unsigned prev()const;
	void update(Time time,bool step_fwd,bool stop,bool step_back);

	Toplevel::Output out()const;

	//could add a 'done' function.
};

std::ostream& operator<<(std::ostream&,BIST);

struct BIST_interface{
	BIST b;
	Posedge_trigger step_fwd,stop,step_back;

	void update(Time time,bool step_fwd_button,bool stop_button,bool step_back_button);
	Toplevel::Output out()const;
};

std::ostream& operator<<(std::ostream&,BIST_interface);

#endif