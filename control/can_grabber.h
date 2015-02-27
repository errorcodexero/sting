#ifndef CAN_GRABBER_H
#define CAN_GRABBER_H

#include <set>
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
	
	enum class Goal{TOP,BOTTOM};
	
	explicit Can_grabber(int);
};
bool operator!=(Can_grabber const&,Can_grabber const&);
bool operator<(Can_grabber::Input,Can_grabber::Input);
bool operator!=(Can_grabber::Estimator const&,Can_grabber::Estimator const&);
std::ostream& operator<<(std::ostream&,Can_grabber::Input);
std::ostream& operator<<(std::ostream&,Can_grabber::Output);
std::ostream& operator<<(std::ostream&,Can_grabber::Goal);
std::ostream& operator<<(std::ostream&,Can_grabber::Status_detail);
std::ostream& operator<<(std::ostream&,Can_grabber::Estimator);
std::ostream& operator<<(std::ostream&,Can_grabber::Output_applicator);
Can_grabber::Status status(Can_grabber::Status_detail const&);
bool ready(Can_grabber::Status,Can_grabber::Goal);
std::ostream& operator<<(std::ostream&,Can_grabber const&);
std::set<Can_grabber::Status_detail> examples(Can_grabber::Status_detail*);
std::set<Can_grabber::Input> examples(Can_grabber::Input*);
std::set<Can_grabber::Output> examples(Can_grabber::Output*);
Can_grabber::Output control(Can_grabber::Status,Can_grabber::Goal);

#endif
