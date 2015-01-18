#include "injector_arms.h"
#include<iostream>
#include<cassert>
#include "../util/util.h"

using namespace std;

namespace Injector_arms{
	ostream& operator<<(ostream& o,Goal g){
		switch(g){
			#define X(name) case name: return o<<""#name;
			X(GOAL_OPEN)
			X(GOAL_CLOSE)
			X(GOAL_X)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Output a){
		switch(a){
			#define X(name) case name: return o<<""#name;
			X(OUTPUT_OPEN)
			X(OUTPUT_CLOSE)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Status s){
		switch(s){
			#define X(name) case name: return o<<""#name;
			X(STATUS_OPEN)
			X(STATUS_CLOSED)
			X(STATUS_OPENING)
			X(STATUS_CLOSING)
			#undef X
			default: assert(false);
		}
	}

	vector<Status> status_options(){
		vector<Status> r;
		r|=STATUS_OPEN;
		r|=STATUS_CLOSED;
		r|=STATUS_OPENING;
		r|=STATUS_CLOSING;
		return r;
	}

	Maybe<Status> parse_status(string const& s){
		return parse_enum(status_options(),s);
	}

	Estimator::Estimator():est(STATUS_CLOSING){} //may want to have it start thinking it's closed

	void Estimator::update(Time t,Output out){
		switch(est){
			case STATUS_OPEN:
				if(out!=OUTPUT_OPEN){
					est=STATUS_CLOSING;
					timer.update(t,1);
				}
				break;
			case STATUS_CLOSED:
				if(out!=OUTPUT_CLOSE){
					est=STATUS_OPENING;
					timer.update(t,1);
				}
				break;
			case STATUS_OPENING:
				if(out==OUTPUT_OPEN){
					timer.update(t,0);
					//This is actually even faster than opening; rounding up to .1 should give plenty of margin.
					static const Time OPEN_TIME=.1;
					if(timer.elapsed()>OPEN_TIME){
						est=STATUS_OPEN;
					}
				}else{
					est=STATUS_CLOSING;
					timer.update(t,1);
				}
				break;
			case STATUS_CLOSING:
				if(out==OUTPUT_CLOSE){
					timer.update(t,0);
					static const Time CLOSE_TIME=0.1; //Is most nearly 0.0738 (31/420), rounding to 0.1 seconds
					if(timer.elapsed()>CLOSE_TIME){
						est=STATUS_CLOSED;
					}
				}else{
					est=STATUS_OPENING;
					timer.update(t,1);
				}
				break;
			default:
				assert(0);
		}
	}

	Status Estimator::estimate()const{
		return est;
	}

	bool operator!=(Estimator,Estimator){
		assert(0);
	}

	void Estimator::out(ostream& o)const{
		o<<"Estimator(";
		o<<est<<" "<<timer;
		o<<")";
	}

	ostream& operator<<(ostream& o,Estimator e){
		e.out(o);
		return o;
	}

	Output control(Status status,Goal goal){
		switch(goal){
			case GOAL_OPEN: return OUTPUT_OPEN;
			case GOAL_CLOSE: return OUTPUT_CLOSE;	
			case GOAL_X: return (status==STATUS_OPEN || status==STATUS_OPENING)?OUTPUT_OPEN:OUTPUT_CLOSE;
			default: assert(0);
		}
	}

	bool ready(Status status,Goal goal){
		switch(goal){
			case GOAL_OPEN: return status==STATUS_OPEN;
			case GOAL_CLOSE: return status==STATUS_CLOSED;
			case GOAL_X: return 1;
			default: assert(0);
		}
	}
}

#ifdef INJECTOR_ARMS_TEST
#include<vector>

int main(){
	Injector_arms::Estimator e;
	assert(e.estimate()==Injector_arms::STATUS_CLOSING);
	e.update(1,Injector_arms::OUTPUT_OPEN);
	assert(e.estimate()==Injector_arms::STATUS_OPENING);
	e.update(4,Injector_arms::OUTPUT_OPEN);
	assert(e.estimate()==Injector_arms::STATUS_OPEN);
	cout<<e<<"\n";
	e.update(5,Injector_arms::OUTPUT_CLOSE);
	e.update(8,Injector_arms::OUTPUT_CLOSE);
	assert(e.estimate()==Injector_arms::STATUS_CLOSED);

	using namespace Injector_arms;
	static const vector<Goal> GOALS{GOAL_OPEN,GOAL_CLOSE,GOAL_X};
	for(auto status:status_options()){
		for(auto goal:GOALS){
			cout<<status<<"\t"<<goal<<"\t"<<control(status,goal)<<"\t"<<ready(status,goal)<<"\n";
		}

		assert(status==parse_status(as_string(status)));
	}
}
#endif
