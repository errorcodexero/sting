#include "collector_tilt.h"
#include<iostream>
#include<cassert>
#include "../util/util.h"

//this is very similar to the injector arms.

using namespace std;

namespace Collector_tilt{
	ostream& operator<<(ostream& o,Goal g){
		switch(g){
			#define X(name) case GOAL_##name: return o<<""#name;
			X(UP)
			X(DOWN)
			#undef X
			default: assert(0);
		}
	}
	
	ostream& operator<<(ostream& o,Output a){
		switch(a){
			#define X(name) case OUTPUT_##name: return o<<""#name;
			X(UP)
			X(DOWN)
			X(NEITHER)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Status s){
		switch(s){
			#define X(name) case STATUS_##name: return o<<""#name;
			X(UP)
			X(DOWN)
			X(RAISING)
			X(LOWERING)
			X(UNKNOWN)
			#undef X
			default: assert(0);
		}
	}

	vector<Status> status_list(){
		vector<Status> r;
		r|=STATUS_UP;
		r|=STATUS_DOWN;
		r|=STATUS_RAISING;
		r|=STATUS_LOWERING;
		r|=STATUS_UNKNOWN;
		return r;
	}

	Maybe<Status> parse_status(std::string const& s){
		return parse_enum(status_list(),s);
	}

	Estimator::Estimator():est(STATUS_UP){}

	void Estimator::update(Time time,Output output){
		switch(est){
			case STATUS_UP:
				switch(output){
					case OUTPUT_DOWN:
						est=STATUS_LOWERING;
						timer.update(time,1);
						break;
					case OUTPUT_NEITHER:
						est=STATUS_UNKNOWN;
						break;
					default: break;
				}
				break;
			case STATUS_DOWN:
				if(output==OUTPUT_UP){
					est=STATUS_RAISING;
					timer.update(time,1);
				}
				break;
			case STATUS_RAISING:
				if(output==OUTPUT_UP){
					timer.update(time,0);
					static const Time RISE_TIME=1.695;//Timed and recorded (The longest timing in the recordings; average was 1.595)
					if(timer.elapsed()>RISE_TIME){
						est=STATUS_UP;
					}
				}else{
					//Could make the estimate more aggressive by having it go to lowering or unknown depending on the output.
					timer.update(time,0);
					est=STATUS_UNKNOWN;
				}
				break;
			case STATUS_LOWERING:
				switch(output){
					case OUTPUT_DOWN:
						timer.update(time,0);
						static const Time LOWER_TIME=0.82;//Timed and recorded (The longest timing in the recordings; average was 0.748)
						if(timer.elapsed()>LOWER_TIME){
							est=STATUS_DOWN;
						}
						break;
					case OUTPUT_UP:
						timer.update(time,0);
						est=STATUS_RAISING;
						break;
					case OUTPUT_NEITHER:
						//could make this a little less conservative by assuming that if you're most of the way down you'll continue to go down
						est=STATUS_UNKNOWN;
						break;
					default: assert(0);
				}
				break;
			case STATUS_UNKNOWN:
				switch(output){
					case OUTPUT_DOWN:
						timer.update(time,1);
						est=STATUS_LOWERING;
						break;
					case OUTPUT_UP:
						timer.update(time,1);
						est=STATUS_RAISING;
						break;
					case OUTPUT_NEITHER:
						break;
					default: assert(0);
				}
				break;
			default: assert(0);
		}
	}

	Status Estimator::estimate()const{ return est; }

	void Estimator::out(ostream& o)const{
		o<<"Estimator("<<est<<","<<timer<<")";
	}

	bool operator==(Estimator a,Estimator b){
		return a.est==b.est && a.timer==b.timer;
	}

	bool operator!=(Estimator a,Estimator b){
		return !(a==b);
	}

	ostream& operator<<(ostream& o,Estimator e){
		e.out(o);
		return o;
	}

	Output control(Goal g){
		switch(g){
			case GOAL_UP: return OUTPUT_UP;
			case GOAL_DOWN: return OUTPUT_DOWN;
			default: assert(0);
		}
	}

	bool ready(Status status,Goal goal){
		switch(goal){
			case GOAL_UP: return status==STATUS_UP;
			case GOAL_DOWN: return status==STATUS_DOWN;
			default: assert(0);
		}
	}
}

#ifdef COLLECTOR_TILT_TEST
#include<vector>
int main(){
	using namespace Collector_tilt;
	Estimator e;
	assert(e.estimate()==STATUS_UP);
	cout<<e<<"\n";
	e.update(.4,OUTPUT_DOWN);
	assert(e.estimate()==STATUS_LOWERING);
	e.update(10,OUTPUT_DOWN);
	assert(e.estimate()==STATUS_DOWN);
	//if wanted to do this well would try all the other transitions as well.

	static const vector<Goal> GOALS{GOAL_UP,GOAL_DOWN};
	for(auto goal:GOALS){
		cout<<goal<<":"<<control(goal)<<"\n";
	}
	cout<<"\n";
	for(auto status:status_list()){
		for(auto goal:GOALS){
			cout<<status<<" "<<goal<<" "<<ready(status,goal)<<"\n";
		}
		assert(status==parse_status(as_string(status)));
	}

	static const vector<Output> OUTPUTS{OUTPUT_UP,OUTPUT_DOWN,OUTPUT_NEITHER};
	for(auto output:OUTPUTS){
		cout<<output<<"\n";
		e.update(3,output);
	}
}
#endif
