#ifndef BUNNY_LAUNCHER
#define BUNNY_LAUNCHER

#include "../util/interface.h"

class Bunny_launcher{
	public:
	typedef enum{COCKED,LAUNCHING,LAUNCHED} State;
	
	private:
	State state_;
	Time launch_start;
	
	public:
	Bunny_launcher();
	void update(Time,bool disabled,bool launch,bool reset);
	bool output()const;
	State state()const;
	
	friend std::ostream& operator<<(std::ostream&,Bunny_launcher);
};

std::ostream& operator<<(std::ostream&,Bunny_launcher::State);
std::ostream& operator<<(std::ostream&,Bunny_launcher);

#endif
