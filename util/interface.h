#ifndef INTERFACE_H
#define INTERFACE_H

#include <iosfwd>
#include <bitset>
#include <array>
#include "jag_interface.h"
#include "driver_station_interface.h"
#include "maybe_inline.h"

typedef double Time;//Seconds
typedef double Pwm_output;
typedef bool Solenoid_output;

//enum class Digital_out{INPUT,_1,_0};
//std::ostream& operator<<(std::ostream&,Digital_out);

enum class Relay_output{_00,_01,_10,_11};
std::ostream& operator<<(std::ostream&,Relay_output);

class Digital_out{
	public:
	enum class Type{INPUT,_1,_0,ENCODER};

	private:
	Type type_;
	int encoder_index_;
	bool input_a_;

	public:
	Digital_out();

	Type type()const;
	int encoder_index()const;
	bool input_a()const;

	static Digital_out input();
	static Digital_out one();
	static Digital_out zero();
	static Digital_out encoder(int encoder_index,bool input_a);
};

struct Talon_srx_input{
	int encoder_position;
	bool fwd_limit_switch;
	bool rev_limit_switch;
	bool a;
	bool b;
	int velocity = 0;
	double current;
	Talon_srx_input():encoder_position(0),fwd_limit_switch(0),rev_limit_switch(0),a(0),b(0),current(0){}
};

struct Talon_srx_output{
	double power_level;
	Talon_srx_output():power_level(0){}
};

std::ostream& operator<<(std::ostream&,Digital_out);
std::ostream& operator<<(std::ostream&,Talon_srx_input);
std::ostream& operator<<(std::ostream&,Talon_srx_output);
bool operator==(Talon_srx_output,Talon_srx_output);
bool operator!=(Talon_srx_output,Talon_srx_output);
bool operator<(Talon_srx_output,Talon_srx_output);
bool operator==(Talon_srx_input,Talon_srx_input);
bool operator!=(Talon_srx_input,Talon_srx_input);
bool operator<(Talon_srx_input,Talon_srx_input);
bool operator<(Digital_out,Digital_out);
bool operator==(Digital_out,Digital_out);
bool operator!=(Digital_out,Digital_out);

struct Robot_outputs{
	static const unsigned PWMS=10;//Number of ports on the digital sidecar; roboRIO headers say 20 but there aren't that many ports on the board.
	std::array<Pwm_output,PWMS> pwm;
	
	static const unsigned SOLENOIDS=8;
	std::array<Solenoid_output,SOLENOIDS> solenoid;
	
	static const unsigned RELAYS=8;
	std::array<Relay_output,RELAYS> relay;
	
	static const unsigned DIGITAL_IOS=10;//there are really 14 on the cRIO and the roboRIO headers say 26.
	std::array<Digital_out,DIGITAL_IOS> digital_io;
	
	static const unsigned TALON_SRX_OUTPUTS=2;
	std::array<Talon_srx_output, TALON_SRX_OUTPUTS> talon_srx;
	
	static const unsigned CAN_JAGUARS=0;
	std::array<Jaguar_output,CAN_JAGUARS> jaguar;
	
	//could add in some setup for the analog inputs
	
	Driver_station_output driver_station;
	bool pump_auto;

	Robot_outputs();
};

std::ostream& operator<<(std::ostream& o, Talon_srx_output);

std::ostream& operator<<(std::ostream& o, Talon_srx_input);

bool operator<(Robot_outputs,Robot_outputs);
bool operator==(Robot_outputs,Robot_outputs);
bool operator!=(Robot_outputs,Robot_outputs);
std::ostream& operator<<(std::ostream& o,Robot_outputs);

//limitation of FRC coms
#define JOY_AXES 8
#define JOY_BUTTONS 12

struct Joystick_data{
	std::array<double,JOY_AXES> axis;
	std::bitset<JOY_BUTTONS> button;
	
	Joystick_data();

	static Maybe<Joystick_data> parse(std::string const&);
};
bool operator==(Joystick_data,Joystick_data);
bool operator!=(Joystick_data,Joystick_data);
std::ostream& operator<<(std::ostream&,Joystick_data);

//We may need to add support for other modes at some point.
struct Robot_mode{
	bool autonomous;
	bool enabled;
	
	Robot_mode();
};
bool operator==(Robot_mode,Robot_mode);
bool operator!=(Robot_mode,Robot_mode);
std::ostream& operator<<(std::ostream&,Robot_mode);

enum class Digital_in{OUTPUT,_0,_1,ENCODER};
std::ostream& operator<<(std::ostream&,Digital_in);

typedef int Encoder_output;

struct Digital_inputs{
	std::array<Digital_in,Robot_outputs::DIGITAL_IOS> in;
	static const unsigned ENCODERS=Robot_outputs::DIGITAL_IOS/2;
	std::array<Maybe_inline<Encoder_output>,ENCODERS> encoder;

	Digital_inputs();
};
bool operator==(Digital_inputs const&,Digital_inputs const&);
bool operator!=(Digital_inputs const&,Digital_inputs const&);
std::ostream& operator<<(std::ostream&,Digital_inputs const&);

typedef float Volt;
typedef double Rad; //radians, clockwise

struct Robot_inputs{
	Robot_mode robot_mode;
	Time now;//time since boot.

	static const unsigned JOYSTICKS=3; //limitation of FRC coms was 4, now highter
	std::array<Joystick_data,JOYSTICKS> joystick;

	//std::array<Digital_in,Robot_outputs::DIGITAL_IOS> digital_io;
	Digital_inputs digital_io;	

	static const unsigned ANALOG_INPUTS=4;
	std::array<Volt,ANALOG_INPUTS> analog;

	static const unsigned TALON_SRX_INPUTS=2;
	std::array<Talon_srx_input, TALON_SRX_INPUTS> talon_srx;
	
	std::array<Jaguar_input,Robot_outputs::CAN_JAGUARS> jaguar;
	Driver_station_input driver_station;
	Rad orientation;
		
	static const unsigned CURRENT =16;
	std::array<double,CURRENT> current;
	bool pump;

	Robot_inputs();
};
bool operator==(Robot_inputs,Robot_inputs);
bool operator!=(Robot_inputs,Robot_inputs);
std::ostream& operator<<(std::ostream& o,Robot_inputs);

#endif
