#ifndef DIO_CONTROL_H
#define DIO_CONTROL_H

#include "util/interface.h"

class DigitalInput;
class DigitalOutput;
class Encoder;

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
std::ostream& operator<<(std::ostream&,DIO_control::Mode);
std::ostream& operator<<(std::ostream&,DIO_control const&);

struct Encoder_control{
	Encoder *encoder;
	int channel_a,channel_b;

	Encoder_control();
	~Encoder_control();
};

class DIO_controls{
	std::array<DIO_control,Robot_outputs::DIGITAL_IOS> channel;
	std::array<Encoder_control,Digital_inputs::ENCODERS> encoder;
	bool init_;

	DIO_controls(DIO_controls const&);
	DIO_controls& operator=(DIO_controls const&);

	void init();

	public:
	DIO_controls();

	void set(std::array<Digital_out,Robot_outputs::DIGITAL_IOS> const&);
	Digital_inputs get();
};
std::ostream& operator<<(std::ostream&,DIO_controls const&);

#endif
