#include "injector.h"
#include<iostream>
#include<cassert>
#include "../util/util.h"

using namespace std;

namespace Injector{
	ostream& operator<<(ostream& o,Status s){
		switch(s){
			#define X(name) case name: return o<<""#name;
			X(IDLE)
			X(SHOOTING)
			X(RECOVERY)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Output a){
		switch(a){
			#define X(name) case OUTPUT_##name: return o<<""#name;
			X(UP)
			X(DOWN)
			X(VENT)
			#undef X
			default: assert(0);
		}
	}

	ostream& operator<<(ostream& o,Goal g){
		switch(g){
			case START: return o<<"START";
			case WAIT: return o<<"WAIT";
			case X: return o<<"X";
			default: assert(0);
		}
	}

	Estimator::Estimator():location(GOING_DOWN){} //might want to change this to assume that it starts down.

	pair<Estimator::Location,bool> next(Estimator::Location loc,Time elapsed,Output out,bool downsensor){
		switch(loc){
			case Estimator::GOING_UP:
				if(out==OUTPUT_DOWN){
					return make_pair(Estimator::GOING_DOWN,0);
				}
				if(out==OUTPUT_UP){
					static const Time RISE_TIME=0.5;
					//Chosen based on testing by regulating down the launch 
					//Normal operation should never be as bad as ~0.4188 seconds
					if(elapsed>RISE_TIME){
						return make_pair(Estimator::UP,0);
					}
					return make_pair(Estimator::GOING_UP,0);
				}
				return make_pair(Estimator::X,0);
			case Estimator::UP:
				switch(out){
					case OUTPUT_UP: return make_pair(Estimator::UP,0);
					case OUTPUT_DOWN: return make_pair(Estimator::GOING_DOWN,0);
					case OUTPUT_VENT: return make_pair(Estimator::X,0);
					default: assert(0);
				}
			case Estimator::GOING_DOWN:
				if(out==OUTPUT_DOWN){
					static const Time MIN_LOWER_TIME=.4288;
					static const Time MAX_LOWER_TIME=2; 
					//Chosen based on testing by regulating down the return 
					//Normal operation should never be as bad as ~0.4288 seconds
					if(elapsed>MAX_LOWER_TIME||(elapsed>MIN_LOWER_TIME&&downsensor)){
						return make_pair(Estimator::DOWN_VENT,0);
					}
					return make_pair(Estimator::GOING_DOWN,0);
				}
				if(out==OUTPUT_UP){
					return make_pair(Estimator::GOING_UP,0);
				}
				//vent
				return make_pair(Estimator::X,0);
			case Estimator::DOWN_VENT:
				switch(out){
					case OUTPUT_UP: return make_pair(Estimator::GOING_UP,0);
					case OUTPUT_DOWN: 
						return make_pair(Estimator::DOWN_VENT,1);
						//return make_pair(Estimator::GOING_DOWN,0);
					case OUTPUT_VENT:
						static const Time VENT_TIME=0.5;//Probably about right now, as it is about the same time as the rest of the cycle
						if(elapsed>VENT_TIME){
							return make_pair(Estimator::DOWN_IDLE,0);
						}
						return make_pair(Estimator::DOWN_VENT,0);
					default: assert(0);//should get test here
				}
			case Estimator::DOWN_IDLE:
				switch(out){
					case OUTPUT_UP: return make_pair(Estimator::GOING_UP,0);
					case OUTPUT_DOWN: return make_pair(Estimator::DOWN_VENT,0);
					case OUTPUT_VENT: return make_pair(Estimator::DOWN_IDLE,0);
					default: assert(0);
				}
				break;
			case Estimator::X:
				switch(out){
					case OUTPUT_UP: return make_pair(Estimator::GOING_UP,0);
					case OUTPUT_DOWN: return make_pair(Estimator::GOING_DOWN,0);
					case OUTPUT_VENT: return make_pair(Estimator::X,0);
					default: assert(0);
				}
			default:
				assert(0);
		}
	}

	void Estimator::update(Time time,Output out,bool downsensor){
		timer.update(time,0);
		pair<Location,bool> n=next(location,timer.elapsed(),out,downsensor);
		if(n.second || n.first!=location) timer.update(time,1);
		location=n.first;
	}

	Status location_to_status(Estimator::Location location){
		switch(location){
			case Estimator::GOING_UP:
			case Estimator::UP:
				return SHOOTING;
			case Estimator::GOING_DOWN:
			case Estimator::DOWN_VENT:
				return RECOVERY;
			case Estimator::DOWN_IDLE:
				return IDLE;
			case Estimator::X:
				return RECOVERY;
			default: assert(0);
		}
	}

	Estimator::Location Estimator::estimate()const{ return location; }

	//I'm not sure that this function should be part of the class.
	Status Estimator::status()const{
		return location_to_status(location);
	}

	void Estimator::out(ostream& o)const{
		o<<"Estimator("<<location<<" "<<timer<<")";
	}

	ostream& operator<<(ostream& o,Estimator::Location a){
		switch(a){
			#define X1(name) case Estimator::name: return o<<""#name;
			X1(GOING_UP)
			X1(UP)
			X1(GOING_DOWN)
			X1(DOWN_VENT)
			X1(DOWN_IDLE)
			X1(X)
			#undef X1
			default: assert(0);
		}
	}

	vector<Estimator::Location> locations(){
		vector<Estimator::Location> r;
		r|=Estimator::GOING_UP;
		r|=Estimator::UP;
		r|=Estimator::GOING_DOWN;
		r|=Estimator::DOWN_VENT;
		r|=Estimator::DOWN_IDLE;
		r|=Estimator::X;
		return r;
	}

	Maybe<Estimator::Location> parse_location(string const& s){
		return parse_enum(locations(),s);
	}

	bool operator==(Estimator a,Estimator b){
		return a.location==b.location && a.timer==b.timer;
	}

	bool operator!=(Estimator a,Estimator b){
		return !(a==b);
	}

	ostream& operator<<(ostream& o,Estimator est){
		est.out(o);
		return o;
	}

	Output control(Estimator::Location loc,Goal g){
		switch(loc){
			case Estimator::GOING_UP:
				return OUTPUT_UP;
			case Estimator::UP:
			case Estimator::GOING_DOWN:
				return OUTPUT_DOWN;
			case Estimator::DOWN_VENT:
				return OUTPUT_VENT;
			case Estimator::DOWN_IDLE:
				return (g==START)?OUTPUT_UP:OUTPUT_VENT;
			case Estimator::X:
				return OUTPUT_DOWN;
			default: assert(0);
		}
	}

	bool ready(Estimator::Location loc,Goal goal){
		return ready(location_to_status(loc),goal);
	}

	bool ready(Status status,Goal goal){
		return goal==X || status==IDLE;
	}
}

#ifdef INJECTOR_TEST
#include<vector>

int main(){
	using namespace Injector;
	Estimator e;
	cout<<e<<"\n";
	assert(e.status()==RECOVERY);
	e.update(0,OUTPUT_UP,0);
	assert(e.status()==SHOOTING);
	//This doesn't actually est all transitions
	Status last=e.status();
	for(unsigned i=0;i<100;i++){
		Time now=i/10.0;
		Output out=control(e.estimate(),START);
		e.update(now,out,0);
		if(e.status()!=last || 1){
			cout<<"location="<<e.estimate()<<" ";
			cout<<"status="<<e.status()<<"\n";
			last=e.status();
		}
	}

	static const vector<Status> STATUS_LIST{IDLE,SHOOTING,RECOVERY};
	static const vector<Goal> GOALS{START,WAIT,X};
	cout<<"Control outputs:\n";
	for(auto location:locations()){
		for(auto goal:GOALS){
			cout<<location<<"\t"<<goal<<"\t"<<control(location,goal)<<"\n";
		}

		assert(location==parse_location(as_string(location)));
	}
	cout<<"Ready states:\n";
	for(auto status:STATUS_LIST){
		for(auto goal:GOALS){
			cout<<status<<"\t"<<goal<<"\t"<<ready(status,goal)<<"\n";
		}
	}
	cout<<"State transitions:\n";
	static const vector<Output> OUTPUTS{OUTPUT_UP,OUTPUT_DOWN,OUTPUT_VENT};
	for(auto location:locations()){
		for(auto out:OUTPUTS){
			for(float elapsed:{0,3}){
				cout<<location<<" "<<out<<" "<<elapsed<<" "<<next(location,elapsed,out,0)<<"\n";
			}
		}
	}
}
#endif
