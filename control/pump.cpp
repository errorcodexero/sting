#include "pump.h"
#include<iostream>
#include<cassert>
#include<vector>
#include "../util/util.h"

using namespace std;

namespace Pump{
	ostream& operator<<(ostream& o,Goal a){
		switch(a){
			#define X(name) case name: return o<<""#name;
			X(GOAL_AUTO)
			X(GOAL_OFF)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Output a){
		#define X(name) if(a==name) return o<<""#name;
		X(OUTPUT_ON)
		X(OUTPUT_OFF)
		#undef X
		assert(0);
	}

	ostream& operator<<(ostream& o,Status a){
		#define X(name) if(a==name) return o<<""#name;
		X(FULL)
		X(NOT_FULL)
		#undef X
		assert(0);
	}

	vector<Pump::Status> status_list(){
		vector<Pump::Status> r;
		r|=Pump::FULL;
		r|=Pump::NOT_FULL;
		return r;
	}

	Maybe<Status> parse_status(string const& s){
		return parse_enum(status_list(),s);
	}

	Output control(Status s,Goal g){
		if(g==GOAL_OFF) return OUTPUT_OFF;
		return (s==FULL)?OUTPUT_OFF:OUTPUT_ON;
	}
}

#ifdef PUMP_TEST
int main(){
	static const vector<Pump::Goal> GOALS{Pump::GOAL_AUTO,Pump::GOAL_OFF};
	for(auto a:Pump::status_list()){
		for(auto g:GOALS){
			cout<<a<<","<<g<<":"<<control(a,g)<<"\n";
		}
		assert(a==Pump::parse_status(as_string(a)));
	}
}
#endif
