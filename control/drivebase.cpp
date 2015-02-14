#include "drivebase.h"
#include<iostream>
#include<math.h>
#include "../util/util.h"
#include "formal.h"

using namespace std;

set<Drivebase::Status> examples(Drivebase::Status*){ return {Drivebase::Status{}}; }

bool operator<(Drivebase::Status,Drivebase::Status){ return 0; }
bool operator==(Drivebase::Status,Drivebase::Status){ return 1; }

ostream& operator<<(ostream& o,Drivebase::Status const&){
	o<<"Drivebase::Status(";
	return o<<")";
}

set<Drivebase::Goal> examples(Drivebase::Goal*){
	return {
		Drivebase::Goal{0,0,0,0,0},
		Drivebase::Goal{0,1,0,0,0}
	};
}

ostream& operator<<(ostream& o,Drivebase::Goal const& a){
	o<<"Drivebase::Goal(";
	o<<a.x<<" "<<a.y<<" "<<a.theta;
	return o<<")";
}

#define CMP(name) if(a.name<b.name) return 1; if(b.name<a.name) return 0;

bool operator<(Drivebase::Goal const& a,Drivebase::Goal const& b){
	CMP(x)
	CMP(y)
	CMP(theta)
	return 0;
}

Drivebase::Output::Output():l(0),r(0),c(0){}
Drivebase::Output::Output(double a,double b,double c1):l(a),r(b),c(c1){}

set<Drivebase::Output> examples(Drivebase::Output*){
	return {
		Drivebase::Output{},
		Drivebase::Output{1,1,0}
	};
}

ostream& operator<<(ostream& o,Drivebase::Output const& a){
	o<<"Drivebase::Output(";
	o<<a.l<<" "<<a.r<<" "<<a.c;
	return o<<")";
}

bool operator<(Drivebase::Output const& a,Drivebase::Output const& b){
	CMP(l) CMP(r) CMP(c)
	return 0;
}

bool operator!=(Drivebase::Output const& a,Drivebase::Output const& b){
	return !(a==b);
}

bool operator==(Drivebase::Output const& a,Drivebase::Output const& b){
	return a.l==b.l && a.r==b.r && a.c==b.c;
}

set<Drivebase::Input> examples(Drivebase::Input*){
	return {Drivebase::Input{}};
}

bool operator<(Drivebase::Input const&,Drivebase::Input const&){
	NYI
}

ostream& operator<<(ostream& o,Drivebase::Input const&){
	o<<"Drivebase::Input(";
	return o<<")";
}

Drivebase::Status_detail Drivebase::Estimator::get()const{ return Status{}; }

ostream& operator<<(ostream& o,Drivebase::Output_applicator){
	return o<<"output_applicator";
}

ostream& operator<<(ostream& o,Drivebase const&){
	o<<"Drivebase(";
	return o<<")";
}

void Drivebase::Estimator::update(double,Drivebase::Input,Drivebase::Output){}

Robot_outputs Drivebase::Output_applicator::operator()(Robot_outputs robot,Drivebase::Output b)const{
	robot.pwm[0]=-pwm_convert(b.l);
	robot.pwm[1]=pwm_convert(b.r);
	robot.pwm[2]=pwm_convert(b.c);
	return robot;
}

Drivebase::Output Drivebase::Output_applicator::operator()(Robot_outputs robot)const{
	return Drivebase::Output{
		-from_pwm(robot.pwm[0]),
		from_pwm(robot.pwm[1]),
		from_pwm(robot.pwm[2])
	};
}

Drivebase::Output control(Drivebase::Status,Drivebase::Goal goal){
	//use jacob's or matthew's code
	double l=goal.y+goal.theta;
	double r=goal.y-goal.theta;
	auto m=max(1.0,max(fabs(l),fabs(r)));
	auto ret=Drivebase::Output{l/m,r/m,goal.x};
	cout<<"r:"<<ret<<"\n";
	return ret;
}

Drivebase::Status status(Drivebase::Status a){ return a; }

bool ready(Drivebase::Status,Drivebase::Goal){ return 1; }

#ifdef DRIVEBASE_TEST
int main(){
	Drivebase d;
	cout<<d.output_applicator<<"\n";
	tester(d);
}
#endif
