#ifndef CAN_GRABBER_H
#define CAN_GRABBER_H

#include "../util/interface.h"
#include "../util/countdown_timer.h"

struct Can_grabber {
	struct Input {
		bool sensor;
	};
	
	enum class Output{ON,OFF};
	
	enum class Status_detail{TOP,MID_DOWN,BOTTOM,MID_UP};
	
	typedef Status_detail Status;
	
	struct Estimator {
		Status_detail last;
		
		Estimator();

		void update(Time,Input,Output);
		Status_detail get()const;
		
		Countdown_timer timer;
	};
	Estimator estimator;
	
	struct Output_applicator{
		int can_address;

		explicit Output_applicator(int);

		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;
	
	typedef Output Goal;
	
	explicit Can_grabber(int);
};
/*
std::ostream& operator<<(std::ostream&,Lift::Input const&);
bool operator<(Lift::Input const&,Lift::Input const&);
std::set<Lift::Input> examples(Lift::Input*);

std::set<Lift::Output> examples(Lift::Output*);

std::ostream& operator<<(std::ostream&,Lift::Status_detail::Type);
std::ostream& operator<<(std::ostream&,Lift::Status_detail const&);
bool operator<(Lift::Status_detail const&,Lift::Status_detail const&);
bool operator==(Lift::Status_detail const&,Lift::Status_detail const&);
std::set<Lift::Status_detail> examples(Lift::Status_detail*);

std::ostream& operator<<(std::ostream&,Lift::Goal::Mode);
std::ostream& operator<<(std::ostream&,Lift::Goal);
bool operator==(Lift::Goal,Lift::Goal);
bool operator!=(Lift::Goal,Lift::Goal);
bool operator<(Lift::Goal,Lift::Goal);
std::set<Lift::Goal> examples(Lift::Goal*);

std::ostream& operator<<(std::ostream&,Lift const&);

Lift::Status status(Lift::Status_detail const&);
Lift::Output control(Lift::Status_detail const&, Lift::Goal const&);
bool ready(Lift::Status,Lift::Goal::Mode);
*/
#endif