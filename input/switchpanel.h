#ifndef SWITCHPANEL_H
#define SWITCHPANEL_H

typedef enum{SW3_UP,SW3_MIN,SW3_DOWN} Switch3;

class Dial10{
	int i;
	
	public:
	explicit Switch10(int);
	int value();
};

std::ostream& operator<<(std::ostream&,Dial10);

struct Switchpanel{
	static const unsigned SWITCHES=9;
	Switch3 switches[SWITCHES];
	
	static const unsigned DIALS=3;
	Dial10 dial[DIALS];
	
	static const unsigned BUTTONS=5;
	bool button[BUTTONS];
	
	Switchpanel();
};

std::ostream& operator<<(std::ostream&,Switchpanel);

#endif