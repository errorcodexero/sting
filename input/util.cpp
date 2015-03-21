#include "util.h"
#include<iostream>
#include<cassert>
#include<math.h>

using namespace std;

ostream& operator<<(ostream& o,Joystick_section j){
	switch(j){
		#define X(name) case Joystick_section::name: return o<<""#name;
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
		return Joystick_section::CENTER;
	}
	if(x<y){
		if(x>-y){
			return Joystick_section::DOWN;
		}
		return Joystick_section::LEFT;
	}
	if(x>-y) return Joystick_section::RIGHT;
	return Joystick_section::UP;
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
	array<Volt,10>limits={
		-.8,
		-.63,
		-.45,
		-.2,
		0,
		.2,
		.45,
		.65,
		.85,
		1.5
	};
	for(unsigned i=0;i<10;i++){
		if(v<limits[i]) return i;
	}
	return 9;
}

#ifdef INPUT_UTIL_TEST
void joystick_section_test(){
	assert(joystick_section(0,0)==Joystick_section::CENTER);
	assert(joystick_section(-1,0)==Joystick_section::LEFT);
	assert(joystick_section(1,0)==Joystick_section::RIGHT);
	assert(joystick_section(0,-1)==Joystick_section::UP);
	assert(joystick_section(0,1)==Joystick_section::DOWN);
}

int main(){
	joystick_section_test();
}
#endif
