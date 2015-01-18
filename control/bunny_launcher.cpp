#include "bunny_launcher.h"
#include<iostream>

using namespace std;

Bunny_launcher::Bunny_launcher():state_(COCKED),launch_start(-1){}

void Bunny_launcher::update(Time t,bool disabled,bool go,bool reset){
	//if we go into disabled mode, the timer for when the launch will finish continually resets.
	if(disabled) launch_start=t;
	
	switch(state_){
		case COCKED:
			if(go){
				state_=LAUNCHING;
				launch_start=t;
			}
			break;
		case LAUNCHING:
			{
				Time elapsed=t-launch_start;
				//we may need to experimentally determine how long to wait.
				if(elapsed>1){
					state_=LAUNCHED;
				}
			}
			break;
		default:
			//this will be used during practice, but not during a round.
			if(reset) state_=COCKED;
	}
}

bool Bunny_launcher::output()const{
	return state_==LAUNCHING;
}

Bunny_launcher::State Bunny_launcher::state()const{
	return state_;
}

ostream& operator<<(ostream& o,Bunny_launcher::State a){
	switch(a){
		#define X(a) case Bunny_launcher::a: return o<<""#a;
		X(COCKED)
		X(LAUNCHING)
		X(LAUNCHED)
		#undef X
		default:
			return o<<"error";
	}
}

ostream& operator<<(ostream& o,Bunny_launcher a){
	return o<<"Bunny_launcher("<<a.state()<<" "<<a.launch_start<<")";
}

#ifdef BUNNY_LAUNCHER_TEST
int main(){
	Bunny_launcher b;
	cout<<b<<"\n";
	b.update(.3,0,0,0);
	cout<<b<<"\n";
	b.update(.5,0,1,0);
	cout<<b<<"\n";
	b.update(2,0,0,0);
	cout<<b<<"\n";
}
#endif
