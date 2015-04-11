#ifndef CAN_GRABBER_H
#define CAN_GRABBER_H

#include <set>
#include "../util/interface.h"
#include "../util/countdown_timer.h"
#include "../util/quick.h"
#include "lift.h"

struct Can_grabber{
	//#define CAN_GRABBER_INPUT(X) X(Lift::Input,lift) X(bool,grab_down)
	struct Input{
		Lift::Input lift;
		bool grab_down;
	};
	
	//DECLARE_STRUCT(Input,CAN_GRABBER_INPUT)
	//typedef Lift::Input Input;
	//typedef Lift::Input_reader Input_reader;

	struct Input_reader{
		Lift::Input_reader lift;
		explicit Input_reader(unsigned);
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};
	Input_reader input_reader;

	enum class Output{RELEASE,LOCK};
	
	#define CAN_GRABBER_STATUS X(INITIAL) X(GOING_DOWN) X(DOWN) X(GOING_UP) X(STUCK_UP) X(GETTING_STUCK)
	enum class Status{
		#define X(NAME) NAME,
		CAN_GRABBER_STATUS
		#undef X
	};

	struct Status_detail {
		Status status;
		Lift::Status lift;
	};
	
	struct Estimator {
		Status last;
		Lift::Estimator lift;

		Estimator();

		void update(Time,Input,Output);
		Status_detail get()const;
		
		Countdown_timer timer;
	};
	Estimator estimator;
	
	struct Output_applicator{
		int pwm;

		explicit Output_applicator(int);

		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;
	
	enum class Goal{TOP,BOTTOM};
	
	explicit Can_grabber(int,int);
};

CMP1(Can_grabber::Input);
bool operator!=(Can_grabber const&,Can_grabber const&);
//bool operator<(Can_grabber::Input,Can_grabber::Input);
bool operator!=(Can_grabber::Estimator const&,Can_grabber::Estimator const&);
//std::ostream& operator<<(std::ostream&,Can_grabber::Input);
std::ostream& operator<<(std::ostream&,Can_grabber::Output);
std::ostream& operator<<(std::ostream&,Can_grabber::Goal);
std::ostream& operator<<(std::ostream&,Can_grabber::Status_detail);
std::ostream& operator<<(std::ostream&,Can_grabber::Status);
std::ostream& operator<<(std::ostream&,Can_grabber::Estimator);
std::ostream& operator<<(std::ostream&,Can_grabber::Output_applicator);
std::ostream& operator<<(std::ostream&,Can_grabber const&);
std::ostream& operator<<(std::ostream&,Can_grabber const&);
bool ready(Can_grabber::Status,Can_grabber::Goal);
std::set<Can_grabber::Status_detail> examples(Can_grabber::Status_detail*);
bool operator<(Can_grabber::Status_detail const&,Can_grabber::Status_detail const&);
bool operator!=(Can_grabber::Status_detail const&,Can_grabber::Status_detail const&);

std::set<Can_grabber::Status> examples(Can_grabber::Status*);
std::set<Can_grabber::Input> examples(Can_grabber::Input*);
std::set<Can_grabber::Output> examples(Can_grabber::Output*);
Can_grabber::Output control(Can_grabber::Status_detail const&,Can_grabber::Goal);
Can_grabber::Status status(Can_grabber::Status_detail const&);

#endif
