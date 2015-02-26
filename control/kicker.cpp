#include "kicker.h"
#include <stdlib.h>
#include "../util/util.h"
#include "../util/interface.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

Kicker::Estimator::Estimator():last(Kicker::Status_detail::HALF_OUT){}

bool operator==(Kicker::Output a,Kicker::Status_detail b){
	if(a==Kicker::Output::IN && b==Kicker::Status_detail::IN) return 1;
	if(a==Kicker::Output::OUT && b==Kicker::Status_detail::OUT) return 1;
	return 0;
}

bool operator!=(Kicker::Output a,Kicker::Status_detail b){
	return !(a==b);
}

void Kicker::Estimator::update(Time time,Kicker::Input /*in*/,Kicker::Output out){
	static const float KICKER_IN_TIME=0.0;
	static const float KICKER_OUT_TIME=0.0;
	if(out!=last_output){
		output_start=time;
		last_output=out;
	}
	float output_duration=time-output_start;
	if(out==Output::IN && output_duration>=KICKER_IN_TIME){
		last=Status_detail::IN;
	}
	else if(out==Output::OUT && output_duration>=KICKER_OUT_TIME){
		last=Status_detail::OUT;
	}	
	else {
		last=Status_detail::HALF_OUT;
	}
}

Kicker::Status_detail Kicker::Estimator::get()const{ return last; }

ostream& operator<<(ostream& o,Kicker::Status_detail a){
	#define X(name) if(a==Kicker::Status_detail::name) return o<<""#name;
	X(IN) X(HALF_OUT) X(OUT)
	#undef X
	nyi
}

ostream& operator<<(ostream& o, Kicker::Estimator estimator) {
	o<<"Kicker::Estimator(";
	o<<"Last: "<<estimator.last;
	return o<<")";
}

ostream& operator<<(ostream& o, Kicker::Output_applicator output_applicator) {
	o<<"Kicker::Output_applicator(";
	o<<"Kicker Address: "<<output_applicator.kicker_address;
	return o<<")";
}

Kicker::Output_applicator::Output_applicator(int a):kicker_address(a){}

Robot_outputs Kicker::Output_applicator::operator()(Robot_outputs /*robot*/,Kicker::Output /*kicker*/)const{
	nyi
}

Kicker::Output Kicker::Output_applicator::operator()(Robot_outputs /*in*/)const{
	nyi
}

std::ostream& operator<<(std::ostream& o,Kicker::Input const& /*a*/){
	return o<<"";
	nyi
}

std::ostream& operator<<(std::ostream& o,Kicker const& kicker){
	o<<"Kicker("<<"Estimator: "<<kicker.estimator<<" Output Applicator: "<<kicker.output_applicator;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Kicker::Output out){
	#define X(name) if(out==Kicker::Output::name) return o<<""#name;
	X(IN) X(OUT)
	#undef X
	nyi
}

std::set<Kicker::Input> examples(Kicker::Input*){
	nyi
}

std::set<Kicker::Output> examples(Kicker::Output*){ 
 nyi
}

Kicker::Status status(Kicker::Status_detail const& a){
	return a;
}

Kicker::Output control(Kicker::Status_detail,Kicker::Goal){
	nyi
}

set<Kicker::Status_detail> examples(Kicker::Status_detail*){
	nyi
}

Kicker::Kicker(int kicker_address):output_applicator(kicker_address){}

bool ready(Kicker::Status,Kicker::Goal){
	nyi
}

#ifdef KICKER_TEST
#include "formal.h"

/*struct Kicker_sim{
	nyi
};*/

int main(){
	Kicker kicker(0);
	tester(kicker);
}
#endif
