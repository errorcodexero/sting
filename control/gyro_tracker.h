#ifndef GYRO_TRACKER_H
#define GYRO_TRACKER_H

#include "../util/interface.h"

class Integrator{
	Time last;
	
	public:
	double total;
	
	Integrator();
	void update(Time,double value);
	
	friend std::ostream& operator<<(std::ostream&,Integrator);
	friend bool operator==(Integrator,Integrator);
};
bool operator==(Integrator,Integrator);
std::ostream& operator<<(std::ostream&,Integrator);

typedef double Degree;

class Gyro_tracker{
	double cal_accumulated;
	unsigned cal_samples;
	Time cal_start;
	Integrator integrator;
	
	public:
	Gyro_tracker();
	Volt center;
	void update(Time now,Volt);
	Degree angle()const;
	
	friend std::ostream& operator<<(std::ostream&,Gyro_tracker);
	friend bool operator==(Gyro_tracker,Gyro_tracker);
	friend bool approx_equal(Gyro_tracker,Gyro_tracker);
};

bool operator==(Gyro_tracker,Gyro_tracker);
bool operator!=(Gyro_tracker,Gyro_tracker);
std::ostream& operator<<(std::ostream&,Gyro_tracker);
bool approx_equal(Gyro_tracker,Gyro_tracker);

#endif
