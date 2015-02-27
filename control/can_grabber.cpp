#include "can_grabber.h"
#include <stdlib.h>
#include "../util/util.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

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

bool ready(Can_grabber::Status status,Can_grabber::Goal goal){//NO
	if(goal==Can_grabber::Goal::ON) return (status==Can_grabber::Status_detail::TOP);//FIX
	if(goal==Can_grabber::Goal::OFF) return (status==Can_grabber::Status_detail::MID_DOWN);//THIS
	nyi
}

std::ostream& operator<<(std::ostream& o,Can_grabber const& can_grabber){
	o<<"Can_grabber("<<"Estimator: "<<can_grabber.estimator<<" Output Applicator: "<<can_grabber.output_applicator;
	return o<<")";
}

std::set<Can_grabber::Status_detail> examples(*Status_detail) {
		return {
			Can_grabber::Status_detail::TOP,
			Can_grabber::Status_detail::MID_DOWN,
			Can_grabber::Status_detail::BOTTOM,
			Can_grabber::Status_detail::MID_UP
		}
}

std::set<Can_grabber::Input> examples(*Input) {
	return {
		Can_grabber::Input{0},
		Can_grabber::Input{1}
	}
}

std::set<Can_grabber::Output> examples(*Output) {
	return {
		Can_grabber::Output::ON,
		Can_grabber::Output::OFF
	}
}

#ifdef CAN_GRABBER_TEST
int main(){

}
#endif