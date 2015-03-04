#include "toplevel.h"
#include<iostream>
#include<cassert>
#include<math.h>
#include "../util/util.h"

using namespace std;

Toplevel::Toplevel():
	kicker(0),
	can_grabber(4),estimator(this),
	output_applicator(this)
{}

bool operator==(Toplevel const& a,Toplevel const& b){
	#define X(A,B,C) if(a.B!=b.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 1;
}

bool operator!=(Toplevel const& a,Toplevel const& b){ return !(a==b); }

Toplevel::Status status(Toplevel::Status_detail const& a){
	/*return Toplevel::Status{
		#define X(A,B,C) status(a.B),
		TOPLEVEL_ITEMS
		#undef X
	};*/
	Toplevel::Status r;
	#define X(A,B,C) r.B=status(a.B);
	TOPLEVEL_ITEMS
	#undef X
	return r;
}

ostream& operator<<(ostream& o,Toplevel const& a){
	o<<"Toplevel(";
	#define X(A,B,C) o<<a.B<<" ";
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

Toplevel::Output_applicator::Output_applicator(Toplevel *p):parent(p){
	assert(p);
}

Robot_outputs Toplevel::Output_applicator::operator()(Robot_outputs r,Toplevel::Output const& a)const{
	#define X(A,B,C) r=parent->B.output_applicator(r,a.B);
	TOPLEVEL_ITEMS
	#undef X
	return r;
}

Toplevel::Output Toplevel::Output_applicator::operator()(Robot_outputs a)const{
	Toplevel::Output r;
	#define X(A,B,C) r.B=parent->B.output_applicator(a);
	TOPLEVEL_ITEMS
	#undef X
	return r;
}

Toplevel::Output::Output():
	drive({0,0,0}),
	pump(Pump::Output::AUTO)
{}

bool operator<(Toplevel::Output const& a,Toplevel::Output const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 0;
}

bool operator<(Toplevel::Status const& a,Toplevel::Status const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 0;
}

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
	kicker(Kicker::Status::IN),
	drive({Motor_check::Status::OK_,Motor_check::Status::OK_,Motor_check::Status::OK_}),
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

Toplevel::Estimator::Estimator(Toplevel *p):parent(p){
	assert(p);
}

void Toplevel::Estimator::update(Time time,Input in,Output out){
	#define X(A,B,C) parent->B.estimator.update(time,in.B,out.B);
	TOPLEVEL_ITEMS
	#undef X
}

Toplevel::Status_detail Toplevel::Estimator::get()const{
	return Status_detail{
		#define X(A,B,C) parent->B.estimator.get(),
		TOPLEVEL_ITEMS
		#undef X
	};
}

void Toplevel::Estimator::out(ostream& o)const{
	o<<"Estimator(";
	/*#define X(A,B,C) o<<parent->B.estimator
	TOPLEVEL_ITEMS
	#undef X*/
	o<<")";
}

bool operator==(Toplevel::Estimator a,Toplevel::Estimator b){
	#define X(A,B,C) if(a.parent->B.estimator!=b.parent->B.estimator) return 0;
	TOPLEVEL_ITEMS
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
	return a.get()==b.get();
}

bool operator==(Toplevel::Output const& a,Toplevel::Output const& b){
	#define X(A,B,C) if(a.B!=b.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 1;
}

bool operator<(Toplevel::Status_detail const& a,Toplevel::Status_detail const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 0;
}

ostream& operator<<(ostream& o,Toplevel::Status_detail const& a){
	o<<"Toplevel::Status_detail(";
	#define X(A,B,C) o<<a.B<<" ";
	TOPLEVEL_ITEMS
	#undef X
	return o<<")";
}

bool operator!=(Toplevel::Output const& a,Toplevel::Output const& b){ return !(a==b); }

bool operator==(Toplevel::Status_detail const& a,Toplevel::Status_detail const& b){
	#define X(A,B,C) if(a.B!=b.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 1;
}

Toplevel::Output control(Toplevel::Status_detail status,Toplevel::Goal g){
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

bool operator<(Toplevel::Input const& a,Toplevel::Input const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 0;
}

bool operator<(Toplevel::Goal const& a,Toplevel::Goal const& b){
	#define X(A,B,C) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	TOPLEVEL_ITEMS
	#undef X
	return 0;
}

set<Toplevel::Status_detail> examples(Toplevel::Status_detail*){
	return {Toplevel::Status_detail{
		{
			Lift::Status_detail::error(),
			Lift::Status_detail::error()
		},
		Kicker::Status_detail{},
		*examples((Drivebase::Status_detail*)0).begin(),
		Pump::Status_detail{},
		Can_grabber::Status_detail{}
	}};
}

set<Toplevel::Goal> examples(Toplevel::Goal*){
	Toplevel::Goal g;
	g.pump=Pump::Goal::OFF;
	return {Toplevel::Goal{},g};
}

set<Toplevel::Status> examples(Toplevel::Status*){ return {Toplevel::Status{}}; }

set<Toplevel::Input> examples(Toplevel::Input*){
	Toplevel::Input a{
		Combo_lift::Input(),
		Kicker::Input(),
		*examples((Drivebase::Input*)0).begin(),
		Pump::Input{},
		Can_grabber::Input{0}
	};
	return {a};
}

set<Toplevel::Output> examples(Toplevel::Output*){
	Toplevel::Output a;
	a.can_grabber=Can_grabber::Output::OFF;
	a.kicker=Kicker::Output::OUT;
	/*return {Toplevel::Output{
		Lift::Output{},
		Lift::Output{},
		Kicker::Output::STOP,
		Pump::Output::AUTO,
		Can_grabber::Output{}
	}};*/
	set<Toplevel::Output> r;
	r.insert(a);
	a.pump=Pump::Output::OFF;
	return r;
}

#ifdef TOPLEVEL_TEST
#include "formal.h"

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

	//tester(Toplevel{});
	//Toplevel::Control control;
	/*Toplevel::Estimator est;
	cout<<est<<"\n";
	cout<<est.estimate()<<"\n";
	Pump::Status ps=Pump::Status::FULL;
	est.update(0,Toplevel::Input(),Toplevel::Output());
	est.update(10,0,Toplevel::Output(),ps,0);
	cout<<est.estimate()<<"\n";*/
	/*
	if we choose one of the modes and use all the built-in controls then we should after some time get to a status where we're ready.  
	*/
	//assert(approx_equal(status,parse_status(as_string(status))));
}
#endif
