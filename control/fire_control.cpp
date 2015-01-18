#include "fire_control.h"
#include<iostream>
#include<cassert>
#include "../util/util.h"

using namespace std;
namespace Fire_control{
	ostream& operator<<(ostream& o,Target t){
		#define X(name) if(t==name) return o<<""#name;
		X(NO_TARGET)
		X(HIGH)
		X(TRUSS)
		X(EJECT)
		X(AUTO_SHOT)
		#undef X
		assert(0);
	}

	Target target(Control_status::Control_status c){
		switch(c){
			//or maybe the auto stuff should all return 'other'
			case Control_status::AUTO_SPIN_UP:
			case Control_status::AUTO_FIRE:
			case Control_status::A2_SPIN_UP:
			case Control_status::A2_FIRE:
				return HIGH;
			case Control_status::AUTO_TO_COLLECT:
			case Control_status::AUTO_COLLECT:
			case Control_status::A2_TO_COLLECT:
			case Control_status::A2_COLLECT:
				return NO_TARGET;
			case Control_status::AUTO_SPIN_UP2:
			case Control_status::AUTO_FIRE2:
			case Control_status::A2_SPIN_UP2:
			case Control_status::A2_FIRE2:
				return HIGH;
			case Control_status::DRIVE_W_BALL:
			case Control_status::DRIVE_WO_BALL:
			case Control_status::COLLECT:
			case Control_status::A2_MOVE:
				return NO_TARGET;
			case Control_status::SHOOT_HIGH_PREP:
			case Control_status::SHOOT_HIGH:
			case Control_status::SHOOT_HIGH_WHEN_READY:
				return HIGH;
			case Control_status::TRUSS_TOSS_PREP:
			case Control_status::TRUSS_TOSS:
			case Control_status::TRUSS_TOSS_WHEN_READY:
				return TRUSS;
			case Control_status::EJECT_PREP:
			case Control_status::EJECT:
			case Control_status::EJECT_WHEN_READY:
				return EJECT;
			case Control_status::AUTO_SHOT_PREP:
			case Control_status::AUTO_SHOT:
			case Control_status::AUTO_SHOT_WHEN_READY:
				return AUTO_SHOT;
			case Control_status::CATCH: return NO_TARGET;
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Goal g){
		#define X(name) if(g==name) return o<<""#name;
		X(PREP)
		X(FIRE)
		X(FIRE_WHEN_READY)
		X(DRIVE)
		X(OTHER)
		#undef X
		assert(0);
	}

	Goal goal(Control_status::Control_status c){
		switch(c){
			//or maybe the auto stuff should all return 'other'
			case Control_status::AUTO_SPIN_UP: return FIRE_WHEN_READY;
			case Control_status::AUTO_FIRE: return FIRE;
			case Control_status::AUTO_TO_COLLECT: return DRIVE;
			case Control_status::AUTO_COLLECT: return DRIVE;
			case Control_status::AUTO_SPIN_UP2: return FIRE_WHEN_READY;
			case Control_status::AUTO_FIRE2: return FIRE;
			case Control_status::A2_SPIN_UP:return FIRE_WHEN_READY;
			case Control_status::A2_FIRE: return FIRE;
			case Control_status::A2_TO_COLLECT: return DRIVE;
			case Control_status::A2_COLLECT: return DRIVE;
			case Control_status::A2_SPIN_UP2:return FIRE_WHEN_READY;
			case Control_status::A2_FIRE2: return FIRE;
			case Control_status::DRIVE_W_BALL:
			case Control_status::DRIVE_WO_BALL:
			case Control_status::COLLECT:
			case Control_status::A2_MOVE:
				return DRIVE;
			case Control_status::SHOOT_HIGH_PREP: return PREP;
			case Control_status::SHOOT_HIGH: return FIRE;
			case Control_status::SHOOT_HIGH_WHEN_READY: return FIRE_WHEN_READY;
			case Control_status::TRUSS_TOSS_PREP: return PREP;
			case Control_status::TRUSS_TOSS: return FIRE;
			case Control_status::TRUSS_TOSS_WHEN_READY: return FIRE_WHEN_READY;
			case Control_status::EJECT_PREP: return PREP;
			case Control_status::EJECT: return FIRE;
			case Control_status::EJECT_WHEN_READY: return FIRE_WHEN_READY;
			case Control_status::AUTO_SHOT_PREP:return PREP;
			case Control_status::AUTO_SHOT: return FIRE;
			case Control_status::AUTO_SHOT_WHEN_READY: return FIRE_WHEN_READY;
			case Control_status::CATCH: return OTHER;
			default: assert(0);
		}
	}

	Control_status::Control_status generate_status(Target target,Goal goal){
		if(target == HIGH && goal == PREP){return Control_status::SHOOT_HIGH_PREP;}
		if(target == TRUSS && goal == PREP) {return Control_status::TRUSS_TOSS_PREP;}
		if(target == EJECT && goal == PREP) {return Control_status::EJECT_PREP;}
		if(target == AUTO_SHOT && goal == PREP) {return Control_status::AUTO_SHOT_PREP;}
		if(target == HIGH && goal == FIRE) {return Control_status::SHOOT_HIGH;}
		if(target == TRUSS && goal == FIRE) {return Control_status::TRUSS_TOSS;}
		if(target == EJECT && goal == FIRE) {return Control_status::EJECT;}
		if(target == AUTO_SHOT && goal == FIRE) {return Control_status::AUTO_SHOT;}
		if(target == HIGH && goal == FIRE_WHEN_READY) {return Control_status::SHOOT_HIGH_WHEN_READY;}
		if(target == TRUSS && goal == FIRE_WHEN_READY) {return Control_status::TRUSS_TOSS_WHEN_READY;}
		if(target == EJECT && goal == FIRE_WHEN_READY) {return Control_status::EJECT_WHEN_READY;}
		if(target == AUTO_SHOT && goal == FIRE_WHEN_READY) {return Control_status::AUTO_SHOT_WHEN_READY;}
		if(target == HIGH && goal == DRIVE) {throw "error1";}
		if(target == TRUSS && goal == DRIVE) {throw "error1";}
		if(target == EJECT && goal == DRIVE) {throw "error1";}
		if(target == AUTO_SHOT && goal == DRIVE) {throw "error1";}
		if(target == HIGH && goal == OTHER) {throw "error1";}
		if(target == TRUSS && goal == OTHER) {throw "error1";}
		if(target == EJECT && goal == OTHER) {throw "error1";}
		if(target == AUTO_SHOT && goal == OTHER) {throw "error1";}
		throw "Unknown";
	}
	
	/*Goal next_goal(Goal g,bool ready,bool fired,bool prep_button,bool fire_button,bool fire_when_ready_button){
		if(fire_button) return FIRE;
		switch(g){
			case PREP:
				return fire_when_ready_button?FIRE_WHEN_READY:PREP;
			case FIRE:
				return fired?DRIVE:FIRE;
			case FIRE_WHEN_READY:
				if(ready) return FIRE;
				if(fire_when_ready_button) return FIRE_WHEN_READY;
				return PREP;
			case DRIVE://why are we in here?
			default: assert(0);
		}
	}*/

	/*Control_status::Control_status next(Control_status::Control_status cs,bool ready,bool fired,bool prep_button,bool fire_button,bool fire_when_ready_button){
		Target t=target(cs);
		Goal g=goal(cs);
		cout<<t<<" "<<g<<"\n";
		assert(0);
	}*/

	vector<Fire_control::Goal> goals(){
		vector<Fire_control::Goal> r;
		r|=PREP;
		r|=FIRE;
		r|=FIRE_WHEN_READY;
		r|=DRIVE;
		r|=OTHER;
		return r;
	}

	vector<Target> targets(){
		vector<Target> r;
		r|=NO_TARGET;
		r|=HIGH;	
		r|=TRUSS;
		r|=EJECT;
		r|=AUTO_SHOT;
		return r;
	}
}

#ifdef FIRE_CONTROL_TEST
#include<vector>

int main(){
	using namespace Fire_control;

	for(auto a:Control_status::all()){
		cout<<a<<" "<<goal(a)<<" "<<target(a)<<"\n";
	}

	cout<<goals()<<"\n";
	cout<<targets()<<"\n";
	for(auto goal:goals()){
		for(auto target:targets()){
			cout<<goal<<" "<<target<<"\n";
			cout.flush();
			try{
				cout<<generate_status(target, goal)<<"\n";
			}catch(...){
				cout<<"Not an option\n";
			}
		}
	}
}
#endif
