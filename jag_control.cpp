#include "jag_control.h"
#include "WPILib.h"
#include <iostream>
#include <cassert>
#include <math.h>

using namespace std;

ostream& operator<<(ostream& o,Jag_control::Mode m){
	#define X(name) if(m==Jag_control::name) return o<<""#name;
	X(INIT)
	X(SPEED)
	X(VOLTAGE)
	X(DISABLE)
	#undef X
	assert(0);
	return o;
}

ostream& operator<<(ostream& o,Jag_control const& j){
	j.output(o);
	return o;
}

bool pidapprox(Jaguar_output a,Jaguar_output b){
	return fabs(a.pid.p-b.pid.p)<.001&&fabs(a.pid.i-b.pid.i)<.001&&fabs(a.pid.d-b.pid.d)<.001;
}

void Jag_control::init(int CANBusAddress){
	assert(!jaguar);//initialization is only allowed once.
	assert(mode==INIT);
	jaguar = new CANJaguar(CANBusAddress);
	assert(jaguar);
	jaguar->DisableControl();
	jaguar->SetSafetyEnabled(0);
	mode=DISABLE;
/*	jaguar->SetSafetyEnabled(false);
	jaguar->Set(0.0, SYNC_GROUP);
	jaguar->ChangeControlMode(CANJaguar::kSpeed);
	controlSpeed = true;
	jaguar->ConfigEncoderCodesPerRev(1);*/
}

Jag_control::Jag_control():jaguar(NULL),since_query(0),mode(INIT) {}

Jag_control::Jag_control(int CANBusAddress):jaguar(NULL),mode(INIT){
	init(CANBusAddress);
}

Jag_control::~Jag_control(){
	delete jaguar;
}
	
void Jag_control::set(Jaguar_output a,bool enable){
	assert(mode!=INIT);
	if(!enable){
		if(mode==DISABLE){
			return;
		}else{
			jaguar->Set(0,SYNC_GROUP);
			//update sync group here?
			jaguar->DisableControl();
			jaguar->SetSafetyEnabled(0);
			mode=DISABLE;
			return;
		}
	}
	//const float kP = 1.000;
	//const float kI = 0.005;
	/*const float kP = .3000;
	const float kI = 0.003;
	const float kD = 0.000;*/
	if(a.controlSpeed){
		if(mode!=SPEED|| !pidapprox(out,a)){
			jaguar->SetSpeedMode(CANJaguar::Encoder, 1, a.pid.p,a.pid.i,a.pid.d);
			jaguar->EnableControl();
			jaguar->SetExpiration(2.0);
			jaguar->Set(a.speed,SYNC_GROUP);
			CANJaguar::UpdateSyncGroup(SYNC_GROUP);
			jaguar->SetSafetyEnabled(true);
			mode=SPEED;
			out=a;
		}else if(a.speed!=out.speed || since_query==1){
			jaguar->Set(a.speed,SYNC_GROUP);
			CANJaguar::UpdateSyncGroup(SYNC_GROUP);
			out.speed=a.speed;
		}
	}else{
		if(mode!=VOLTAGE){
			jaguar->SetPercentMode(CANJaguar::Encoder, 1);
			jaguar->EnableControl();
			jaguar->SetExpiration(2.0);
			jaguar->SetVoltageRampRate(45);
			jaguar->Set(a.voltage,SYNC_GROUP);
			CANJaguar::UpdateSyncGroup(SYNC_GROUP);
			jaguar->SetSafetyEnabled(true);
			mode=VOLTAGE;
		}else if(a!=out || since_query==1){
			jaguar->Set(a.voltage,SYNC_GROUP);
			CANJaguar::UpdateSyncGroup(SYNC_GROUP);
			out=a;
		}
	}
}

Jaguar_input Jag_control::get(){
	if(since_query>20){
		in.speed = jaguar -> GetSpeed();
		since_query=0;
	}
	since_query++;
	return in;
}

void Jag_control::output(ostream& o)const{
	o<<"Jag_control(";
	o<<"init:"<<!!jaguar;
	o<<" "<<mode;
	o<<")";
}
