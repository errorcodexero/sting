#include "toplevel_mode.h"
#include<sstream>
#include "../util/util.h"

using namespace std;

namespace Toplevel{
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

	Subgoals subgoals(Mode m,Drive_goal d,Shooter_wheels::Calibration calib){
		Subgoals r;
		r.drive=d;
		switch(m){
			case DRIVE_WO_BALL:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_X;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);
				break;
			case DRIVE_W_BALL:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_CLOSE;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);
				break;
			case COLLECT_SPIN_UP:
				r.collector=Collector_mode::ON;
				r.collector_tilt=Collector_tilt::GOAL_DOWN;
				r.injector_arms=Injector_arms::GOAL_OPEN;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::AUTO_SHOT_NONBLOCK);
				r.pump=Pump::GOAL_OFF;//to make the spin up faster.
				break;
			case COLLECT:
				r.collector=Collector_mode::ON;
				r.collector_tilt=Collector_tilt::GOAL_DOWN;
				r.injector_arms=Injector_arms::GOAL_OPEN;
				//r.shooter_wheels=convert_goal(calib,Shooter_wheels::HIGH_GOAL_NONBLOCK);
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);
				break;
			case SHOOT_HIGH_PREP:
			case SHOOT_HIGH:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_CLOSE;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::HIGH_GOAL);
				if(m==SHOOT_HIGH){
					r.injector=Injector::START;
				}
				break;
			case SHOOT_HIGH_PREP_NO_PUMP:
			case SHOOT_HIGH_NO_PUMP:
			case AUTO_SHOT_PREP:
			case AUTO_SHOT:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_CLOSE;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::AUTO_SHOT);
				if(m==AUTO_SHOT){
					r.injector=Injector::START;
				}
				break;
			case TRUSS_TOSS_PREP:
			case TRUSS_TOSS:
				r.collector_tilt=Collector_tilt::GOAL_UP;
				r.injector_arms=Injector_arms::GOAL_CLOSE;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::TRUSS);
				if(m==TRUSS_TOSS || m==SHOOT_HIGH_NO_PUMP) r.injector=Injector::START;
				if(m==SHOOT_HIGH_PREP_NO_PUMP || m==SHOOT_HIGH_NO_PUMP){
									r.pump=Pump::GOAL_OFF;
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
                r.collector_tilt=Collector_tilt::GOAL_UP;//was down, but with the current ejector geometry works better this way.
                r.injector_arms=Injector_arms::GOAL_OPEN;
                if(m==EJECT) r.ejector=Ejector::START;
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::X);
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
				r.collector_tilt=Collector_tilt::GOAL_DOWN;
				r.injector_arms=Injector_arms::GOAL_CLOSE;//not sure that this matters
				r.shooter_wheels=convert_goal(calib,Shooter_wheels::STOP);//could also have a reverse mode here
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

	Toplevel::Mode to_mode(Control_status::Control_status status){
		switch(status){
			case Control_status::AUTO_SPIN_UP: return Toplevel::SHOOT_HIGH_PREP;
			case Control_status::AUTO_FIRE: return Toplevel::SHOOT_HIGH;
			case Control_status::AUTO_TO_COLLECT: return Toplevel::COLLECT;
			case Control_status::AUTO_COLLECT: return Toplevel::COLLECT;
			case Control_status::AUTO_SPIN_UP2: return Toplevel::SHOOT_HIGH_PREP;
			case Control_status::AUTO_FIRE2: return Toplevel::SHOOT_HIGH;		
			case Control_status::A2_SPIN_UP: return Toplevel::AUTO_SHOT_PREP;
			case Control_status::A2_FIRE: return Toplevel::AUTO_SHOT;
			case Control_status::A2_TO_COLLECT: return Toplevel::COLLECT_SPIN_UP;
			case Control_status::A2_COLLECT: return Toplevel::COLLECT_SPIN_UP; //Eric says worry about this ANOTHER day 
			case Control_status::A2_SPIN_UP2: return Toplevel::AUTO_SHOT_PREP;
			case Control_status::A2_FIRE2: return Toplevel::AUTO_SHOT;
			case Control_status::A2_MOVE: return Toplevel::DRIVE_WO_BALL;
			case Control_status::DRIVE_W_BALL: return Toplevel::DRIVE_W_BALL;
			case Control_status::DRIVE_WO_BALL: return Toplevel::DRIVE_WO_BALL;
			case Control_status::COLLECT: return Toplevel::COLLECT;
			case Control_status::SHOOT_HIGH_PREP: return Toplevel::SHOOT_HIGH_PREP;
			case Control_status::SHOOT_HIGH: return Toplevel::SHOOT_HIGH;
			case Control_status::SHOOT_HIGH_WHEN_READY: return Toplevel::SHOOT_HIGH_PREP;
			case Control_status::TRUSS_TOSS_PREP: return Toplevel::TRUSS_TOSS_PREP;
			case Control_status::TRUSS_TOSS: return Toplevel::TRUSS_TOSS;
			case Control_status::TRUSS_TOSS_WHEN_READY: return Toplevel::TRUSS_TOSS_PREP;
			case Control_status::AUTO_SHOT_PREP: return Toplevel::AUTO_SHOT_PREP;
			case Control_status::AUTO_SHOT: return Toplevel::AUTO_SHOT;
			case Control_status::AUTO_SHOT_WHEN_READY: return Toplevel::AUTO_SHOT_PREP;
			case Control_status::EJECT_PREP: return Toplevel::EJECT_PREP;
			case Control_status::EJECT: return Toplevel::EJECT;
			case Control_status::EJECT_WHEN_READY: return Toplevel::EJECT_PREP;
			case Control_status::CATCH: return Toplevel::CATCH;
			default:
				assert(0);
		}
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


namespace Toplevel{
	static const vector<Mode> MODES{
		DRIVE_WO_BALL,DRIVE_W_BALL,
		COLLECT,
		SHOOT_HIGH_PREP,SHOOT_HIGH,
		TRUSS_TOSS_PREP,TRUSS_TOSS,
		//PASS_PREP,PASS,
		EJECT_PREP,EJECT,
		CATCH, //SHOOT_LOW
		COLLECT_SPIN_UP,
		SHOOT_HIGH_PREP_NO_PUMP,
		SHOOT_HIGH_NO_PUMP,
		AUTO_SHOT,
		AUTO_SHOT_PREP
	};
}

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
					for(auto mode:Toplevel::MODES){
						Toplevel::Subgoals g=subgoals(mode,Drive_goal(),Shooter_wheels::Calibration());
						tag(ss,"tr",
							tag("td",as_string(mode))+
							tag("td",g.collector)+
							tag("td",g.collector_tilt)+
							tag("td",g.injector)+
							tag("td",g.injector_arms)+
							tag("td",g.ejector)+
							tag("td",g.shooter_wheels.high_level)
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
	using namespace Toplevel;
	Status status;
	for(auto mode:MODES){
		cout<<mode<<":\n";
		auto g=subgoals(mode,Drive_goal(),Shooter_wheels::Calibration());
		cout<<"\t"<<g<<"\n";
		cout<<"\t"<<control(status,g)<<"\n";
		cout<<"\t"<<ready(status,g)<<"\n";
	}
}
#endif 
