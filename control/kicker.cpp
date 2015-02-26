#include "kicker.h"
#include <stdlib.h>
#include "../util/util.h"
#include "../util/interface.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

Kicker::Estimator::Estimator():last(Kicker::Status_detail::HALF_OUT){}

void Kicker::Estimator::update(Time,Kicker::Input in,Kicker::Output out){
	if(out==Output::IN){
		last=Status_detail::IN;
	}
	else if(out==Output::OUT){
		last=Status_detail::OUT;
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

/*Robot_outputs Kicker::Output_applicator::operator()(Robot_outputs robot,Kicker::Output kicker)const{
	nyi
}*/

/*Kicker::Output Kicker::Output_applicator::operator()(Robot_outputs in)const{
	nyi
}*/

std::ostream& operator<<(std::ostream& o,Kicker::Input const& a){
	o<<"kicker::Input(";
	o<<"";
	return o<<")";
}


std::ostream& operator<<(std::ostream& o,Kicker const& kicker){
	o<<"Kicker("<<"Estimator: "<<kicker.estimator<<" Output Applicator: "<<kicker.output_applicator;
	return o<<")";
}

/*
std::set<Kicker::Input> examples(Kicker::Input*){
	nyi
}*/

//std::set<Kicker::Output> examples(Kicker::Output*){ return {0,1}; }

Kicker::Status status(Kicker::Status_detail const& a){
	return a;
}

/*Kicker::Output control(){
	nyi
}*/

Kicker::Kicker(int kicker_address):output_applicator(kicker_address){}

/*bool ready(){
	nyi
}*/

#ifdef KICKER_TEST
/*struct Kicker_sim{
	nyi
};*/

int main(){
	
}
#endif
