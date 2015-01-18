#include "cyborg_joystick.h"
#include<iostream>

using namespace std;

Cyborg_joystick::Cyborg_joystick():x(0),y(0),theta(0),throttle(0),hat_x(0),hat_y(0){
	for(unsigned i=0;i<Cyborg_joystick::BUTTONS;i++){
		button[i]=0;
	}
}

ostream& operator<<(ostream& o,Cyborg_joystick c){
	o<<"Cyborg_joystick(";
	o<<"x:"<<c.x;
	o<<" y:"<<c.y;
	o<<" theta:"<<c.theta;
	o<<" throttle:"<<c.throttle;
	o<<" hat_x:"<<c.hat_x;
	o<<" hat_y:"<<c.hat_y;
	o<<" btn:";
	for(unsigned i=0;i<Cyborg_joystick::BUTTONS;i++){
		o<<c.button[i];
	}
	return o<<")";
}

#ifdef CYBORG_JOYSTICK_TEST
int main(){
	Cyborg_joystick c;
	cout<<c<<"\n";
}
#endif
