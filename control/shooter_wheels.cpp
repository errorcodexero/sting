#include "shooter_wheels.h"
#include<iostream>
#include<cassert>
#include<stdlib.h>
#include<math.h>
#include<unistd.h>
#include"wheelrpms.h"

using namespace std;

namespace Shooter_wheels{
	ostream& operator<<(ostream& o,High_level_goal g){
		switch(g){
			#define X1(name) case name: return o<<""#name;
			X1(HIGH_GOAL)
			X1(TRUSS)
			X1(STOP)
			X1(X)
			X1(AUTO_SHOT)
			X1(HIGH_GOAL_NONBLOCK)
			X1(AUTO_SHOT_NONBLOCK)
			#undef X1
			default:assert(0);
		}
	}

	ostream& operator<<(ostream& o,Output a){
		o<<"Shooter_wheel::Output(";
		for(unsigned i=0;i<2;i++){
			o<<"t"<<i<<":"<<a.top[i];
		}
		for(unsigned i=0;i<2;i++){
			o<<"b"<<i<<":"<<a.bottom[i];
		}
		return o<<")";
	}

	Goal::Goal():high_level(X){}

	Goal::Goal(High_level_goal a,Status b,PID_coefficients c):high_level(a),speed(b),pid(c){}

	ostream& operator<<(ostream& o,Goal g){
		o<<"Shooter_wheels::Goal(";
		o<<g.high_level<<" "<<g.speed<<" "<<g.pid;
		return o<<")";
	}	

	Calibration_manager::Calibration_manager(){
		calib = readconfig();
		//this could be done in a better way!
		if(calib.highgoal.top == 0){
			calib = rpmsdefault();
		}
	}

	RPM& find_rpm(Shooter_wheels::Status &status,bool top){
		return top?status.top:status.bottom;
	}
	
	RPM& find_rpm(wheelcalib &in,Calibration_target t){
		switch(t.target){
			case Fire_control::HIGH: return find_rpm(in.highgoal,t.top);
			case Fire_control::TRUSS: return find_rpm(in.overtruss,t.top);
			case Fire_control::AUTO_SHOT: return find_rpm(in.passing,t.top);
			case Fire_control::NO_TARGET:
			case Fire_control::EJECT:
				return in.lowgoal.bottom;
			default:assert(0);
		}
	}
	
	Calibration Calibration_manager::update(bool learn_button,double adjust_wheel,Calibration_target t,Panel::PIDselect /*pidselect*/,bool /*pidadjust*/){
		wheelcalib w=calib;
		{
			double adjustment=((adjust_wheel/3.3)-.5)*2*100;
			//cout<<"adjustment:"<<adjustment<<"\n";
			find_rpm(w,t)+=adjustment;
			if(learn(learn_button)){
				//write changes to the file
				calib=w;
				writeconfig(w);
				cerr<<"Learning now!\r\n";
				cerr<<w<<endl;
			}
			//cerr<<"direct_mode:"<<t.direct_mode<<"\r\n";
			if(t.direct_mode){
				double a2=adjust_wheel/3.3*4000;
				w.highgoal.bottom=a2;
				w.highgoal.top=a2-1250;
			}
		}
		PID_coefficients pid;//TODO: Make this adjust.
		return make_pair(w,pid);
	}

	bool operator==(Calibration_manager a,Calibration_manager b){
		#define X(name) if(a.name!=b.name) return 0;
		X(calib)
		X(learn)
		X(pid)
		X(pidadjust)
		#undef X
		return 1;
	}
	
	RPM target_speed_top(High_level_goal g,wheelcalib c){
		//this stuff eventually should come out of a config file.
		switch(g){
			case TRUSS:
				return 1825;//c.overtruss.top; //Previously 1200
			case HIGH_GOAL:
				return 2900;//c.highgoal.top; //Previously 1200
			case AUTO_SHOT:
				return c.passing.top; //Previously (FILL IN)
			case STOP:
			case X:
				return 0;
			default:assert(0);
		}
	}

	RPM target_speed_bottom(High_level_goal g, wheelcalib c){
		switch(g){
			case TRUSS:
				return c.overtruss.bottom; //Previously 1200
			case HIGH_GOAL:
				return c.highgoal.bottom; //Previously 3000
			case AUTO_SHOT:
				return c.passing.bottom; //Previously (FILL IN)
			case STOP:
			case X:
				return 0;
			default:assert(0);
		}
	}
	
	RPM free_speed(){
		return 5984;  //Calculated top speed on 100% output
	}

	Jaguar_output open_loop(RPM status,RPM goal){
		if(goal>free_speed()) goal=free_speed();
		/*
		if(status>0.10*goal&&goal>1000) return Jaguar_output::voltageOut(0.75*(goal/62.5+4.199)/100); //Until one of us has time to figure 
		if(status>0.35*goal&&goal>1000) return Jaguar_output::voltageOut(0.79*(goal/62.5+4.199)/100); //A curve to do the this 'right' way
		if(status>0.60*goal&&goal>1000) return Jaguar_output::voltageOut(0.83*(goal/62.5+4.199)/100); //Just brute force it
		*/
		if(status>0.95*goal&&goal>1000) return Jaguar_output::voltageOut((goal/62.5+4.199)/100);
		return Jaguar_output::voltageOut(goal>1000);
	} 
	
	Output control(Status status,Goal goal){
		Output r;
		//Jaguar_output top=Jaguar_output::voltageOut(.25),bottom=Jaguar_output::voltageOut(.6);
		bool all_open_loop=0;
		r.top[Output::FEEDBACK]=all_open_loop?open_loop(status.top,goal.speed.top):Jaguar_output::speedOut(goal.speed.top);
		r.top[Output::OPEN_LOOP]=open_loop(status.top,goal.speed.top);
		r.bottom[Output::FEEDBACK]=all_open_loop?open_loop
				(status.bottom,goal.speed.bottom):Jaguar_output::speedOut(goal.speed.bottom);
		r.bottom[Output::OPEN_LOOP]=open_loop(status.bottom,goal.speed.bottom);
		return r;
	}

	ostream& operator<<(ostream& o,Calibration_manager c){
		return o<<"Shooter_wheels::Calibration_manager("<<c.calib<<")";
	}
	
	bool ready(Status status,Goal goal){
		if(goal.high_level==Shooter_wheels::STOP || 
				goal.high_level==Shooter_wheels::X || 
				goal.high_level==Shooter_wheels::HIGH_GOAL_NONBLOCK || 
				goal.high_level==Shooter_wheels::AUTO_SHOT_NONBLOCK) return 1;
		//this could be refined.
		return goal.speed.top>=status.top-100 &&
			goal.speed.top<=status.top+100 &&
			goal.speed.bottom>=status.bottom-25 &&
			goal.speed.bottom<=status.bottom+100;
	}
	
	//Goal convert_goal(wheelcalib /*c*/,PID_coefficients /*pid*/,High_level_goal /*g*/){
	Goal convert_goal(Calibration cal,High_level_goal g){
		wheelcalib c=cal.first;
		PID_coefficients pid=cal.second;
		switch(g){
			case TRUSS:
				return Goal(g,c.overtruss,pid); //Previously 1200 //PassLong Button uses lowgoal speeds
			case HIGH_GOAL:
			case HIGH_GOAL_NONBLOCK:
				return Goal(g,c.highgoal,pid); //Previously 3000
			case AUTO_SHOT_NONBLOCK:
			case AUTO_SHOT:
				return Goal(g,c.passing,pid); //Previously 2200
			case STOP:
			case X:
				return Goal(g,Shooter_wheels::Status(),pid);
			default:assert(0);
		}
	}
}

#ifdef SHOOTER_WHEELS_TEST
#include<vector>

int main(){
	using namespace Shooter_wheels;
	cout<<1<<endl;
	static const vector<High_level_goal> GOALS{HIGH_GOAL,TRUSS,STOP,X,AUTO_SHOT,AUTO_SHOT_NONBLOCK,HIGH_GOAL_NONBLOCK};
	cout<<2<<endl;
	for(auto goal:GOALS){
		//Control control;
		//assert(control.ready(goal,target_speed_top(goal,rpmsdefault()),target_speed_bottom(goal,rpmsdefault())));
		cout<<goal<<"\n";
	}
	cout<<3<<endl;
	/*{
		auto r=unlink("wheelrpms.txt");
		assert(r==0);
	}*/
	Calibration_manager c;
	cout<<4<<endl;
	cout<<c<<"\n";
	cout<<5<<endl;
	for(auto a:Calibration_target::all()){
		cout<<6<<endl;
		cout<<"-----------------------\n";
		for(double d:vector<double>{0,1.5,3.3}){
			cout<<a<<"\n";
			cout<<7<<endl;
			c.update(0,d,a,Panel::P,0);
			cout<<c.update(1,d,a,Panel::P,0).first-rpmsdefault()<<"\n";
			cout<<8<<endl;
		}
	}
	
	wheelcalib a;
	wheelcalib b;
	cout<<(a-b)<<"\n";
}
#endif
