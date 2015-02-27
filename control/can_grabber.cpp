#include "can_grabber.h"
#include <stdlib.h>
#include "../util/util.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

ostream& operator<<(ostream& o,Can_grabber::Input a){
	return o<<"Can_grabber::Input("<<a.sensor<<")";
}

ostream& operator<<(ostream& o,Can_grabber::Output a){
	#define X(NAME) if(a==Can_grabber::Output::NAME) return o<<""#NAME;
	X(ON) X(OFF)
	#undef X
	assert(0);
}

Can_grabber::Output control(Can_grabber::Status_detail status,Can_grabber::Goal goal){
	return ready(status,goal)?Can_grabber::Output::OFF:Can_grabber::Output::ON;
}

bool operator<(Can_grabber::Input a,Can_grabber::Input b){
	return a.sensor<b.sensor;
}

Can_grabber::Estimator::Estimator():last(Can_grabber::Status_detail::MID_UP){}

void Can_grabber::Estimator::update(Time time,Can_grabber::Input in,Can_grabber::Output out){
	timer.update(time,1);
	if (in.sensor) {
		if (last==Status_detail::MID_UP) {
			last=Status_detail::TOP;
		}
	} else if (out==Output::ON) {
		if (last==Status_detail::BOTTOM) {
			last=Status_detail::MID_UP;
		} else if (last==Status_detail::TOP) {
			last=Status_detail::MID_DOWN;
			static const double SECONDS=.5;
			timer.set(SECONDS);
		} else if (timer.done()) {
			last=Status_detail::BOTTOM;
		}
	}
}

Can_grabber::Status_detail Can_grabber::Estimator::get()const{ return last; }

ostream& operator<<(ostream& o,Can_grabber::Status_detail a){
	#define X(name) if(a==Can_grabber::Status_detail::name) return o<<""#name;
	X(TOP) X(MID_DOWN) X(BOTTOM) X(MID_UP)
	#undef X
	nyi
}

ostream& operator<<(ostream& o, Can_grabber::Estimator estimator) {
	o<<"Can_grabber::Estimator(";
	o<<"Last: "<<estimator.last<<"Countdown Timer: "<<estimator.timer;
	return o<<")";
}

ostream& operator<<(ostream& o, Can_grabber::Output_applicator output_applicator) {
	o<<"Can_grabber::Output_applicator(";
	o<<"Can Address: "<<output_applicator.can_address;
	return o<<")";
}

Can_grabber::Output_applicator::Output_applicator(int a):can_address(a){}

Can_grabber::Status status(Can_grabber::Status_detail const& a){
	return a;
}

Can_grabber::Can_grabber(int can_address):output_applicator(can_address){}

bool ready(Can_grabber::Status status,Can_grabber::Goal goal){
	if(goal==Can_grabber::Goal::TOP) return (status==Can_grabber::Status_detail::TOP);
	if(goal==Can_grabber::Goal::BOTTOM) return (status==Can_grabber::Status_detail::BOTTOM);
	assert(0);
}

std::ostream& operator<<(std::ostream& o,Can_grabber const& can_grabber){
	o<<"Can_grabber("<<"Estimator: "<<can_grabber.estimator<<" Output Applicator: "<<can_grabber.output_applicator;
	return o<<")";
}

std::set<Can_grabber::Status_detail> examples(Can_grabber::Status_detail*) {
	return {
		Can_grabber::Status_detail::TOP,
		Can_grabber::Status_detail::MID_DOWN,
		Can_grabber::Status_detail::BOTTOM,
		Can_grabber::Status_detail::MID_UP
	};
}

std::set<Can_grabber::Input> examples(Can_grabber::Input*) {
	return {
		Can_grabber::Input{0},
		Can_grabber::Input{1}
	};
}

std::set<Can_grabber::Output> examples(Can_grabber::Output*) {
	return {
		Can_grabber::Output::ON,
		Can_grabber::Output::OFF
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
	r.pwm[can_address]=(out==Can_grabber::Output::ON)?.5:0;
	return r;
}

Can_grabber::Output Can_grabber::Output_applicator::operator()(Robot_outputs r)const{
	return (r.pwm[can_address]>.25)?Can_grabber::Output::ON:Can_grabber::Output::OFF;
}

bool operator==(Can_grabber::Estimator const& a,Can_grabber::Estimator const& b){
	return a.last==b.last && a.timer==b.timer;
}

bool operator!=(Can_grabber::Estimator const& a,Can_grabber::Estimator const& b){ return !(a==b); }

bool operator==(Can_grabber::Output_applicator const& a,Can_grabber::Output_applicator const& b){
	return a.can_address==b.can_address;
}

bool operator==(Can_grabber const& a,Can_grabber const& b){
	return a.estimator==b.estimator && a.output_applicator==b.output_applicator;
}

bool operator!=(Can_grabber const& a,Can_grabber const& b){
	return !(a==b);
}

//std::set<Can_grabber::Goal> examples(Can_grabber::Goal*)nyi

#ifdef CAN_GRABBER_TEST
#include "formal.h"

int main(){
	tester(Can_grabber(4));
}
#endif
