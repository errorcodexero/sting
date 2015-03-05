#include "drivebase.h"
#include<iostream>
#include<math.h>
#include "../util/util.h"
#include "formal.h"

using namespace std;

unsigned pdb_location(Drivebase::Motor m){
        #define X(NAME,INDEX) if(m==Drivebase::NAME) return INDEX;
        X(LEFT1,12)
        X(LEFT2,13)
        X(RIGHT1,14)
        X(RIGHT2,15)
        X(CENTER1,2)
        X(CENTER2,3)
        #undef X
        assert(0);
        //assert(m>=0 && m<Drivebase::MOTORS);
}

Robot_inputs Drivebase::Input_reader::operator()(Robot_inputs all,Input in)const{
	for(unsigned i=0;i<MOTORS;i++){
		all.current[pdb_location((Motor)i)]=in.current[i];
	}
	return all;
}

Drivebase::Input Drivebase::Input_reader::operator()(Robot_inputs in)const{
	return Drivebase::Input{[&](){
		array<double,Drivebase::MOTORS> r;
		for(unsigned i=0;i<Drivebase::MOTORS;i++){
			Drivebase::Motor m=(Drivebase::Motor)i;
			r[i]=in.current[pdb_location(m)];
		}
		return r;
	}()};
}

ostream& operator<<(ostream& o,Drivebase::Piston a){
	#define X(NAME) if(a==Drivebase::Piston::NAME) return o<<""#NAME;
	PISTON_STATES
	#undef X
	assert(0);
}

IMPL_STRUCT(Drivebase::Status::Status,DRIVEBASE_STATUS)
IMPL_STRUCT(Drivebase::Input::Input,DRIVEBASE_INPUT)
IMPL_STRUCT(Drivebase::Output::Output,DRIVEBASE_OUTPUT)

CMP_OPS(Drivebase::Input,DRIVEBASE_INPUT)

CMP_OPS(Drivebase::Status,DRIVEBASE_STATUS)

set<Drivebase::Status> examples(Drivebase::Status*){
	return {Drivebase::Status{
		array<Motor_check::Status,Drivebase::MOTORS>{
			Motor_check::Status::OK_,
			Motor_check::Status::OK_,
			Motor_check::Status::OK_
		},
		Drivebase::Piston::FULL
	}};
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

CMP_OPS(Drivebase::Output,DRIVEBASE_OUTPUT)

set<Drivebase::Output> examples(Drivebase::Output*){
	return {
		Drivebase::Output{0,0,0,1},
		Drivebase::Output{1,1,0,0}
	};
}

set<Drivebase::Input> examples(Drivebase::Input*){
	return {Drivebase::Input{{0,0,0}}};
}

Drivebase::Status_detail Drivebase::Estimator::get()const{
	array<Motor_check::Status,MOTORS> a;
	for(unsigned i=0;i<a.size();i++){
		a[i]=motor_check[i].get();
	}
	return Status{a,piston};
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

Drivebase::Estimator::Estimator():piston_last(0),piston(Piston::EMPTYING){}

void Drivebase::Estimator::update(Time now,Drivebase::Input in,Drivebase::Output out){
	for(unsigned i=0;i<MOTORS;i++){
		Drivebase::Motor m=(Drivebase::Motor)i;
		auto current=in.current[i];
		auto set_power_level=get_output(out,m);
		motor_check[i].update(now,current,set_power_level);
	}

	if(out.piston==piston_last){
		piston_timer.update(now,1);
		if(piston_timer.done()){
			if(piston_last){
				piston=Piston::FULL;
			}else{
				piston=Piston::EMPTY;
			}
		}
	}else{
		if(out.piston){
			piston=Piston::FILLING;
		}else{
			piston=Piston::EMPTYING;
		}
		piston_last=out.piston;
		piston_timer.set(.2);//total guess
	}
}

Robot_outputs Drivebase::Output_applicator::operator()(Robot_outputs robot,Drivebase::Output b)const{
	robot.pwm[0]=-pwm_convert(b.l);
	robot.pwm[1]=pwm_convert(b.r);
	robot.pwm[2]=pwm_convert(b.c);
	robot.solenoid[1]=b.piston;
	return robot;
}

Drivebase::Output Drivebase::Output_applicator::operator()(Robot_outputs robot)const{
	return Drivebase::Output{
		-from_pwm(robot.pwm[0]),
		from_pwm(robot.pwm[1]),
		from_pwm(robot.pwm[2]),
		robot.solenoid[1]
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

Drivebase::Output control(Drivebase::Status status,Drivebase::Goal goal){
	double l=goal.y+goal.theta;
	double r=goal.y-goal.theta;
	auto m=max(1.0,max(fabs(l),fabs(r)));

	auto main_wheel_portion=max(fabs(l),fabs(r));
	auto strafe_portion=fabs(goal.x);
	auto mostly_stationary=max(main_wheel_portion,strafe_portion)<.1;
	bool piston=[=]()->bool{
		if(mostly_stationary){
			switch(status.piston){
				case Drivebase::Piston::FULL:
				case Drivebase::Piston::FILLING:
					return 1;
				case Drivebase::Piston::EMPTY:
				case Drivebase::Piston::EMPTYING:
					return 0;
				default: assert(0);
			}
		}
		return strafe_portion>=main_wheel_portion/2;
	}();

	return Drivebase::Output{l/m,r/m,goal.x,piston};
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
