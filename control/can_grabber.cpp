#include "can_grabber.h"
#include <stdlib.h>
#include "../util/util.h"
#include "monitor.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

#define wall { cout<<"\nYou hit a brick wall at line "<<__LINE__<<"\n"; exit(44); }

Can_grabber::Input_reader::Input_reader(unsigned int tote_lift):lift(tote_lift) {}
ostream& operator<<(std::ostream& print, Can_grabber::Input const& grabber_info){ 
	print<<grabber_info.lift<<" "<<grabber_info.grab_down<<"\n";
	return print;
}


//Can_grabber::Input::Input(Lift::Input, bool, bool) wall
std::set<Can_grabber::Input> examples(Can_grabber::Input*) {
	std::set<Can_grabber::Input> can_in;
	for(auto lift:examples((Lift::Input*)0)){
		for(unsigned i=0;i<2;i++){
			can_in|=Can_grabber::Input{lift,(bool)i};
		}
	}
	return can_in;
}

static const unsigned CAN_GRABBER_IO=6;

Robot_inputs Can_grabber::Input_reader::operator()(Robot_inputs rtn, Can_grabber::Input in) const {
	rtn=lift(rtn,in.lift);
	rtn.digital_io.in[CAN_GRABBER_IO]=in.grab_down?Digital_in::_1:Digital_in::_0;
	return rtn;
}

Can_grabber::Input Can_grabber::Input_reader::operator()(Robot_inputs in) const {
	return Can_grabber::Input{
		lift(in),
		in.digital_io.in[CAN_GRABBER_IO]==Digital_in::_1
	};
}

bool operator!=(Can_grabber::Input const& a, Can_grabber::Input const& b) {
	return !(a==b);
}

bool operator==(Can_grabber::Input const& a, Can_grabber::Input const& b) {
	return a.lift==b.lift && a.grab_down==b.grab_down;
}

bool operator<(Can_grabber::Input const& a, Can_grabber::Input const& b){
	if(a.lift<b.lift) return 1;
	if(b.lift<a.lift) return 0;
	return a.grab_down<b.grab_down;
}

ostream& operator<<(ostream& o,Can_grabber::Output a){
	#define X(NAME) if(a==Can_grabber::Output::NAME) return o<<""#NAME;
	X(RELEASE) X(LOCK)
	#undef X
	assert(0);
}

Can_grabber::Output control(Can_grabber::Status_detail const& status,Can_grabber::Goal goal){
	switch(status.status){
		case Can_grabber::Status::INITIAL:
			return (goal==Can_grabber::Goal::TOP)?Can_grabber::Output::LOCK:Can_grabber::Output::RELEASE;
		case Can_grabber::Status::GETTING_STUCK:
			return Can_grabber::Output::LOCK;
		case Can_grabber::Status::GOING_UP:
		case Can_grabber::Status::STUCK_UP:
		case Can_grabber::Status::GOING_DOWN:
		case Can_grabber::Status::SETTLING_DOWN:
		case Can_grabber::Status::DOWN:
			return Can_grabber::Output::RELEASE;
		default: assert(0);
	}
}

Can_grabber::Estimator::Estimator():last(Status::INITIAL){}

void Can_grabber::Estimator::update(Time time,Can_grabber::Input in,Can_grabber::Output out){
	lift.update(time,in.lift,Lift::Output{}); //we're lucky that the lift's estimator doesn't depend on its output at the moment
	auto height=lift.get().inches_off_ground();

	static const double LIFT_THRESHOLD=3; //level at which the mechanism would fail to get to the bottom
	static const double LIFT_FULL=55; //level where the mechanism is fully folded in

	const bool part_up=height>LIFT_THRESHOLD;
	const bool full_up=height>LIFT_FULL;

	switch(last){
		case Can_grabber::Status::INITIAL:
			if(out==Output::RELEASE){
				static const double DROP_TIME=2.5;
				timer.set(DROP_TIME);
				timer.update(time,0);
				last=Status::GOING_DOWN;
				return;
			}
			if(part_up){
				last=Status::GETTING_STUCK;
			}
			break;
		case Can_grabber::Status::GOING_DOWN:
			timer.update(time,out==Output::RELEASE);
			if(timer.done() || in.grab_down){
				static const double SETTLING_TIME=1.5;
				timer.set(SETTLING_TIME);
				timer.update(time,0);
				last=Status::SETTLING_DOWN;
				return;
			}
			/*
			if(switch
			*/
			if(part_up){
				last=Status::GETTING_STUCK;
			}
			break;
		case Can_grabber::Status::SETTLING_DOWN:
			timer.update(time,1);
			if(timer.done()){
				last=Status::DOWN;
			}
			break;
		case Can_grabber::Status::DOWN:
			if(part_up) last=Status::GOING_UP;
			break;
		case Can_grabber::Status::GOING_UP:
			if(full_up) last=Status::STUCK_UP;
			break;
		case Can_grabber::Status::STUCK_UP:
			break;
		case Can_grabber::Status::GETTING_STUCK:
			if(full_up){
				last=Status::STUCK_UP;
				return;
			}
			if(out==Output::RELEASE){
				last=Status::GOING_UP;
			}
			break;
		default: assert(0);
	}
}

Can_grabber::Status_detail Can_grabber::Estimator::get()const{
	return Can_grabber::Status_detail{last,lift.get()};
}

ostream& operator<<(ostream& o,Can_grabber::Status_detail  a){
	o<<"Can_grabber::Status_detail(";
	o<<a.status<<","<<a.lift;
	return o<<")";
}

ostream& operator<<(ostream& o, Can_grabber::Estimator estimator) {
	o<<"Can_grabber::Estimator(";
	o<<"Last: "<<estimator.last<<"Countdown Timer: "<<estimator.timer;
	return o<<")";
}

ostream& operator<<(ostream& o, Can_grabber::Output_applicator output_applicator) {
	o<<"Can_grabber::Output_applicator(";
	o<<"Can Address: "<<output_applicator.pwm;
	return o<<")";
}

Can_grabber::Output_applicator::Output_applicator(int a):pwm(a){}

Can_grabber::Status status(Can_grabber::Status_detail const& status){
	return status.status;
}

Can_grabber::Can_grabber(int sensor_dio,int pwm):input_reader(sensor_dio),output_applicator(pwm){}

bool ready(Can_grabber::Status status,Can_grabber::Goal goal){
	if(goal==Can_grabber::Goal::TOP){
		switch(status){
			case Can_grabber::Status::INITIAL:
			case Can_grabber::Status::GETTING_STUCK:
			case Can_grabber::Status::STUCK_UP:
				return 1;
			case Can_grabber::Status::GOING_UP:
			case Can_grabber::Status::GOING_DOWN:
			case Can_grabber::Status::SETTLING_DOWN:
			case Can_grabber::Status::DOWN:
				return 0;
			default: assert(0);
		}
	}else if(goal==Can_grabber::Goal::BOTTOM){
		switch(status){
			case Can_grabber::Status::INITIAL: return 0;
			case Can_grabber::Status::GOING_UP: return 0;
			case Can_grabber::Status::GOING_DOWN: return 0;
			case Can_grabber::Status::SETTLING_DOWN: return 0;
			case Can_grabber::Status::DOWN: return 1;
			case Can_grabber::Status::STUCK_UP:
			case Can_grabber::Status::GETTING_STUCK:
				return 0;
			default: assert(0);
		}
	}
	/*if(goal==Can_grabber::Goal::TOP) return (status==Can_grabber::Status_detail::TOP);
	if(goal==Can_grabber::Goal::BOTTOM) return (status==Can_grabber::Status_detail::BOTTOM);
	assert(0);*/nyi;
}

std::ostream& operator<<(std::ostream& o,Can_grabber const& can_grabber){
	o<<"Can_grabber("<<"Estimator: "<<can_grabber.estimator<<" Output Applicator: "<<can_grabber.output_applicator;
	return o<<")";
}

std::set<Can_grabber::Status_detail> examples(Can_grabber::Status_detail*) {
	set<Can_grabber::Status_detail> r;
	for(auto status:examples((Can_grabber::Status*)0)){
		for(auto l:examples((Lift::Status*)0)){
			r|=Can_grabber::Status_detail{status,l};
		}
	}
	return  r;

	/*return {
		Can_grabber::Status_detail::TOP,
		Can_grabber::Status_detail::MID_DOWN,
		Can_grabber::Status_detail::BOTTOM,
		Can_grabber::Status_detail::MID_UP
	};*/
}

//std::set<Can_grabber::Input> examples(Can_grabber::Input*) {
//nyi/*	return {
/*		Can_grabber::Input{0},
		Can_grabber::Input{1}
	};*/
//}*/

std::set<Can_grabber::Output> examples(Can_grabber::Output*) {
	return {
		Can_grabber::Output::RELEASE,
		Can_grabber::Output::LOCK
	};
}

std::set<Can_grabber::Goal> examples(Can_grabber::Goal*){
	return {Can_grabber::Goal::TOP,Can_grabber::Goal::BOTTOM};
}

ostream& operator<<(ostream& o,Can_grabber::Goal a){
	#define X(NAME) if(a==Can_grabber::Goal::NAME) return o<<""#NAME;
	X(TOP) X(BOTTOM)
	#undef X
	assert(0);
}

Robot_outputs Can_grabber::Output_applicator::operator()(Robot_outputs r,Output out)const{
	r.solenoid[2]=(out==Output::RELEASE);
	return r;
}

Can_grabber::Output Can_grabber::Output_applicator::operator()(Robot_outputs r)const{
	return r.solenoid[2]?Output::RELEASE:Output::LOCK;
}

bool operator==(Can_grabber::Estimator const& a,Can_grabber::Estimator const& b){
	return a.last==b.last && a.timer==b.timer;
}

bool operator!=(Can_grabber::Estimator const& a,Can_grabber::Estimator const& b){ return !(a==b); }

bool operator==(Can_grabber::Output_applicator const& a,Can_grabber::Output_applicator const& b){
	return a.pwm==b.pwm;
}

bool operator==(Can_grabber const& a,Can_grabber const& b){
	return a.estimator==b.estimator && a.output_applicator==b.output_applicator;
}

bool operator!=(Can_grabber const& a,Can_grabber const& b){
	return !(a==b);
}

//std::set<Can_grabber::Goal> examples(Can_grabber::Goal*)nyi
struct Sim{
	Time last=-1;
	double angle;//radians, 0=up, goes clockwise

	public:
	Sim():angle(0){}

	void update(Time /*now*/,Can_grabber::Output /*out*/){
		/*if(last==-1){
			last=now;
			return;
		}
		Time elapsed=now-last;
		last=now;
		static const Time CYCLE_TIME=4;
		switch(out){
			case Can_grabber::Output::ON:
				angle+=elapsed/CYCLE_TIME*2*M_PI;
				while(angle>2*M_PI) angle-=2*M_PI;
				return;
			case Can_grabber::Output::OFF:return;
			default: assert(0);
		}*/
	}

	Can_grabber::Input get()const{
		return Can_grabber::Input{
			Lift::Input{0,0,0,0},0
		};
		//just choosing some port of the travel where the sensor is on
		nyi//return .4<angle && angle<1;
	}
};

set<Can_grabber::Status> examples(Can_grabber::Status*){
	return {
		#define X(NAME) Can_grabber::Status::NAME,
		CAN_GRABBER_STATUS
		#undef X
	};
}

ostream& operator<<(ostream& o,Can_grabber::Status a){
	#define X(NAME) if(a==Can_grabber::Status::NAME) return o<<""#NAME;
	CAN_GRABBER_STATUS
	#undef X
	assert(0);
}

bool operator<(Can_grabber::Status_detail const& a,Can_grabber::Status_detail const& b){
	if(a.status<b.status) return 1;
	if(b.status<a.status) return 0;
	return a.lift<b.lift;
}

bool operator==(Can_grabber::Status_detail const& a,Can_grabber::Status_detail const& b){
	return a.status==b.status && a.lift==b.lift;
}

bool operator!=(Can_grabber::Status_detail const& a,Can_grabber::Status_detail const& b){
	return !(a==b);
}

#ifdef CAN_GRABBER_TEST
#include "formal.h"
template<typename T>
string clip1(T t){
	return as_string(t).substr(0,4);
}

int main(){
	//numbers are totally made up
	Can_grabber c(6,4);

	tester(c);

	Sim s;
	static const Time STEPSIZE=.01;
	Monitor<Can_grabber::Input> sensor;
	Monitor<Can_grabber::Status_detail> status;
	Can_grabber::Goal goal=Can_grabber::Goal::TOP;
	cout<<"time\tangle\tinput\t\t\toutput\n";
	Can_grabber::Output out=Can_grabber::Output::LOCK;
	Time f=0;
	auto step=[&](){
		c.estimator.update(f,s.get(),out);
		out=control(c.estimator.get(),goal);
		s.update(f,out);
		cout<<sensor.update(s.get());
		cout<<status.update(c.estimator.get());
		cout<<f<<"\t"<<clip1(s.angle)<<"\t"<<s.get()<<"\t"<<out<<"\t"<<c.estimator<<"\n";
		f+=STEPSIZE;
	};
	for(;c.estimator.get().status!=Can_grabber::Status::INITIAL && f<10;){
		step();
	}
	if(c.estimator.get().status!=Can_grabber::Status::INITIAL){
		nyi
	}
	goal=Can_grabber::Goal::BOTTOM;
	while(c.estimator.get().status!=Can_grabber::Status::DOWN && f<13){
		step();
	}
	if(c.estimator.get().status!=Can_grabber::Status::DOWN){
		cout<<c.estimator.get().status;
		nyi
	}
	goal=Can_grabber::Goal::TOP;
	for(unsigned i=0;i<10;i++){
		step();
	}
	return 0;
}
#endif
