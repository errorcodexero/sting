#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "pump.h"
#include "drivebase.h"
#include "lift.h"
#include "kicker.h"
#include "can_grabber.h"

#define TOPLEVEL_ITEMS\
	X(Lift,lift_can,Lift::Goal::stop())\
	X(Lift,lift_tote,Lift::Goal::stop())\
	X(Kicker,kicker,Kicker::Goal::IN)\
	X(Drivebase,drive,)\
	X(Pump,pump,Pump::Goal::AUTO)\
	X(Can_grabber,can_grabber,Can_grabber::Goal::TOP)

class Toplevel{
	public:
	struct Input{
		#define X(A,B,C) A::Input B;
		TOPLEVEL_ITEMS
		#undef X
	};

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
		Status();
		#define X(A,B,C) A::Status B;
		TOPLEVEL_ITEMS
		#undef X
	};

	class Estimator{
		//no estimate for collector
		Pump::Status pump;//for now just taking the sensor's measurement as gospel.
		
		public:
		Estimator();
		void update(Time,bool enabled,Output,Pump::Status,bool);
		Status estimate()const;
		void out(std::ostream&)const;

		friend bool operator==(Estimator,Estimator);
	};
};
std::ostream& operator<<(std::ostream&,Toplevel::Output);
std::ostream& operator<<(std::ostream&,Toplevel::Goal);
std::ostream& operator<<(std::ostream&,Toplevel::Status_detail const&);
bool operator==(Toplevel::Status,Toplevel::Status);
bool operator!=(Toplevel::Status,Toplevel::Status);
std::ostream& operator<<(std::ostream& o,Toplevel::Status);
//Maybe<Toplevel::Status> parse_status(std::string const&);

bool operator==(Toplevel::Estimator,Toplevel::Estimator);
bool operator!=(Toplevel::Estimator,Toplevel::Estimator);
std::ostream& operator<<(std::ostream& o,Toplevel::Estimator);
bool approx_equal(Toplevel::Estimator,Toplevel::Estimator);

Toplevel::Output control(Toplevel::Status,Toplevel::Goal);
bool ready(Toplevel::Status,Toplevel::Goal);
std::vector<std::string> not_ready(Toplevel::Status,Toplevel::Goal);
Toplevel::Status status(Toplevel::Status_detail const&);

#endif
