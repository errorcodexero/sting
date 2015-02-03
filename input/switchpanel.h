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

#endif
