#include "lift.h"
#include "../util/util.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

Lift::Estimator::Estimator():last(Lift::Status_detail::error()){}

void Lift::Estimator::update(Time,Lift::Input in,Lift::Output){
	if(in.top){
		if(in.bottom){
			last=Lift::Status_detail::error();
		}else{
			last=Lift::Status_detail::top();
		}
	}else{
		if(in.bottom){
			last=Lift::Status_detail::bottom();
		}else{
			last=Lift::Status_detail::mid(6+in.ticks/30.0);
		}
	}
}

Lift::Status_detail Lift::Estimator::get()const{ return last; }

Robot_outputs Lift::Output_applicator::operator()(Robot_outputs robot,Lift::Output lift)const{
	//just making up which pwm it is
	robot.pwm[4]=pwm_convert(lift);
	return robot;
}

Lift::Output Lift::Output_applicator::operator()(Robot_outputs in)const{
	return from_pwm(in.pwm[4]);
}

std::ostream& operator<<(std::ostream& o,Lift::Input const& a){
	o<<"Lift::Input(";
	o<<a.top<<a.bottom<<" "<<a.ticks;
	return o<<")";
}

#define CMP(name) if(a.name<b.name) return 1; if(b.name<a.name) return 0;

bool operator<(Lift::Input const& a,Lift::Input const& b){
	CMP(top) CMP(bottom) CMP(ticks)
	return 0;
}

std::set<Lift::Input> examples(Lift::Input*){
	return {
		Lift::Input{0,0,0},
		Lift::Input{0,1,0},
		Lift::Input{1,0,0},
		Lift::Input{1,1,0}
	};
}

std::set<Lift::Output> examples(Lift::Output*){ return {0,1,-1}; }

Lift::Status_detail::Status_detail(){}

Lift::Status_detail::Type Lift::Status_detail::type()const{ return type_; }

double Lift::Status_detail::inches_off_ground()const{
	assert(type_==Lift::Status_detail::Type::MID);
	return height;
}

Lift::Status_detail Lift::Status_detail::error(){
	Status_detail r;
	r.type_=Lift::Status_detail::Type::ERROR;
	return r;
}

Lift::Status_detail Lift::Status_detail::top(){
	Status_detail r;
	r.type_=Lift::Status_detail::Type::TOP;
	return r;
}

Lift::Status_detail Lift::Status_detail::bottom(){
	Status_detail r;
	r.type_=Lift::Status_detail::Type::BOTTOM;
	return r;
}

Lift::Status_detail Lift::Status_detail::mid(double d){
	Status_detail r;
	r.type_=Lift::Status_detail::Type::MID;
	r.height=d;
	return r;
}

ostream& operator<<(ostream& o,Lift::Status_detail::Type a){
	#define X(name) if(a==Lift::Status_detail::Type::name) return o<<""#name;
	X(ERROR) X(TOP) X(BOTTOM) X(MID)
	#undef X
	nyi
}

std::ostream& operator<<(ostream& o,Lift::Status_detail const& a){
	o<<"Lift::Status_detail(";
	o<<a.type();
	if(a.type()==Lift::Status_detail::Type::MID){
		o<<" "<<a.inches_off_ground();
	}
	return o<<")";
}

bool operator<(Lift::Status_detail const& a,Lift::Status_detail const& b){
	CMP(type())
	if(a.type()==Lift::Status_detail::Type::MID){
		return a.inches_off_ground()<b.inches_off_ground();
	}
	return 0;
}

bool operator==(Lift::Status_detail const& a,Lift::Status_detail const& b){
	if(a.type()!=b.type()) return 0;
	return a.type()!=Lift::Status_detail::Type::MID || a.inches_off_ground()==b.inches_off_ground();
}

std::set<Lift::Status_detail> examples(Lift::Status_detail*){
	return {
		Lift::Status_detail::top(),
		Lift::Status_detail::bottom(),
		Lift::Status_detail::error(),
		Lift::Status_detail::mid(0)
	};
}

std::ostream& operator<<(std::ostream& o,Lift::Goal a){
	#define X(name) if(a==Lift::Goal::name) return o<<""#name;
	X(MIN) X(MAX) X(STOP)
	#undef X
	nyi
}

std::ostream& operator<<(std::ostream& o,Lift const&){
	o<<"Lift(";
	return o<<")";
}

Lift::Status status(Lift::Status_detail const& a){
	return a;
}

Lift::Output control(Lift::Status_detail const& status,Lift::Goal const& goal){
	switch(goal){
		case Lift::Goal::MIN:
			switch(status.type()){
				case Lift::Status_detail::Type::BOTTOM:
				case Lift::Status_detail::Type::ERROR:
					return 0;
				case Lift::Status_detail::Type::TOP:
				case Lift::Status_detail::Type::MID:
					return -1;
				default: assert(0);
			}
		case Lift::Goal::MAX:
			switch(status.type()){
				case Lift::Status_detail::Type::TOP:
				case Lift::Status_detail::Type::ERROR:
					return 0;
				case Lift::Status_detail::Type::BOTTOM:
				case Lift::Status_detail::Type::MID:
					return 1;
				default: assert(0);
			}
		case Lift::Goal::STOP: return 0;
		default:
			nyi
	}
}

set<Lift::Goal> examples(Lift::Goal*){ return {Lift::Goal::MIN,Lift::Goal::MAX,Lift::Goal::STOP}; }

bool ready(Lift::Status status,Lift::Goal goal){
	switch(goal){
		case Lift::Goal::MIN: return status.type()==Lift::Status::Type::BOTTOM;
		case Lift::Goal::MAX: return status.type()==Lift::Status::Type::TOP;
		case Lift::Goal::STOP: return 1;
		default:
			nyi
	}
}

#ifdef LIFT_TEST
#include "formal.h"

int main(){
	/*for(double x=-1;x<=1;x+=.1){
		auto p=pwm_convert(x);
		auto rec=from_pwm(p);
		cout<<x<<"\t"<<(int)p<<"\t"<<rec<<"\n";
	}*/
	Lift a;
	tester(a);
	run(a,0,Lift::Input{0,0,0},Lift::Output{},Lift::Goal::MAX);
}
#endif
