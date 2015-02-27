#include "toplevel.h"
#include<iostream>
#include<cassert>
#include<math.h>
#include "../util/util.h"

using namespace std;

namespace Toplevel{
	Output::Output():
		pump(Pump::Output::AUTO)
	{}

	ostream& operator<<(ostream& o,Output g){
		o<<"Output(";
		//o<<" shoot:"<<g.shooter_wheels;
		o<<" pump:"<<g.pump;
		return o<<")";
	}

	Subgoals::Subgoals():
		//shooter_wheels(Shooter_wheels:)
		lift_can(Lift::Goal::stop()),
		lift_tote(Lift::Goal::stop()),
		pump(Pump::Goal::AUTO)
	{}

	ostream& operator<<(ostream& o,Subgoals g){
		o<<"Toplevel::Subgoals(";
		//o<<g.shooter_wheels.second;
		//o<<g.shooter_wheels; not sure why this line doesn't work.
		o<<" pump:"<<g.pump;
		return o<<")";
	}

	Status::Status():
		lift_can(Lift::Status::error()),
		lift_tote(Lift::Status::error()),
		pump(Pump::Status::NOT_FULL)
	{}

	bool operator==(Status a,Status b){
		#define X(name) if(a.name!=b.name) return 0;
		X(pump)
		#undef X
		return 1;
	}

	bool operator!=(Status a,Status b){
		return !(a==b);
	}

	ostream& operator<<(ostream& o,Status s){
		o<<"Status(";
		o<<" pump:"<<s.pump;
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
			Maybe<Pump::Status> m;//=parse_status(X(5));
			if(!m) return Maybe<Status>();
			r.pump=*m;
		}
		#undef X
		return Maybe<Status>(r);
	}

	Estimator::Estimator():pump(Pump::Status::NOT_FULL){}

	void Estimator::update(Time /*time*/,bool /*enabled*/,Output /*out*/,Pump::Status pump_status, bool){
		pump=pump_status;
	}

	Status Estimator::estimate()const{
		Status r;
		r.pump=pump;
		return r;
	}

	void Estimator::out(ostream& o)const{
		o<<"Estimator(";
		o<<" pump:"<<pump;
		o<<")";
	}

	bool operator==(Estimator a,Estimator b){
		#define X(name) if(a.name!=b.name) return 0;
		X(pump)
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
		r.lift_can=control(status.lift_can,g.lift_can);
		r.lift_tote=control(status.lift_tote,g.lift_tote);
		r.drive=control(status.drive,g.drive);
		r.pump=control(status.pump,g.pump);
		return r;
	}

	bool ready(Status /*status*/,Subgoals /*g*/){
		return 1;//ready(status.shooter_wheels,g.shooter_wheels);
	}
	
	vector<string> not_ready(Status /*status*/,Subgoals /*g*/){
		vector<string> r;
		#define X(name) if(!ready(status.name,g.name)) r|=as_string(""#name);
		//X(shooter_wheels)
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
bool approx_equal(float a, float b){
	return a==b;
}

bool approx_equal(Toplevel::Status /*a*/,Toplevel::Status /*b*/){
	#define X(name) if(a.name!=b.name) return 0;
	//X(shooter_wheels)
	#undef X
	return 1;//approx_equal(a.orientation,b.orientation);
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
	Pump::Status ps=Pump::Status::FULL;
	est.update(0,1,Output(),ps,0);
	est.update(10,0,Output(),ps,0);
	cout<<est.estimate()<<"\n";
	/*
	if we choose one of the modes and use all the built-in controls then we should after some time get to a status where we're ready.  
	*/
	//assert(approx_equal(status,parse_status(as_string(status))));
}
#endif
