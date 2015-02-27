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

namespace Toplevel{
	struct Output{
		Output();
		#define X(A,B,C) A::Output B;
		TOPLEVEL_ITEMS
		#undef X
	};
	std::ostream& operator<<(std::ostream&,Output);

	struct Subgoals{
		Subgoals();
		#define X(A,B,C) A::Goal B;
		TOPLEVEL_ITEMS
		#undef X
		bool dummy[0];
	};
	std::ostream& operator<<(std::ostream&,Subgoals);

	struct Status{
		Status();
		#define X(A,B,C) A::Status B;
		TOPLEVEL_ITEMS
		#undef X
	};
	bool operator==(Status,Status);
	bool operator!=(Status,Status);
	std::ostream& operator<<(std::ostream& o,Status);
	Maybe<Status> parse_status(std::string const&);

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
	bool operator==(Estimator,Estimator);
	bool operator!=(Estimator,Estimator);
	std::ostream& operator<<(std::ostream& o,Estimator);
	bool approx_equal(Estimator,Estimator);

	Output control(Status,Subgoals);
	bool ready(Status,Subgoals);
	std::vector<std::string> not_ready(Status,Subgoals);
}

#endif
