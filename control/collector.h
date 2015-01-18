#ifndef COLLECTOR_H
#define COLLECTOR_H

#include"../util/posedge_trigger.h"
#include"../util/negedge_trigger.h"

enum class Collector_mode{ON, OFF, REVERSE};

std::ostream& operator << (std::ostream& o, Collector_mode c);

struct Collector{
	Posedge_trigger on,rev;
	Negedge_trigger rev_release;
	Collector_mode mode;
	Collector();
	
	Collector_mode run (bool,bool);
};

#endif
