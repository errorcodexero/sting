#include "toplevel.h"
#include<iostream>
#include<cassert>
#include<math.h>
#include "../util/util.h"

using namespace std;

namespace Toplevel{
	Output::Output():
		collector(Collector_mode::OFF),
		collector_tilt(Collector_tilt::OUTPUT_UP),
		injector(Injector::OUTPUT_DOWN),
		injector_arms(Injector_arms::OUTPUT_CLOSE),
		ejector(Ejector::OUTPUT_DOWN),
		pump(Pump::OUTPUT_ON)
	{}

	ostream& operator<<(ostream& o,Output g){
		o<<"Output(";
		o<<"collect:"<<g.collector;
		o<<" colct_tlt:"<<g.collector_tilt;
		o<<" inject:"<<g.injector;
		o<<" inj arm:"<<g.injector_arms;
		o<<" eject:"<<g.ejector;
		o<<" shoot:"<<g.shooter_wheels;
		o<<" pump:"<<g.pump;
		o<<" drive:"<<g.drive;
		return o<<")";
	}

	Subgoals::Subgoals():
		collector(Collector_mode::OFF),
		collector_tilt(Collector_tilt::GOAL_UP),
		injector(Injector::WAIT),
		injector_arms(Injector_arms::GOAL_X),
		ejector(Ejector::WAIT),
		//shooter_wheels(Shooter_wheels:)
		pump(Pump::GOAL_AUTO)
	{}

	ostream& operator<<(ostream& o,Subgoals g){
		o<<"Toplevel::Subgoals(";
		o<<"collect:"<<g.collector;
		o<<" colct_tlt:"<<g.collector_tilt;
		o<<" inject:"<<g.injector;
		o<<" inj_arm:"<<g.injector_arms;
		o<<" eject:"<<g.ejector;
		o<<" shoot:";
		o<<g.shooter_wheels;
		//o<<g.shooter_wheels.second;
		//o<<g.shooter_wheels; not sure why this line doesn't work.
		o<<" pump:"<<g.pump;
		o<<" drive:"<<g.drive;
		return o<<")";
	}

	Status::Status():
		collector_tilt(Collector_tilt::STATUS_LOWERING),
		injector(Injector::Estimator::GOING_DOWN),
		injector_arms(Injector_arms::STATUS_CLOSING),
		ejector(Ejector::Estimator::GOING_DOWN),
		pump(Pump::NOT_FULL)
	{}

	bool operator==(Status a,Status b){
		#define X(name) if(a.name!=b.name) return 0;
		X(collector_tilt)
		X(injector)
		X(injector_arms)
		X(ejector)
		X(shooter_wheels)
		X(pump)
		X(orientation)
		#undef X
		return 1;
	}

	bool operator!=(Status a,Status b){
		return !(a==b);
	}

	ostream& operator<<(ostream& o,Status s){
		o<<"Status(";
		o<<"colct_tlt:"<<s.collector_tilt;
		o<<" inject:"<<s.injector;
		o<<" inj_arm:"<<s.injector_arms;
		o<<" eject:"<<s.ejector;
		o<<" shoot:"<<s.shooter_wheels;
		o<<" pump:"<<s.pump;
		o<<" orientation:"<<s.orientation;
		return o<<")";
	}

	string remove_till_colon(string s){
		unsigned i=0;
		while(i<s.size() && s[i]!=':') i++;
		if(s[i]==':') i++;
		string r=s.substr(i,s.size());
		return r;
	}

	Maybe<Status> parse_status(string const& s){
		vector<string> v=split(inside_parens(s));
		if(v.size()!=7) return Maybe<Status>();
		Status r;
		//yes, there is a better way to do this; it's called a monad. (or exceptions)
		#define X(i) remove_till_colon(v[i])
		{
			Maybe<Collector_tilt::Status> m=Collector_tilt::parse_status(X(0));
			if(!m) return Maybe<Status>();
			r.collector_tilt=*m;
		}
		{
			Maybe<Injector::Estimator::Location> m=Injector::parse_location(X(1));
			if(!m) return Maybe<Status>();
			r.injector=*m;
		}
		{
			Maybe<Injector_arms::Status> m=Injector_arms::parse_status(X(2));
			if(!m) return Maybe<Status>();
			r.injector_arms=*m;
		}
		{
			Maybe<Ejector::Estimator::Location> m=Ejector::parse_location(X(3));
			if(!m) return Maybe<Status>();
			r.ejector=*m;
		}
		{
			cout<<X(4)<<"\n";
			Maybe<Shooter_wheels::Status> m=Shooter_wheels::parse_status(X(4));
			if(!m) return Maybe<Status>();
			m=*Shooter_wheels::parse_status(v[4]);
		}
		{
			Maybe<Pump::Status> m=Pump::parse_status(X(5));
			if(!m) return Maybe<Status>();
			r.pump=*m;
		}
		r.orientation=atof(X(6));
		#undef X
		return Maybe<Status>(r);
	}

	Estimator::Estimator():pump(Pump::NOT_FULL), orientation(0){}

	void Estimator::update(Time time,bool enabled,Output out,Pump::Status pump_status, float orientation1,Shooter_wheels::Status wheels_in,bool downsensor){
		collector_tilt.update(time,enabled?out.collector_tilt:Collector_tilt::OUTPUT_NEITHER);
		injector.update(time,enabled?out.injector:Injector::OUTPUT_VENT,downsensor);
		injector_arms.update(time,enabled?out.injector_arms:Injector_arms::OUTPUT_OPEN);
		ejector.update(time,enabled?out.ejector:Ejector::OUTPUT_DOWN);
		shooter_wheels=wheels_in;
		pump=pump_status;
		orientation = orientation1;
	}

	Status Estimator::estimate()const{
		Status r;
		r.collector_tilt=collector_tilt.estimate();
		r.injector=injector.estimate();
		r.injector_arms=injector_arms.estimate();
		r.ejector=ejector.estimate();
		r.pump=pump;
		r.shooter_wheels = shooter_wheels;
		return r;
	}

	void Estimator::out(ostream& o)const{
		o<<"Estimator(";
		o<<"colct_tlt:"<<collector_tilt;
		o<<" inject:"<<injector;
		o<<" inj arm:"<<injector_arms;
		o<<" eject:"<<ejector;
		o<<" shooter_wheels:"<<shooter_wheels;
		o<<" pump:"<<pump;
		o<<")";
	}

	bool operator==(Estimator a,Estimator b){
		#define X(name) if(a.name!=b.name) return 0;
		X(collector_tilt)
		X(injector)
		X(injector_arms)
		X(ejector)
		X(shooter_wheels)
		X(pump)
		X(orientation)
		#undef X
		return 1;
	}

	bool operator!=(Estimator a,Estimator b){
		return !(a==b);
	}

	ostream& operator<<(ostream& o,Estimator e){
		e.out(o);
		return o;
	}

	bool approx_equal(Estimator a,Estimator b){
		return a.estimate()==b.estimate();
	}

	Output control(Status status,Subgoals g){
		Output r;
		r.collector=g.collector;
		r.collector_tilt=Collector_tilt::control(g.collector_tilt);
		r.injector=Injector::control(status.injector,g.injector);
		r.injector_arms=Injector_arms::control(status.injector_arms,g.injector_arms);
		r.ejector=Ejector::control(status.ejector,g.ejector);
		r.shooter_wheels=control(status.shooter_wheels,g.shooter_wheels);
		r.pump=Pump::control(status.pump,g.pump);
		r.drive=::control(g.drive, status.orientation);
		return r;
	}

	bool ready(Status status,Subgoals g){
		return Collector_tilt::ready(status.collector_tilt,g.collector_tilt) && 
			Injector::ready(status.injector,g.injector) && 
			Injector_arms::ready(status.injector_arms,g.injector_arms) && 
			Ejector::ready(status.ejector,g.ejector) && 
			ready(status.shooter_wheels,g.shooter_wheels);
	}
	
	vector<string> not_ready(Status status,Subgoals g){
		vector<string> r;
		#define X(name) if(!ready(status.name,g.name)) r|=as_string(""#name);
		X(collector_tilt)
		X(injector)
		X(injector_arms)
		X(ejector)
		X(shooter_wheels)
		#undef X
		return r;
	}
	
	/*ostream& operator<<(ostream& o,Control a){
		o<<"Toplevel::Control(";
		o<<a.shooter_wheels;
		return o<<")";
	}*/

}

#ifdef TOPLEVEL_TEST
bool approx_equal(Toplevel::Status a,Toplevel::Status b){
	#define X(name) if(a.name!=b.name) return 0;
	X(collector_tilt)
	X(injector)
	X(injector_arms)
	X(ejector)
	X(shooter_wheels)
	#undef X
	return approx_equal(a.orientation,b.orientation);
}

template<typename T>
bool approx_equal(T t,Maybe<T> m){
	if(!m) return 0;
	return approx_equal(t,*m);
}

int main(){
	using namespace Toplevel;
	Toplevel::Subgoals g;
	cout<<g<<"\n";
	Toplevel::Status status;
	cout<<status<<"\n";
	//Toplevel::Control control;
	Estimator est;
	cout<<est<<"\n";
	cout<<est.estimate()<<"\n";
	Pump::Status ps=Pump::FULL;
	est.update(0,1,Output(),ps,0,Shooter_wheels::Status(),0);
	est.update(10,0,Output(),ps,0,Shooter_wheels::Status(),0);
	cout<<est.estimate()<<"\n";
	/*
	if we choose one of the modes and use all the built-in controls then we should after some time get to a status where we're ready.  
	*/
	assert(approx_equal(status,parse_status(as_string(status))));
}
#endif
