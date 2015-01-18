#ifndef OCTOCANUM_H
#define OCTOCANUM_H

#include<iosfwd>
#include<utility>

struct Traction_output{
	double l,r;
};

std::ostream& operator<<(std::ostream& o,Traction_output);

struct Traction_mode_goal{
	double y,theta;
	
	Traction_mode_goal();
	Traction_mode_goal(double,double);
};

std::ostream& operator<<(std::ostream&,Traction_mode_goal);

Traction_output calculate(Traction_mode_goal);

struct Mecanum_output{
	double lf,rf,lr,rr;
	
	Mecanum_output();
	Mecanum_output(double,double,double,double);
};

std::ostream& operator<<(std::ostream&,Mecanum_output);

struct Mecanum_goal{
	double x,y,theta;
	
	Mecanum_goal();
	Mecanum_goal(double,double,double);
};

std::ostream& operator<<(std::ostream& o,Mecanum_goal);

Mecanum_output calculate(Mecanum_goal);

struct Octocanum_output{
	bool traction_mode;
	Mecanum_output wheels;
	
	Octocanum_output();
	Octocanum_output(bool,Mecanum_output);
};

std::ostream& operator<<(std::ostream&,Octocanum_output);

struct Octocanum_goal{
	bool traction_mode;
	//could user a union here.
	Mecanum_goal mec;
	Traction_mode_goal t;
	
	Octocanum_goal();
	explicit Octocanum_goal(Traction_mode_goal);
	explicit Octocanum_goal(Mecanum_goal);
	Octocanum_goal& operator=(Traction_mode_goal);
	Octocanum_goal& operator=(Mecanum_goal);
};

std::ostream& operator<<(std::ostream& o,Octocanum_goal);

Mecanum_output as_mecanum(Traction_output);

typedef enum{TRACTION,MECH,TO_TRACTION,TO_MECH} Octocanum_mode;

std::ostream& operator<<(std::ostream& o,Octocanum_mode);

typedef double Time;//in seconds

Octocanum_mode next_mode(Octocanum_mode mode,bool goal_traction,Time elapsed);
Octocanum_output calculate(Octocanum_mode mode,Octocanum_goal goal);

struct Octocanum_state{
	Octocanum_mode mode;
	Time last_change_time;
	
	Octocanum_state();
};

std::ostream& operator<<(std::ostream&,Octocanum_state);

std::pair<Octocanum_state,Octocanum_output> run(Octocanum_state,Octocanum_goal,Time now);

#endif
