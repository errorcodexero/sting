#include "util.h"
#include<iostream>
#include<cassert>
#include<math.h>

using namespace std;

ostream& operator<<(ostream& o,Joystick_section j){
	switch(j){
		#define X(name) case JOY_##name: return o<<""#name;
		X(LEFT)
		X(RIGHT)
		X(UP)
		X(DOWN)
		X(CENTER)
		#undef X
		default: assert(0); break;
	}
	return o;
}

Joystick_section joystick_section(double x,double y){
	static const double LIM=.25;
	if(fabs(x)<LIM && fabs(y)<LIM){
		return JOY_CENTER;
	}
	if(x<y){
		if(x>-y){
			return JOY_DOWN;
		}
		return JOY_LEFT;
	}
	if(x>-y) return JOY_RIGHT;
	return JOY_UP;
}

Joystick_section divide_vertical(double y){ return joystick_section(0,y); }

unsigned interpret_10_turn_pot(Volt v){
	/*measured values, measured on the Fall 2013 mecanum base:
	0.005
	0.56
	1.116
	1.66
	2.22
	2.77
	3.33
	3.89
	4.44
	5.01
	limits are halfway between each value.
	*/
	Volt limits[]={
		0.2825,
		0.838,
		1.388,
		1.94,
		2.495,
		3.05,
		3.61,
		4.165,
		4.725,
		5.01
	};
	for(unsigned i=0;i<10;i++){
		if(v<limits[i]) return i;
	}
	return 9;
}

#ifdef INPUT_UTIL_TEST
void joystick_section_test(){
	assert(joystick_section(0,0)==JOY_CENTER);
	assert(joystick_section(-1,0)==JOY_LEFT);
	assert(joystick_section(1,0)==JOY_RIGHT);
	assert(joystick_section(0,-1)==JOY_UP);
	assert(joystick_section(0,1)==JOY_DOWN);
}

int main(){
	joystick_section_test();
}
#endif
