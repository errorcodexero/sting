#include "can_grabber.h"
#include <stdlib.h>
#include "../util/util.h"
#include "../util/interface.h"

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

#ifdef CAN_GRABBER_TEST
int main(){

}
#endif