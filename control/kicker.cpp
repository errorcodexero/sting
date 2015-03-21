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
	static const float KICKER_IN_TIME=0.1;
	static const float KICKER_OUT_TIME=0.2;
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

Robot_inputs Kicker::Input_reader::operator()(Robot_inputs all,Input)const{ return all; }

Kicker::Input Kicker::Input_reader::operator()(Robot_inputs)const{ return Kicker::Input{}; }

Kicker::Input::Input(bool){}

bool operator<(Kicker::Input,Kicker::Input){
	return 0;
}

bool operator==(Kicker::Input,Kicker::Input){
	return 1;
}

bool operator!=(Kicker::Input a,Kicker::Input b){
	return !(a==b);
}

Kicker::Output_applicator::Output_applicator(int a):kicker_address(a){}

Robot_outputs Kicker::Output_applicator::operator()(Robot_outputs robot,Kicker::Output kicker)const{
	robot.solenoid[kicker_address]=kicker==Kicker::Output::OUT;
	return robot;
}

Kicker::Output Kicker::Output_applicator::operator()(Robot_outputs in)const{
	if(in.solenoid[kicker_address]==1) return Kicker::Output::OUT;
	return Kicker::Output::IN;
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
	set<Kicker::Input> s;
	s.insert(Kicker::Input{});
	assert(s.size());
	return s;
	//return set<Kicker::Input> {Kicker::Input{}};
}

std::set<Kicker::Output> examples(Kicker::Output*){ 
	set<Kicker::Output> outputs;
	outputs.insert(Kicker::Output::IN);
	outputs.insert(Kicker::Output::OUT);
	return outputs;
}

Kicker::Status status(Kicker::Status_detail const& a){
	return a;
}

Kicker::Output control(Kicker::Status_detail /*status*/,Kicker::Goal goal){
	if(goal==Kicker::Goal::IN) return Kicker::Output::IN;
	if(goal==Kicker::Goal::OUT) return Kicker::Output::OUT;
	nyi
}

set<Kicker::Status_detail> examples(Kicker::Status_detail*){
	set<Kicker::Status_detail> status;
	status.insert(Kicker::Status_detail::IN);
	status.insert(Kicker::Status_detail::HALF_OUT);
	status.insert(Kicker::Status_detail::OUT);
	return status;
}

Kicker::Kicker(int kicker_address):output_applicator(kicker_address){}

bool ready(Kicker::Status status,Kicker::Goal goal){
	if(goal==Kicker::Goal::IN) return status==Kicker::Status_detail::IN;
	if(goal==Kicker::Goal::OUT) return status==Kicker::Status_detail::OUT;
	nyi
}

bool operator==(Kicker::Estimator const& a,Kicker::Estimator const& b){
	return a.last_output==b.last_output && a.output_start==b.output_start && a.last==b.last;
}

bool operator!=(Kicker::Estimator const& a,Kicker::Estimator const& b){
	return !(a==b);
}

bool operator==(Kicker::Output_applicator const& a,Kicker::Output_applicator const& b){
	return a.kicker_address==b.kicker_address;
}

bool operator==(Kicker const& a,Kicker const& b){ return a.estimator==b.estimator && a.output_applicator==b.output_applicator; }

bool operator!=(Kicker const& a,Kicker const& b){ return !(a==b); }

#ifdef KICKER_TEST
#include "formal.h"

/*struct Kicker_sim{
	nyi
};*/

int main(){
	Kicker a(0);
	tester(a);

	Time t=0;
	static const Time TIMESTEP=.01;
	Kicker::Goal goal=Kicker::Goal::IN;
	cout<<"time\tgoal\tstatus\n";
	auto step=[&](){
		auto out=control(a.estimator.get(),goal);
		a.estimator.update(t,Kicker::Input{},out);
		cout<<t<<"\t"<<goal<<"\t"<<a.estimator.get()<<"\t"<<a.estimator<<"\n";
		t+=TIMESTEP;
	};
	static const Time TIME_LIMIT=10;

	for(unsigned i=0;i<10;i++){
		step();
	}

	for(auto g:examples((Kicker::Goal*)0)){
		goal=g;
		cout<<g<<"\n";
		while(!ready(a.estimator.get(),goal) && t<TIME_LIMIT){
			step();
		}
		assert(t<TIME_LIMIT);
		cout<<"go to: "<<goal<<"\n";
	}
}
#endif
