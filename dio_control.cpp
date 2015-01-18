#include "dio_control.h"
#include <iostream>
#include "DigitalOutput.h"
#include "DigitalInput.h"
using namespace std;

//Unfortunately, there's not a good way to test all this without WPIlib since it's just an interface to WPIlib.

DIO_control::DIO_control():channel(-1),in(nullptr),out(nullptr){}
DIO_control::DIO_control(int i):channel(i),in(nullptr),out(nullptr){}
DIO_control::~DIO_control(){ free(); }

int DIO_control::set_channel(int i){
	if(channel!=-1) return 1;
	channel=i;
	return 0;
}

int DIO_control::set(Digital_out d){
	if(channel==-1) return 4;
	if(d==Digital_out::INPUT) return set_mode(Mode::IN);
	{
		int r=set_mode(Mode::OUT);
		if(r) return r;
	}
	if(!out) return 1;
	out->Set(d==Digital_out::_1);
	return 0;
}

Digital_in DIO_control::get()const{
	//return DI_0;
	if(!in){
		//cerr<<"Mode is actually: "<<mode<<"\n";
		return DI_OUTPUT;
	}
	return in->Get()?DI_1:DI_0;
}

DIO_control::Mode DIO_control::mode()const{
	assert(!in || !out);
	if(in) return Mode::IN;
	if(out) return Mode::OUT;
	return Mode::FREE;
}

int DIO_control::set_mode(Mode m){
	if(m==mode()) return 0;
	
	{
		int r=free();
		if(r) return r;
	}

	switch(m){
	case Mode::FREE: return 0;
	case Mode::IN:
		in=new DigitalInput(channel);
		break;
	case Mode::OUT:
		out=new DigitalOutput(channel);
		break;
	default: return 1;
	}
	if(!in && !out) return 2;
	return 0;
}
	
int DIO_control::free(){
	delete in;
	in=nullptr;
	delete out;
	out=nullptr;
	return 0;
}

ostream& operator<<(ostream& o,DIO_control::Mode a){
	switch(a){
	case DIO_control::Mode::IN: return o<<"in";
	case DIO_control::Mode::OUT: return o<<"out";
	case DIO_control::Mode::FREE: return o<<"free";
	default: return o<<"error";
	}
}

ostream& operator<<(ostream& o,DIO_control const& a){
	o<<"DIO_control(";
	o<<a.mode()<<" ";
	o<<a.channel;
	return o<<")";
}
