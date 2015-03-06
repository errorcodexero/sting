#include "tote_sensors.h"
#include<set>
#include<iostream>
#include "../util/util.h"

using namespace std;

#define nyi { cout<<"error "<<__LINE__<<"\n"; exit(44); }

//todo: check these constants
static const unsigned IO_LEFT=7,IO_RIGHT=8,IO_CENTER=9;

Tote_sensors::Input Tote_sensors::Input_reader::operator()(Robot_inputs const& a)const{
	auto get=[&](unsigned index){
		return a.digital_io.in[index]==Digital_in::_1;
	};
	return Input{get(IO_LEFT),get(IO_RIGHT),get(IO_CENTER)};
}

Robot_inputs Tote_sensors::Input_reader::operator()(Robot_inputs all,Tote_sensors::Input value)const{
	auto set=[&](unsigned index,bool value){
		all.digital_io.in[index]=value?Digital_in::_1:Digital_in::_0;
	};
	set(IO_LEFT,value.left);
	set(IO_RIGHT,value.right);
	set(IO_CENTER,value.center);
	return all;
}

bool operator<(Tote_sensors::Input const& a,Tote_sensors::Input const& b){
	#define CMP(NAME) if(a.NAME<b.NAME) return 1; if(b.NAME<a.NAME) return 0;
	CMP(left) CMP(right) CMP(center)
	#undef CMP
	return 0;
}

bool operator==(Tote_sensors::Input const& a,Tote_sensors::Input const& b){
	return a.left==b.left && a.right==b.right && a.center==b.center;
}

set<Tote_sensors::Input> examples(Tote_sensors::Input*){
	set<Tote_sensors::Input> r;
	vector<bool>  v{0,1};
	for(auto a:v){
		for(auto b:v){
			for(auto c:v){
				r|=Tote_sensors::Input{a,b,c};
			}
		}
	}
	return r;
}

ostream& operator<<(ostream& o,Tote_sensors::Input const& a){
	o<<"Input(";
	o<<a.left<<" "<<a.center<<" "<<a.right;
	return o<<")";
}

bool operator<(Tote_sensors::Output,Tote_sensors::Output){ return 0; }
bool operator==(Tote_sensors::Output,Tote_sensors::Output){ return 1; }
bool operator!=(Tote_sensors::Output a,Tote_sensors::Output b){ return !(a==b); }

ostream& operator<<(ostream& o,Tote_sensors::Output){
	return o<<"Tote_sensors::Output()";
}

set<Tote_sensors::Output> examples(Tote_sensors::Output*){
	return {Tote_sensors::Output{}};
}

void Tote_sensors::Estimator::update(Time,Input,Output){}

Tote_sensors::Status_detail Tote_sensors::Estimator::get()const{
	return Tote_sensors::Status_detail{};
}

Robot_outputs Tote_sensors::Output_applicator::operator()(Robot_outputs a,Output)const{
	return a;
}

Tote_sensors::Output Tote_sensors::Output_applicator::operator()(Robot_outputs)const{
	return Tote_sensors::Output{};
}

ostream& operator<<(ostream& o,Tote_sensors const&){
	return o<<"Tote_sensors";
}

Tote_sensors::Output control(Tote_sensors::Status,Tote_sensors::Goal){ return Tote_sensors::Output{}; }
bool ready(Tote_sensors::Status,Tote_sensors::Goal){ return 1; }
Tote_sensors::Status status(Tote_sensors::Status_detail a){ return a; }

#ifdef TOTE_SENSORS_TEST
#include "formal.h"

int main(){
	Tester_mode m;
	m.check_multiple_outputs=0;
	tester(Tote_sensors(),m);
}
#endif
