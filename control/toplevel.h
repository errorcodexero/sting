#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "pump.h"
#include "drivebase.h"
#include "combo_lift.h"
#include "kicker.h"
#include "can_grabber.h"
#include "tote_sensors.h"

#define TOPLEVEL_ITEMS\
	X(Combo_lift,combo_lift,Combo_lift::Goal{})\
	X(Kicker,kicker,Kicker::Goal::IN)\
	X(Drivebase,drive,)\
	X(Pump,pump,Pump::Goal::AUTO)\
	X(Can_grabber,can_grabber,Can_grabber::Goal::TOP)\
	X(Tote_sensors,tote_sensors,Tote_sensors::Goal{})

class Toplevel{
	public:

	#define X(A,B,C) A B;
	TOPLEVEL_ITEMS
	#undef X

	struct Input{
		#define X(A,B,C) A::Input B;
		TOPLEVEL_ITEMS
		#undef X
	};

	class Input_reader{
		Toplevel *parent;

		public:
		explicit Input_reader(Toplevel*);
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};
	Input_reader input_reader;

	struct Output{
		Output();
		#define X(A,B,C) A::Output B;
		TOPLEVEL_ITEMS
		#undef X
	};

	struct Goal{
		Goal();
		#define X(A,B,C) A::Goal B;
		TOPLEVEL_ITEMS
		#undef X
		bool dummy[0];
	};

	struct Status_detail{
		#define X(A,B,C) A::Status_detail B;
		TOPLEVEL_ITEMS
		#undef X
	};
	
	struct Status{
		Status();//todo:remove this
		#define X(A,B,C) A::Status B;
		TOPLEVEL_ITEMS
		#undef X
	};

	class Estimator{
		Toplevel *parent;
		
		public:
		Estimator(Toplevel*);
		void update(Time,Input,Output);
		Status_detail get()const;
		void out(std::ostream&)const;

		friend bool operator==(Estimator,Estimator);
	};
	Estimator estimator;

	class Output_applicator{
		Toplevel *parent;

		public:
		Output_applicator(Toplevel*);

		Robot_outputs operator()(Robot_outputs,Output const&)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;

	Toplevel();
};
bool operator<(Toplevel::Output const&,Toplevel::Output const&);
bool operator==(Toplevel::Output const&,Toplevel::Output const&);
bool operator!=(Toplevel::Output const&,Toplevel::Output const&);
std::ostream& operator<<(std::ostream&,Toplevel::Output);
std::ostream& operator<<(std::ostream&,Toplevel::Goal);
std::ostream& operator<<(std::ostream&,Toplevel::Status_detail const&);
bool operator<(Toplevel::Status const&,Toplevel::Status const&);
bool operator==(Toplevel::Status,Toplevel::Status);
bool operator!=(Toplevel::Status,Toplevel::Status);
std::ostream& operator<<(std::ostream& o,Toplevel::Status);
//Maybe<Toplevel::Status> parse_status(std::string const&);
bool operator<(Toplevel::Status_detail const&,Toplevel::Status_detail const&);
bool operator==(Toplevel::Status_detail const&,Toplevel::Status_detail const&);
std::set<Toplevel::Status_detail> examples(Toplevel::Status_detail*);
std::set<Toplevel::Goal> examples(Toplevel::Goal*);
std::set<Toplevel::Output> examples(Toplevel::Output*);
std::set<Toplevel::Status> examples(Toplevel::Status*);
std::set<Toplevel::Input> examples(Toplevel::Input*);
bool operator<(Toplevel::Input const&,Toplevel::Input const&);
bool operator==(Toplevel::Input const&,Toplevel::Input const&);

bool operator==(Toplevel::Estimator,Toplevel::Estimator);
bool operator!=(Toplevel::Estimator,Toplevel::Estimator);
std::ostream& operator<<(std::ostream& o,Toplevel::Estimator);
bool approx_equal(Toplevel::Estimator,Toplevel::Estimator);

Toplevel::Output control(Toplevel::Status_detail,Toplevel::Goal);
bool ready(Toplevel::Status,Toplevel::Goal);
std::vector<std::string> not_ready(Toplevel::Status,Toplevel::Goal);
Toplevel::Status status(Toplevel::Status_detail const&);

bool operator==(Toplevel const&,Toplevel const&);
bool operator!=(Toplevel const&,Toplevel const&);
std::ostream& operator<<(std::ostream&,Toplevel const&);

#endif
