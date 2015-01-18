#include "wheel_sim.h"
#include<iostream>
#include<cassert>
#include<cmath>
#include "../util/util.h"
#include "shooter_status.h"
#include "shooter_wheels.h"

using namespace std;
using Shooter_wheels::RPM;

double portion_of_max_speed(RPM r){
	return (double)r/Shooter_wheels::free_speed();
}

//estimated basec on some open loop shooting tests.
static const double BREAK_FREE_POWER=.04199;

/*double portion_used_as_drag(Shooter_wheels::RPM wheel_speed){
	//the drag appears to be mostly linear
	const double stop_power=.04199;
	return stop_power+(1-stop_power)*portion_of_max_speed(wheel_speed);
}*/

double oz_in_to_ft_lb(double d){
	return d/16/12;
}

double ft_lb_to_nm(double d){
	return d/0.73756214837;
}

double oz_in_to_nm(double d){
	double ft_lb=oz_in_to_ft_lb(d);
	return ft_lb_to_nm(ft_lb);
}

static const double BANEBOTS_550_FREE_SPEED=19300;

//in N*M
double torque_available(RPM rpm){
	static const double BANEBOTS_550_TORQUE_OZ_IN=70.55;
	static const double REDUCTION=2.7;
	static const int NUMBER_OF_MOTORS=2;
	double motor_speed=rpm*REDUCTION;
	double torque_proportion=1-motor_speed/BANEBOTS_550_FREE_SPEED;
	//cout<<"torque proportion:"<<torque_proportion<<"\n";
	static const double max_torque=oz_in_to_nm(BANEBOTS_550_TORQUE_OZ_IN)*REDUCTION*NUMBER_OF_MOTORS;
	static const double EFFICIENCY=.75;
	return max_torque*torque_proportion*EFFICIENCY;
}

double drag(Shooter_wheels::RPM wheel_speed){
	if(wheel_speed<0) return -drag(-wheel_speed);
	double min_drag=torque_available(0)*BREAK_FREE_POWER;
	double max_drag=torque_available(Shooter_wheels::free_speed());
	return min_drag+portion_of_max_speed(wheel_speed)*(max_drag-min_drag);
}

double in_to_m(double d){
	return d*.0254;
}

//in kg*m^2
double moment_of_inertia(){
	static const double WHEEL_WEIGHT_LB=1.25;//lb
	static const double WHEEL_WEIGHT_KG=2.215*WHEEL_WEIGHT_LB;
	//assuming that this behaves as a disk.
	static const double WHEEL_DIA_IN=6;
	double wheel_dia_m=in_to_m(WHEEL_DIA_IN);

	//.25 for uniformly distributed weight
	//would be 1 if w assumed all the weight was at the edge.
	//the truth is somehwere in the middle.
	return .5*WHEEL_WEIGHT_KG*wheel_dia_m*wheel_dia_m;
}

static const double TWO_PI=2*3.1415;

double rad_per_s_to_rpm(double d){
	return d/TWO_PI*60;
}

//double rpm_to_rad_per_s(double d){
//	return 

//in N*m
double output_torque(RPM rpm,double power_portion){
	return torque_available(rpm)*power_portion-drag(rpm);
}

//returns RPM
double step(double initial /*in rpms*/,Time stepsize,double power_portion,bool shoot_end){
	if(shoot_end){
		return initial/4;
	}

	assert(fabs(power_portion)<=1);//take this out when actually running on the robot, or at least make the range larger.
	power_portion=clip(power_portion);
	//power_portion-=portion_used_as_drag(initial);
	//double torque=torque_available(initial)*power_portion;
	//if(power_portion<0) torque*=-1;//this could be done in a nicer way.
	double torque=output_torque(initial,power_portion);

	//cout<<"torque="<<torque<<"\n";

	static const double G=9.8; //kg/N conversion
	//in rad/s/s
	double angular_acceleration=torque/(moment_of_inertia()/G);
	//cout<<"angh:"<<angular_acceleration<<"\n";

	//in rad/s
	double speed_change=angular_acceleration*stepsize;
	//cout<<"speed ch:"<<speed_change<<"\n";

	double diff=rad_per_s_to_rpm(speed_change);
	//cout<<"diff="<<diff<<"\n";

	return initial+diff;
}

//rpms->watts
/*double wheel_energy(double speed){
	return .5*moment_of_inertia()*
}*/

/*double power_step(double initial,Time stepsize,double power_portion){
	static const double BANEBOTS_540_POWER=123.49;//watts
	assert(0);
}*/

Wheel_sim::Wheel_sim():last(-1),est(0){}

void Wheel_sim::update(Time now,double power,bool shooting){
	if(last==-1){
		last=now;
	}
	Time step_size=now-last;
	//cout<<"step="<<step_size<<"\n";
	est=step(est,step_size,power,shooter(shooting));
	last=now;
}

RPM Wheel_sim::estimate()const{ return est; }

ostream& operator<<(ostream& o,Wheel_sim a){
	o<<"Wheel_sim(";
	o<<a.last<<" "<<a.est;
	return o<<")";
}

void Shooter_sim::update(Time t,Shooter_wheels::Output a,bool shooting){
	//for now we're assuming that the motor that's in PID mode has the same output as the one in voltage mode.  This is incorrect, but much simpler than the alternatives.
	//top.update(t,a.top[Shooter_wheels::Output::OPEN_LOOP].voltage);
	//bottom.update(t,a.bottom[Shooter_wheels::Output::OPEN_LOOP].voltage);

	//just implement bang-bang control for the simulation.
	bool top_on=a.top[Shooter_wheels::Output::FEEDBACK].speed>top.estimate();
	bool bottom_on=a.bottom[Shooter_wheels::Output::FEEDBACK].speed>bottom.estimate();
	//cout<<"Bang!:"<<top_on<<bottom_on<<"\n";
	top.update(t,top_on,shooting);
	bottom.update(t,bottom_on,shooting);
}

Shooter_wheels::Status Shooter_sim::estimate()const{
	return Shooter_wheels::Status(top.estimate(),bottom.estimate());
}

ostream& operator<<(ostream& o,Shooter_sim a){
	o<<"Shooter_sim(";
	o<<a.top<<a.bottom;
	return o<<")";
}

#ifdef WHEEL_SIM_TEST
void test_drag(){
	cout<<"RPM:Drag (N*m)\n";
	for(unsigned i=0;i<30;i++){
		double d=i*300.0;
		cout<<d<<":"<<drag(d)<<"\n";
	}
}

void torque_test(){
	cout<<"RPM:Torque (N*m)\n";
	for(unsigned i=0;i<30;i++){
		double d=i*300;
		cout<<d<<":"<<torque_available(d)<<"\n";
	}
}

void sim_test(){
	double rpm=0;
	Time STEPSIZE=.1;
	for(Time t=0;t<105;t+=STEPSIZE){
		cout<<t<<"\t"<<rpm<<"\n";
		rpm=step(rpm,STEPSIZE,0,0);
	}
}

void shooter_sim_test(){
	Shooter_sim s;
	wheelcalib calib=rpmsdefault();
	calib.highgoal.top=0;
	auto c=Shooter_wheels::control(
		Shooter_wheels::Status(),
		convert_goal(make_pair(calib,PID_coefficients()),Shooter_wheels::HIGH_GOAL)
	);
	cout<<c<<"\n";
	for(Time t=0;t<5;t+=.1){
		s.update(
			t,c,0
		);
		cout<<s<<"\n";
	}
}

int main(){
	using Shooter_wheels::RPM;

	test_drag();

	//sim_test();

	torque_test();

	shooter_sim_test();

	/*Wheel_sim w;
	for(Time t=0;t<3;t+=.05){
		cout<<w<<"\n";
		w.update(t,1);
	}*/

	return 0;
}
#endif
