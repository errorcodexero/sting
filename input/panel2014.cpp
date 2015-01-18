#include "panel2014.h"
#include<stdlib.h>
#include "util.h"
#include "../util/util.h"

using namespace std;

Mode_buttons::Mode_buttons():
	drive_wo_ball(0),
	drive_w_ball(0),
	collect(0),
	shoot_high(0),
	truss_toss(0),
	eject(0),
	catch_mode(0),
	auto_shot(0)
{}

ostream& operator<<(ostream& o,Panel::Auto_mode a){
	if(a==Panel::DO_NOTHING)o<<"Auto-mode:DO_NOTHING";
	if(a==Panel::ONE_BALL)o<<"Auto-mode:ONE_BALL";
	if(a==Panel::TWO_BALL)o<<"Auto-mode:TWO_BALL";
	if(a==Panel::MOVE)o<<"Auto-mode:MOVE";
	return o;
}

ostream& operator<<(ostream& o,Panel::PIDselect a){
	if(a==Panel::P)o<<"PID:P";
	if(a==Panel::I)o<<"PID:I";
	if(a==Panel::D)o<<"PID:D";
	if(a==Panel::PID_NONE)o<<"PID:PID_NONE";
	return o;
}
	
ostream& operator<<(ostream& o,Mode_buttons m){
	o<<"Mode_buttons( ";
	#define X(name) o<<""#name<<":"<<m.name<<" ";
	X(drive_wo_ball)
	X(drive_w_ball)
	X(collect)
	X(shoot_high)
	X(truss_toss)
	X(eject)
	X(catch_mode)
	X(auto_shot)
	#undef X
	return o<<")";
}

vector<Panel::Auto_mode> automodes(){
	vector<Panel::Auto_mode> a;
	a.push_back(Panel::DO_NOTHING);
	a.push_back(Panel::ONE_BALL);
	a.push_back(Panel::TWO_BALL);
	a.push_back(Panel::MOVE);
	return a;
}

vector<Panel::PIDselect> pids(){
	vector<Panel::PIDselect> a;
	a.push_back(Panel::P);
	a.push_back(Panel::I);
	a.push_back(Panel::D);
	a.push_back(Panel::PID_NONE);
	return a;
}

Panel::Auto_mode automodeconvert(int potin){
	if(potin==0)return Panel::DO_NOTHING;
	if(potin==1)return Panel::ONE_BALL;
	if(potin==2)return Panel::TWO_BALL;
	if(potin==3)return Panel::MOVE;
	return Panel::DO_NOTHING;
}

Panel::PIDselect PIDconvert(int potin){
	if(potin==4)return Panel::P;
	if(potin==5)return Panel::I;
	if(potin==6)return Panel::D;
	return Panel::PID_NONE;
}

Panel::Panel():
	auto_mode(DO_NOTHING),
	pidselect(PID_NONE),
	fire(0),
	pass_now(0),
	pidadjust(0),
	speed(0),
	learn(0),
	force_wheels_off(0)
{}

ostream& operator<<(ostream& o,Panel p){	
	o<<"Panel( ";
	#define X(name) o<<""#name<<":"<<p.name<<" ";
	o<<p.mode_buttons;
	X(fire)
	X(pass_now)
	X(target)
//	X(speed)
	o<<"spd:"<<((int)(p.speed)*10)/10<<" ";
	X(learn)
	X(force_wheels_off)
	X(collector)
	X(collector_tilt)
	X(injector)
	//X(injector_arms)
	X(ejector)
	X(auto_mode)
	X(pidselect)
	X(pidadjust)
	#undef X
	return o<<")";
}

Calibration_target interpret_target(double f){
	/*int x=a+2*b+4*c;
	switch(x){
		case 0: return Fire_control::NO_TARGET;
		case 1: return Fire_control::HIGH;
		case 2: return Fire_control::TRUSS;
		case 3: return Fire_control::PASS;
		case 4: return Fire_control::EJECT;
		default:
			//may want to add some way of returning an error;
			return Fire_control::NO_TARGET;
	}*/
	//TODO: Measure the analog values of the switch.
	int i=interpret_10_turn_pot(f/3.3*5);
	Calibration_target r;
	r.top=i%2;
	//int x=i/2;
	//cerr<<"x="<<x<<"\r\n";
	switch(i/2){
		case 0:
			r.target=Fire_control::HIGH;
			break;
		case 1:
			r.target=Fire_control::TRUSS;
			break;
		case 2:
			r.target=Fire_control::AUTO_SHOT;
			break;
		//case 9:
		default:
			r.direct_mode=1;
			break;
		//default:
		//	r.target=Fire_control::NO_TARGET;
	}
	return r;
}

Maybe<Collector_mode> interpret_collector(double analog){
	//assuming that analog goes from 0 to 1.  This is likely not right, I think it may be actual voltages.
	int x=analog*4;
	if(analog==4) analog=3;
	switch(x){
		case 0: return Maybe<Collector_mode>();
		case 1: return Maybe<Collector_mode>(Collector_mode::ON);
		case 2: return Maybe<Collector_mode>(Collector_mode::OFF);
		case 3: return Maybe<Collector_mode>(Collector_mode::REVERSE);
		default:
			//may want to add error detection here.
			return Maybe<Collector_mode>();
	}
}

pair<int,int> demux_3x3(double analog){
	int x=(int)analog;//todo: fix scaling.
	if(x==9) x=8;
	return make_pair(x%3,x/3);
}

Maybe<Collector_tilt::Output> interpret_collector_tilt(int x){
	switch(x){
		case 0: return Maybe<Collector_tilt::Output>();
		case 1: return Maybe<Collector_tilt::Output>(Collector_tilt::OUTPUT_UP);
		case 2: return Maybe<Collector_tilt::Output>(Collector_tilt::OUTPUT_DOWN);
		default: assert(0);
	}
}
Maybe<Injector::Output> interpret_injector(int x){
	switch(x){
		case 0: return Maybe<Injector::Output>();
		case 1: return Maybe<Injector::Output>(Injector::OUTPUT_DOWN);
		case 2: return Maybe<Injector::Output>(Injector::OUTPUT_UP);
		default: assert(0);
	}
}

Maybe<Injector_arms::Output> interpret_injector_arms(int x){
	switch(x){
		case 0: return Maybe<Injector_arms::Output>();
		case 1: return Maybe<Injector_arms::Output>(Injector_arms::OUTPUT_OPEN);
		case 2: return Maybe<Injector_arms::Output>(Injector_arms::OUTPUT_CLOSE);
		default: assert(0);
	}
}

Maybe<Ejector::Output> interpret_ejector(int x){
	switch(x){
		case 0: return Maybe<Ejector::Output>();
		case 1: return Maybe<Ejector::Output>(Ejector::OUTPUT_UP);
		case 2: return Maybe<Ejector::Output>(Ejector::OUTPUT_DOWN);
		default: assert(0);
	}
}

Panel interpret(Driver_station_input d){
	Panel panel;
	{
		int i=interpret_10_turn_pot(d.analog[0]/3.3*5);
		panel.pidselect=PIDconvert(i);
		panel.auto_mode=automodeconvert(i);
	}
	{
		double x=d.analog[3];
		panel.mode_buttons.drive_wo_ball=x>3.1;
		panel.mode_buttons.drive_w_ball=(x<2.9 && x>2.6);
		panel.mode_buttons.collect=(x<2.35 && x>2.05);
		panel.mode_buttons.shoot_high=!d.digital[0];
		panel.mode_buttons.truss_toss=(x<1.8 && x>1.5);
		panel.mode_buttons.auto_shot=(x<.6&&x>.4);
		panel.mode_buttons.eject=(x<1.25 && x>.95);
		panel.pass_now=(x<.7&&x>.4);
	}
	panel.mode_buttons.catch_mode=!d.digital[2];

	panel.fire=!d.digital[1];

	panel.target=interpret_target(d.analog[2]);
	panel.speed=d.analog[1];
	panel.force_wheels_off=d.digital[3];//note: Which way is on/off is not labeled on the console yet.  
	
	{
		double x=d.analog[4];
		if(x>2 && x<2.35) panel.injector=Injector::OUTPUT_UP;
		if(x>1.65 && x<2) panel.ejector=Ejector::OUTPUT_UP;
		if(x>2.7 && x<3.10) panel.collector_tilt=Collector_tilt::OUTPUT_UP;
		if(x>2.35 && x<2.65) panel.collector_tilt=Collector_tilt::OUTPUT_DOWN;
		if(x>1.35 && x<1.75) panel.collector=Collector_mode::ON;
		if(x>1.05 && x<1.35) panel.collector=Collector_mode::REVERSE;
		if(x>.4 && x<1) panel.learn=1;
		if(x>.2&&x<.4)panel.pidadjust=1;
	}
	
	//panel.learn=TBD
	return panel;
}

#ifdef PANEL2014_TEST

//this should probably be moved to interface.cpp
Driver_station_input driver_station_input_rand(){
	Driver_station_input r;
	for(unsigned i=0;i<r.ANALOG_INPUTS;i++){
		r.analog[i]=(0.0+rand()%101)/100;
	}
	for(unsigned i=0;i<r.DIGITAL_INPUTS;i++){
		r.digital[i]=rand()%2;
	}
	return r;
}

int main(){
	Panel p;
	//cout<<p<<"\n";
	//cout<<interpret(Driver_station_input())<<"\n";
	for(unsigned i=0;i<50;i++){
		interpret(driver_station_input_rand());
	}
	//cout<<automodes()<<endl;
	//cout<<pids()<<endl;
	cout<<Panel()<<endl;
}
#endif
