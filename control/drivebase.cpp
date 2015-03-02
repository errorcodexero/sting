#include "drivebase.h"
#include<iostream>
#include<math.h>
#include "../util/util.h"
#include "formal.h"

using namespace std;

Drivebase::Input::Input():current({0,0,0}){}
Drivebase::Input::Input(array<double,Drivebase::MOTORS> a):current(a){}

Drivebase::Status::Status(array<Motor_check::Status,Drivebase::MOTORS> a):motor(a){}

set<Drivebase::Status> examples(Drivebase::Status*){
	return {Drivebase::Status{array<Motor_check::Status,Drivebase::MOTORS>{
		Motor_check::Status::OK_,
		Motor_check::Status::OK_,
		Motor_check::Status::OK_
	}}};
}

bool operator<(Drivebase::Status,Drivebase::Status){ return 0; }
bool operator==(Drivebase::Status,Drivebase::Status){ return 1; }
bool operator!=(Drivebase::Status a,Drivebase::Status b){ return !(a==b); }

ostream& operator<<(ostream& o,Drivebase::Status const& a){
	o<<"Drivebase::Status(";
	o<<a.motor;
	return o<<")";
}

set<Drivebase::Goal> examples(Drivebase::Goal*){
	return {
		Drivebase::Goal{0,0,0},
		Drivebase::Goal{0,1,0}
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
	return {Drivebase::Input{{0,0,0}}};
}

bool operator<(Drivebase::Input const&,Drivebase::Input const&){
	NYI
}

ostream& operator<<(ostream& o,Drivebase::Input const& a){
	o<<"Drivebase::Input(";
	o<<a.current;
	return o<<")";
}

Drivebase::Status_detail Drivebase::Estimator::get()const{
	array<Motor_check::Status,MOTORS> a;
	for(unsigned i=0;i<a.size();i++){
		a[i]=motor_check[i].get();
	}
	return Status{a};
}

ostream& operator<<(ostream& o,Drivebase::Output_applicator){
	return o<<"output_applicator";
}

ostream& operator<<(ostream& o,Drivebase const& a){
	o<<"Drivebase(";
	o<<a.estimator.get();
	return o<<")";
}

double get_output(Drivebase::Output out,Drivebase::Motor m){
	#define X(NAME,POSITION) if(m==Drivebase::NAME) return out.POSITION;
	X(LEFT1,l)
	X(LEFT2,l)
	X(RIGHT1,r)
	X(RIGHT2,r)
	X(CENTER1,c)
	X(CENTER2,c)
	#undef X
	assert(0);
}

void Drivebase::Estimator::update(Time time,Drivebase::Input in,Drivebase::Output out){
	for(unsigned i=0;i<MOTORS;i++){
		Drivebase::Motor m=(Drivebase::Motor)i;
		auto current=in.current[i];
		auto set_power_level=get_output(out,m);
		motor_check[i].update(time,current,set_power_level);
	}
}

Robot_outputs Drivebase::Output_applicator::operator()(Robot_outputs robot,Drivebase::Output b)const{
	robot.pwm[0]=-pwm_convert(b.l);
	robot.pwm[1]=pwm_convert(b.r);
	robot.pwm[2]=pwm_convert(b.c);
	robot.solenoid[1]=fabs(b.c)>.1;
	return robot;
}

Drivebase::Output Drivebase::Output_applicator::operator()(Robot_outputs robot)const{
	return Drivebase::Output{
		-from_pwm(robot.pwm[0]),
		from_pwm(robot.pwm[1]),
		from_pwm(robot.pwm[2])
	};
}

bool operator==(Drivebase::Output_applicator const&,Drivebase::Output_applicator const&){
	return 1;
}

bool operator==(Drivebase::Estimator const&,Drivebase::Estimator const&){
	return 1;
}

bool operator!=(Drivebase::Estimator const& a,Drivebase::Estimator const& b){
	return !(a==b);
}

bool operator==(Drivebase const& a,Drivebase const& b){
	return a.estimator==b.estimator && a.output_applicator==b.output_applicator;
}

bool operator!=(Drivebase const& a,Drivebase const& b){ return !(a==b); }

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
