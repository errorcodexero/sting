#include "gyro_tracker.h"
#include<iostream>
#include<cassert>
#include "../util/point.h"
using namespace std;

Integrator::Integrator():last(-1),total(0){}

void Integrator::update(Time t,double value){
	if(last!=-1){
		double elapsed=t-last;
		total+=elapsed*value;
	}
	last=t;
}

bool operator==(Integrator a,Integrator b){
	return a.last==b.last && a.total==b.total;
}

ostream& operator<<(ostream& o,Integrator a){
	return o<<"Integrator("<<a.last<<" "<<a.total<<")";
}

Gyro_tracker::Gyro_tracker():cal_accumulated(0),cal_samples(0),cal_start(-1),center(0){
}
/*
    m_center = (uint32_t)((float)value / (float)count + .5);
    m_offset = ((float)value / (float)count) - (float)m_center;
    //In this context, value is the accumulated value over (count) number of samples
   	  
    m_analog->GetAccumulatorOutput(&rawValue, &count);
    INT64 value = rawValue - (INT64)((float)count * m_offset);
    double scaledValue = value * 1e-9 * (double)m_analog->GetLSBWeight() * (double)(1 << m_analog->GetAverageBits()) /
	(m_analog->GetModule()->GetSampleRate() * m_voltsPerDegreePerSecond);
    return (float)scaledValue;
    
 */
void Gyro_tracker::update(Time now,Volt v){
	bool cal_done = cal_start!=-1 && now-cal_start>1;
	if(cal_done){
		//this value is totally bogus and needs to be calibrated or copied carefully from Gyro.cpp.
		double x=(v-center)*138.89;
		
		integrator.update(now,x);
	/* What needs to happen to get a meaningful value for degrees
	 CurrentVolts - CenterVolts = DiffVolts
	 DiffVolts * ConvertVoltsToDegPerSec = DegPerSec
	 DegPerSec * Seconds/Sample = DegressTraveled/Sample
	 DegressTraveled/Sample * Samples = TotalDegTraveled
	 AccumulatedDegrees += TotalDegTraveled
	 */
	/*
	double timePassed += (now - timePassed);
	double diff = v - center;
	double DegPerSec = diff * (1/0.0072) //This is the inverse of the value of Volts/(Deg/Sec) to get (Deg/Sec)/Volts
	double DegPerSam = DegPerSec *  
	 
	 */
	}else{
		if(cal_start==-1) cal_start=now;
		cal_accumulated+=v;
		cal_samples++;
		center = cal_accumulated / cal_samples;
	}
}

//it might make more sense to do the unit convertion in here instead of in update.
Degree Gyro_tracker::angle()const{ 
	return integrator.total; 
}

bool operator==(Gyro_tracker a,Gyro_tracker b){
	return a.cal_accumulated==b.cal_accumulated && a.cal_samples==b.cal_samples && a.cal_start==b.cal_start && a.center==b.center && a.integrator==b.integrator;
}

bool operator!=(Gyro_tracker a,Gyro_tracker b){
	return !(a==b);
}

bool approx_equal(Gyro_tracker a,Gyro_tracker b){
	return approx_equal(a.cal_accumulated,b.cal_accumulated) /*&& a.cal_samples==b.cal_samples*/ && approx_equal(a.cal_start,b.cal_start) && approx_equal(a.center,b.center) && approx_equal(a.integrator.total,b.integrator.total);
}

ostream& operator<<(ostream& o,Gyro_tracker a){
	return o<<"Gyro_tracker(cal_a:"<<a.cal_accumulated<<" cal_s:"<<a.cal_samples<<" cal_t:"<<a.cal_start<<" ctr:"<<a.center<<" int:"<<a.integrator<<")";
}

#ifdef GYRO_TRACKER_TEST
int main(){
	Integrator a;
	a.update(0,100);
	for(unsigned i=1;i<=10;i++){
		a.update(i,1);
	}
	cout<<a<<"\n";
	
	Gyro_tracker g;
	double expected_center=1.5;
	unsigned i;
	for(i=0;i<20;i++){
		g.update(i,expected_center);
	}
	cout<<g<<"\n";
	g.update(i,0);
	cout<<g<<"\n";
}
#endif
