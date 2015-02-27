#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "pump.h"
#include "drivebase.h"
#include "lift.h"
#include "kicker.h"

#define TOPLEVEL_ITEMS\
	X(Lift,lift_can)\
	X(Lift,lift_tote)\
	X(Kicker,kicker)\
	X(Drivebase,drive)

namespace Toplevel{
	struct Output{
		Output();
		#define X(A,B) A::Output B;
		TOPLEVEL_ITEMS
		#undef X
		Pump::Output pump;
	};
	std::ostream& operator<<(std::ostream&,Output);

	struct Subgoals{
		Subgoals();
		#define X(A,B) A::Goal B;
		TOPLEVEL_ITEMS
		#undef X
		Pump::Goal pump;
	};
	std::ostream& operator<<(std::ostream&,Subgoals);

	struct Status{
		Status();
		#define X(A,B) A::Status B;
		TOPLEVEL_ITEMS
		#undef X
		Pump::Status pump;
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
