#include "toplevel.h"
#include<iostream>
#include<cassert>
#include<math.h>
#include "../util/util.h"

using namespace std;

Toplevel::Output::Output():
	pump(Pump::Output::AUTO)
{}

ostream& operator<<(ostream& o,Toplevel::Output g){
	o<<"Output(";
	#define X(A,B,C) o<<" "#B<<":"<<g.B;
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

ostream& operator<<(ostream& o,Toplevel::Input const& a){
	o<<"Toplevel::Input(";
	#define X(A,B,C) o<<" "#B<<":"<<a.B;
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

Toplevel::Goal::Goal():
	//shooter_wheels(Shooter_wheels:)
	#define X(A,B,C) B(C),
	TOPLEVEL_ITEMS
	#undef X
	dummy()
	/*lift_can(Lift::Goal::stop()),
	lift_tote(Lift::Goal::stop()),
	kicker(Kicker::Goal::IN),
	pump(Pump::Goal::AUTO)*/
{}

ostream& operator<<(ostream& o,Toplevel::Goal g){
	o<<"Toplevel::Goal(";
	#define X(A,B,C) o<<" "#B<<":"<<g.B;
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

Toplevel::Status::Status():
	lift_can(Lift::Status::error()),
	lift_tote(Lift::Status::error()),
	kicker(Kicker::Status::IN),
	pump(Pump::Status::NOT_FULL),
	can_grabber(Can_grabber::Status::MID_UP)
{}

bool operator==(Toplevel::Status a,Toplevel::Status b){
	#define X(A,name,C) if(a.name!=b.name) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 1;
}

bool operator!=(Toplevel::Status a,Toplevel::Status b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Toplevel::Status s){
	o<<"Status(";
	#define X(A,B,C) o<<" "#B<<":"<<s.B;
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

string remove_till_colon(string s){
	unsigned i=0;
	while(i<s.size() && s[i]!=':') i++;
	if(s[i]==':') i++;
	string r=s.substr(i,s.size());
	return r;
}

/*Maybe<Toplevel::Status> parse_status(string const& s){
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
}*/

Toplevel::Estimator::Estimator():pump(Pump::Status::NOT_FULL){}

void Toplevel::Estimator::update(Time /*time*/,bool /*enabled*/,Output /*out*/,Pump::Status pump_status, bool){
	pump=pump_status;
}

Toplevel::Status Toplevel::Estimator::estimate()const{
	Status r;
	r.pump=pump;
	return r;
}

void Toplevel::Estimator::out(ostream& o)const{
	o<<"Estimator(";
	o<<" pump:"<<pump;
	o<<")";
}

bool operator==(Toplevel::Estimator a,Toplevel::Estimator b){
	#define X(name) if(a.name!=b.name) return 0;
	X(pump)
	#undef X
	return 1;
}

bool operator!=(Toplevel::Estimator a,Toplevel::Estimator b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Toplevel::Estimator e){
	e.out(o);
	return o;
}

bool approx_equal(Toplevel::Estimator a,Toplevel::Estimator b){
	return a.estimate()==b.estimate();
}

Toplevel::Output control(Toplevel::Status status,Toplevel::Goal g){
	Toplevel::Output r;
	#define X(A,B,C) r.B=control(status.B,g.B);
	TOPLEVEL_ITEMS
	#undef X
	return r;
}

bool ready(Toplevel::Status status,Toplevel::Goal g){
	#define X(A,B,C) if(!ready(status.B,g.B)) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 1;
}

vector<string> not_ready(Toplevel::Status status,Toplevel::Goal g){
	vector<string> r;
	#define X(A,name,C) if(!ready(status.name,g.name)) r|=as_string(""#name);
	TOPLEVEL_ITEMS
	#undef X
	return r;
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
	Toplevel::Goal g;
	cout<<g<<"\n";
	Toplevel::Status status;
	cout<<status<<"\n";
	//Toplevel::Control control;
	Toplevel::Estimator est;
	cout<<est<<"\n";
	cout<<est.estimate()<<"\n";
	Pump::Status ps=Pump::Status::FULL;
	est.update(0,1,Toplevel::Output(),ps,0);
	est.update(10,0,Toplevel::Output(),ps,0);
	cout<<est.estimate()<<"\n";
	/*
	if we choose one of the modes and use all the built-in controls then we should after some time get to a status where we're ready.  
	*/
	//assert(approx_equal(status,parse_status(as_string(status))));
}
#endif
