#ifndef JAG_CONTROL_H
#define JAG_CONTROL_H

#include <iosfwd>
// #ifndef _WRS_KERNEL
#include <stdint.h> //for uint8_t
// #endif
#include "util/jag_interface.h"

class CANJaguar;

class Jag_control
{
public:
	static const uint8_t SYNC_GROUP=0x40;
private:
	CANJaguar *jaguar;

	Jaguar_input in;
	int since_query;
	
	Jaguar_output out;
public:
	enum Mode{INIT,SPEED,VOLTAGE,DISABLE};
	//at some point we may want to remember the speed/voltage that we were driving to so that we can just send it when it changes.
	
private:
	//bool controlSpeed; //0 = voltage; 1 = speed
	Mode mode;
	
public:
	void init(int CANBusAddress);
	Jag_control();
	explicit Jag_control(int CANBusAddress);
	
private:
	Jag_control(Jag_control const&);
	Jag_control& operator=(Jag_control const&);
	
public:
	~Jag_control();
	
	void set(Jaguar_output,bool enable);
	Jaguar_input get();
	void output(std::ostream&)const;
};

std::ostream& operator<<(std::ostream&,Jag_control::Mode);
std::ostream& operator<<(std::ostream&,Jag_control const&);

#endif
