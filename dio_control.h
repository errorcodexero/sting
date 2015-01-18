#ifndef DIO_CONTROL_H
#define DIO_CONTROL_H

#include "util/interface.h"

class DigitalInput;
class DigitalOutput;

class DIO_control{
	public:
	enum class Mode{IN,OUT,FREE};
	
	private:
	int channel;
	DigitalInput *in;
	DigitalOutput *out;

	public:
	DIO_control();
	explicit DIO_control(int);
	~DIO_control();

	int set_channel(int);
	int set(Digital_out);
	Digital_in get()const;
	
	private:
	int set_mode(Mode);
	Mode mode()const;
	int free();
	
	friend std::ostream& operator<<(std::ostream&,DIO_control const&);
};

std::ostream& operator<<(std::ostream&,DIO_control const&);

#endif
