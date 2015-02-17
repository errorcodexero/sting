#include "lift.h"
#include <stdlib.h>
#include "../util/util.h"
#include "../util/interface.h"

using namespace std;

#define nyi { cout<<"\nnyi "<<__LINE__<<"\n"; exit(44); }

Lift::Estimator::Estimator():last(Lift::Status_detail::error()),bottom_location(0){}

void Lift::Estimator::update(Time,Lift::Input in,Lift::Output){
	if(in.top){
		if(in.bottom){
			last=Lift::Status_detail::error();
		}else{
			last=Lift::Status_detail::top();
			bottom_location=in.ticks-5712;
		}
	}else{
		if(in.bottom){
			last=Lift::Status_detail::bottom();
			bottom_location=in.ticks;
		}else{
			static const float CLICKS_PER_INCH = 8*11.7892550438441;
			last=Lift::Status_detail::mid(((in.ticks-bottom_location)/CLICKS_PER_INCH));
		}
	}
	//cout<<endl<<"Bottom: "<<bottom_location<<endl<<"Ticks: "<<in.ticks<<endl<<endl;
	
}

Lift::Status_detail Lift::Estimator::get()const{ return last; }

ostream& operator<<(ostream& o, Lift::Estimator estimator) {
	o<<"Lift::Estimator(";
	o<<"Last: "<<estimator.last<<"Bottom Location: "<<estimator.bottom_location;
	return o<<")";
}

ostream& operator<<(ostream& o, Lift::Output_applicator output_applicator) {
	o<<"Lift::Output_applicator(";
	o<<"Can Address: "<<output_applicator.can_address;
	return o<<")";
}

Lift::Output_applicator::Output_applicator(int a):can_address(a){}

Robot_outputs Lift::Output_applicator::operator()(Robot_outputs robot,Lift::Output lift)const{
	//just making up which pwm it is
	robot.talon_srx[can_address].power_level=lift;
	return robot;
}

Lift::Output Lift::Output_applicator::operator()(Robot_outputs in)const{
	return in.talon_srx[can_address].power_level;
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

std::set<Lift::Output> examples(Lift::Output*){ return {0,.45,-.45}; }

Lift::Status_detail::Status_detail(){}

Lift::Goal::Goal(){}

Lift::Status_detail::Type Lift::Status_detail::type()const{ return type_; }

double Lift::Status_detail::inches_off_ground()const{
	assert(type_==Lift::Status_detail::Type::MID);
	return height;
}

Lift::Status_detail Lift::Status_detail::error(){
	Status_detail r;
	r.type_=Lift::Status_detail::Type::ERRORS;
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

Lift::Goal::Mode Lift::Goal::mode()const{
	return mode_;
}

double Lift::Goal::height()const{
	assert(mode_==Lift::Goal::Mode::GO_TO_HEIGHT);
	return height_;
}

Lift::Goal Lift::Goal::up(){
	Lift::Goal r;
	r.mode_=Lift::Goal::Mode::UP;
	return r;
}

Lift::Goal Lift::Goal::down(){
	Lift::Goal r;
	r.mode_=Lift::Goal::Mode::DOWN;
	return r;
}

Lift::Goal Lift::Goal::stop(){
	Lift::Goal r;
	r.mode_=Lift::Goal::Mode::STOP;
	return r;
}

Lift::Goal Lift::Goal::go_to_height(double d){
	Lift::Goal r;
	r.mode_=Lift::Goal::Mode::GO_TO_HEIGHT;
	r.height_=d;
	return r;
}

bool operator==(Lift::Goal a,Lift::Goal b){
	if(a.mode()==Lift::Goal::Mode::GO_TO_HEIGHT) return a.mode()==b.mode() && a.height()==b.height();
	return a.mode()==b.mode();
}

bool operator!=(Lift::Goal a,Lift::Goal b){
	return !(a==b);
}

bool operator<(Lift::Goal a,Lift::Goal b){
	if(a.mode()==b.mode()){
		if(a.mode()==Lift::Goal::Mode::GO_TO_HEIGHT) return a.height()<b.height();
		return 0;
	}
	return a.mode()<b.mode();
}

ostream& operator<<(ostream& o,Lift::Status_detail::Type a){
	#define X(name) if(a==Lift::Status_detail::Type::name) return o<<""#name;
	X(ERRORS) X(TOP) X(BOTTOM) X(MID)
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

std::ostream& operator<<(std::ostream& o,Lift::Goal::Mode a){
	#define X(name) if(a==Lift::Goal::Mode::name) return o<<""#name;
	X(GO_TO_HEIGHT) X(DOWN) X(UP) X(STOP)
	#undef X
	nyi
}

std::ostream& operator<<(std::ostream& o,Lift::Goal a){
	o<<"mode: "<<a.mode();
	if(a.mode()==Lift::Goal::Mode::GO_TO_HEIGHT) o<<" height:"<<a.height();
	return o;
}

std::ostream& operator<<(std::ostream& o,Lift const& lift){
	o<<"Lift("<<"Estimator: "<<lift.estimator<<" Output Applicator: "<<lift.output_applicator;
	return o<<")";
}

Lift::Status status(Lift::Status_detail const& a){
	return a;
}

Lift::Output control(Lift::Status_detail const& status,Lift::Goal const& goal){
	const double PRESET_POWER=1.0;//The sign of this variable changes which direction the lifters go
	const double MANUAL_POWER=0.45;
	const double P=(PRESET_POWER/5);
	//cout<<endl<<"Inches off ground: "<<status.inches_off_ground()<<endl<<endl;
	if(goal.mode()==Lift::Goal::Mode::GO_TO_HEIGHT) {
		switch (status.type()) {
			case Lift::Status_detail::Type::MID:
				{
					double error = goal.height()-status.inches_off_ground();
					//cout<<endl<<"Error: "<<error<<endl<<endl;
					double desired_output_power =error*P;
					//cout<<"Desired Output Power: "<<desired_output_power<<endl<<endl;
					if (desired_output_power>PRESET_POWER) return PRESET_POWER;
					if (desired_output_power<-PRESET_POWER) return -PRESET_POWER;
					return desired_output_power;
					/*if (status.inches_off_ground()<goal.height()) return POWER;
					else if (status.inches_off_ground()>goal.height()) return -POWER;
					return 0.0;*/
				}
			case Lift::Status_detail::Type::TOP:
				return -MANUAL_POWER;
			case Lift::Status_detail::Type::BOTTOM:
				return MANUAL_POWER;
			case Lift::Status_detail::Type::ERRORS:
				return 0.0;
			default:
				assert(0);
		}
	}
	if(goal.mode()==Lift::Goal::Mode::UP) return  MANUAL_POWER; 
	if(goal.mode()==Lift::Goal::Mode::DOWN) return -MANUAL_POWER;
	if(goal.mode()==Lift::Goal::Mode::STOP) return 0.0;
	assert(0);
	/*switch(goal){
		case Lift::Goal::Mode::DOWN:
			switch(status.type()){
				case Lift::Status_detail::Type::BOTTOM:
				case Lift::Status_detail::Type::ERROR:
					return 0;
				case Lift::Status_detail::Type::TOP:
				case Lift::Status_detail::Type::MID:
					return -1;
				default: assert(0);
			}
		case Lift::Goal::Mode::UP:
			switch(status.type()){
				case Lift::Status_detail::Type::TOP:
				case Lift::Status_detail::Type::ERROR:
					return 0;
				case Lift::Status_detail::Type::BOTTOM:
				case Lift::Status_detail::Type::MID:
					return 1;
				default: assert(0);
			}
		case Lift::Goal::Mode::STOP: return 0;
		default:
			nyi
	}*/
}

set<Lift::Goal> examples(Lift::Goal*){ 
	set<Lift::Goal> goals;
	Lift::Goal goal=goal.go_to_height(0);
	goals.insert(goal);
	goal=goal.down();
	goals.insert(goal);
	goal=goal.up();
	goals.insert(goal);
	goal=goal.stop();
	goals.insert(goal);
	return goals;//{Lift::Goal::Mode::GO_TO_HEIGHT,Lift::Goal::Mode::DOWN,Lift::Goal::Mode::UP,Lift::Goal::Mode::STOP}; 
}

Lift::Lift(int can_address):output_applicator(can_address){}

bool ready(Lift::Status status,Lift::Goal goal){
	switch(goal.mode()){
		case Lift::Goal::Mode::GO_TO_HEIGHT: return 1;
		case Lift::Goal::Mode::DOWN: return status.type()==Lift::Status::Type::BOTTOM;
		case Lift::Goal::Mode::UP: return status.type()==Lift::Status::Type::TOP;
		case Lift::Goal::Mode::STOP: return 1;
		default:
			nyi
	}
}

#ifdef LIFT_TEST
#include "formal.h"

static const double MAX_HEIGHT=65-5;
static const double MIN_HEIGHT=6;
static const double LIMIT_SWITCH_RANGE=.25;

static const unsigned TICKS_PER_REVOLUTION=60;//todo: check this

static const unsigned CIM_FREE_SPEED=5310;//RPM
static const unsigned CIM_STALL_TORQUE=343.4;//oz-in
static const unsigned REDUCTION=10;//to 1
static const double CHAIN_EFFICIENCY=.9;
static const double PLANETARY_EFFICIENCY=.8;

static const double SPROCKET_RADIUS=1+11.0/16;//inches

static const double TOTE_WEIGHT=10.5;//lb
static const double CAN_WEIGHT=12;//check this
static const double CARRIAGE_WEIGHT=8;//lb, just a guesss

//rpm->in/s
double linear_speed(double motor_speed){
	const double shaft_speed=motor_speed/REDUCTION/60;//rev/sec
	const double circumference=2*SPROCKET_RADIUS*M_PI;
	return shaft_speed*circumference;
}

//returns in/sec
double free_speed_linear(){
	return linear_speed(CIM_FREE_SPEED);
}

//lb->oz in
double torque_required(double load){
	const double load_oz=load*16/REDUCTION;
	return load_oz*SPROCKET_RADIUS;
}

//load in ppounds, returns in/sec
double speed_with_load(double load){
	auto t=torque_required(load);
	const double torque_portion_left=CHAIN_EFFICIENCY*PLANETARY_EFFICIENCY-t/CIM_STALL_TORQUE;
	//note that torque portion left might be negative
	const double motor_speed=torque_portion_left*CIM_FREE_SPEED;
	return linear_speed(motor_speed);
}

struct Lift_sim{
	double height;//in inches off the ground for the end effector

	Lift_sim():height(10){}

	Lift::Input now(){
		return Lift::Input{
			height>=MAX_HEIGHT-LIMIT_SWITCH_RANGE,
			height<=MIN_HEIGHT+LIMIT_SWITCH_RANGE,
			(int)(height/(2*M_PI*SPROCKET_RADIUS)*TICKS_PER_REVOLUTION)
		};
	}
};

ostream& operator<<(ostream& o,Lift_sim const& a){
	o<<"Lift_sim(";
	o<<a.height;
	return o<<")";
}

static const double GEAR_RATIO=10;//to 1

//returns motor speed
double linear_to_rpm(double in_per_sec){
	return (in_per_sec*60)/SPROCKET_RADIUS*GEAR_RATIO;
}

//returns in/s^2
pair<double,double> acceleration_range(double load/*lb - including lifter parts*/,double current_speed/*in inches/s*/){
	static const double CIM_STALL_TORQUE=343.4/16;//lb*in
	static const double CIM_FREE_SPEED=5310;//rpm
	double current_rpm=linear_to_rpm(current_speed);//rpm
	//cout<<"current_rpm:"<<current_rpm<<"\n";
	static const double PLANETARY_EFFICIENCY=.8;
	static const double CHAIN_EFFICIENCY=.9;
	double current_available_torque=CIM_STALL_TORQUE*PLANETARY_EFFICIENCY*CHAIN_EFFICIENCY*(1-current_rpm/CIM_FREE_SPEED);//lb-in
	//cout<<"oz in avail:"<<current_available_torque<<"\n";

	//assume going up:
	double force=current_available_torque*GEAR_RATIO/SPROCKET_RADIUS;//lb
	//cout<<"\tforce:"<<force<<"\n";
	double net_force=force-load;//lb
	static const double G_IN_IN_PER_S=32.2*12;
	double max_accel=net_force/load*G_IN_IN_PER_S;

	//assume going down:
	double reverse_available_torque=CIM_STALL_TORQUE*PLANETARY_EFFICIENCY*CHAIN_EFFICIENCY*(-1-current_rpm/CIM_FREE_SPEED);//lb-in
	double force_min=reverse_available_torque*GEAR_RATIO/SPROCKET_RADIUS;//lb
	double min_net_force=force_min-load;//lb
	double min_accel=min_net_force/load*G_IN_IN_PER_S;//lb

	return make_pair(min_accel,max_accel);
}

int main(){
	//need to calculate the speed at which need to stop in order to avoid throwing totes
	/*32 ft/s/s
	12*32=384 in/s/s
	estimated speed to robot, acceleration & rotation -> lift accel limits?
	desired lift accel
	required lift accel/decel
	priorities:
	1) required deceleration of lifter (due to end of rails)
	2) robot movement
	3) acceleration of lifter/deceleration due to destination approaching in a way that's open*/

	cout<<free_speed_linear()<<"\n";
	for(int i=0;i<=7;i++){
		double d=CARRIAGE_WEIGHT+i*TOTE_WEIGHT;
		cout<<i<<"\t"<<d<<"\t"<<speed_with_load(d)<<"\n";
	}
	/*for(double x=-1;x<=1;x+=.1){
		auto p=pwm_convert(x);
		auto rec=from_pwm(p);
		cout<<x<<"\t"<<(int)p<<"\t"<<rec<<"\n";
	}*/
	Lift a(4);
	tester(a);
	Lift::Goal goal=goal.up();
	run(a,0,Lift::Input{0,0,0},Lift::Output{},goal);

	for(unsigned i=0;i<30;i++){
		cout<<i<<"\t"<<acceleration_range(60,i)<<"\n";
	}
}
#endif
