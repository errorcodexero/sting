#include "toplevel_mode.h"
#include<sstream>
#include "../util/util.h"

using namespace std;

namespace Toplevel_mode{
	ostream& operator<<(ostream& o,Mode m){
		#define X(name) if(m==name) return o<<""#name;
		X(DRIVE_WO_BALL)
		X(DRIVE_W_BALL)
		X(COLLECT)
		X(COLLECT_SPIN_UP)
		X(SHOOT_HIGH_PREP)
		X(SHOOT_HIGH)
		X(TRUSS_TOSS_PREP)
		X(TRUSS_TOSS)
		//X(PASS_PREP)
		//X(PASS)
		X(EJECT_PREP)
		X(EJECT)
		X(AUTO_SHOT_PREP)
		X(AUTO_SHOT)
		X(CATCH)
		//X(SHOOT_LOW)
		X(SHOOT_HIGH_PREP_NO_PUMP)
		X(SHOOT_HIGH_NO_PUMP)
		#undef X
		assert(0);
	}
	Toplevel::Goal subgoals(Mode m){
		Toplevel::Goal r;
		switch(m){
			case DRIVE_WO_BALL:
				break;
			case DRIVE_W_BALL:
				break;
			case COLLECT_SPIN_UP:
				r.pump=Pump::Goal::OFF;//to make the spin up faster.
				break;
			case COLLECT:
				//r.shooter_wheels=convert_goal(calib,Shooter_wheels::HIGH_GOAL_NONBLOCK);
				break;
			case SHOOT_HIGH_PREP:
			case SHOOT_HIGH:
				break;
			case SHOOT_HIGH_PREP_NO_PUMP:
			case SHOOT_HIGH_NO_PUMP:
			case AUTO_SHOT_PREP:
			case AUTO_SHOT:
				break;
			case TRUSS_TOSS_PREP:
			case TRUSS_TOSS:
				if(m==SHOOT_HIGH_PREP_NO_PUMP || m==SHOOT_HIGH_NO_PUMP){
									r.pump=Pump::Goal::OFF;
				}
				break;
		/*	case PASS_PREP:
			case PASS:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_CLOSE;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::PASS);
				if(m==PASS) r.injector=Injector::START;
				break;*/
			case EJECT_PREP:
			case EJECT:
                //Copied from a previous commit of the code, basically what it was before modification
				break;
				/*
				r.collector=REVERSE;
				r.collector_tilt=Collector_tilt::GOAL_DOWN;
				r.injector_arms=Injector_arms::GOAL_OPEN;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);
				if(m==EJECT) r.ejector=Ejector::START;
				break;
				*/
			case CATCH:
				break;
			/*
			case SHOOT_LOW:
				r.collector_tilt=Collector_tilt::GOAL_UP;//was down, but with the current ejector geometry works better this way.
				r.injector_arms=Injector_arms::GOAL_OPEN;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);//could also have a reverse mode here
				if(m==SHOOT_LOW) r.ejector=Ejector::START;
				break;
			*/
			default:cout<<m<<endl;
			assert(0);
		}
		return r;
	}
}

#ifdef TOPLEVEL_MODE_TEST
#include<vector>
#include<fstream>

template<typename T>
void tag(ostream& o,string name,T contents){
	assert(name.size());
	o<<"<"<<name<<">";
	o<<contents;
	o<<"</"<<split(name)[0]<<">";
}

template<typename T>
string tag(string name,T contents){
	stringstream ss;
	tag(ss,name,contents);
	return ss.str();
}


static const vector<Toplevel_mode::Mode> MODES{
	Toplevel_mode::DRIVE_WO_BALL,Toplevel_mode::DRIVE_W_BALL,
	Toplevel_mode::COLLECT,
	Toplevel_mode::SHOOT_HIGH_PREP,Toplevel_mode::SHOOT_HIGH,
	Toplevel_mode::TRUSS_TOSS_PREP,Toplevel_mode::TRUSS_TOSS,
	//PASS_PREP,PASS,
	Toplevel_mode::EJECT_PREP,Toplevel_mode::EJECT,
	Toplevel_mode::CATCH, //SHOOT_LOW
	Toplevel_mode::COLLECT_SPIN_UP,
	Toplevel_mode::SHOOT_HIGH_PREP_NO_PUMP,
	Toplevel_mode::SHOOT_HIGH_NO_PUMP,
	Toplevel_mode::AUTO_SHOT,
	Toplevel_mode::AUTO_SHOT_PREP
};

void toplevel_modes(){
	ofstream f("out.html");
	tag(
		f,
		"html",
		tag(
			"body",
			tag("table border",
				[](){
					stringstream ss;
					ss<<"<tr>";
					for(auto s:{"Mode","Collector","Collector tilt","Injector","Injector_arms","Ejector","Target"}){
						tag(ss,"th",s);
					}
					ss<<"</tr>";	
					for(auto mode:MODES){
						//Toplevel::Subgoals g=subgoals(mode);
						tag(ss,"tr",
							tag("td",as_string(mode))
						);
					}
					return ss.str();
				}()
			)
		)
	);
}

int main(){
	toplevel_modes();
	Toplevel::Status status;
	for(auto mode:MODES){
		cout<<mode<<":\n";
		auto g=subgoals(mode);
		cout<<"\t"<<g<<"\n";
		//cout<<"\t"<<control(status,g)<<"\n";
		cout<<"\t"<<ready(status,g)<<"\n";
	}
}
#endif 
