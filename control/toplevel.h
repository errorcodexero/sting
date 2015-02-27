#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "pump.h"
#include "drivebase.h"
#include "lift.h"
#include "kicker.h"

namespace Toplevel{
	struct Output{
		Output();
		Lift::Output lift_can;
		Lift::Output lift_tote;
		Kicker::Output kicker;
		Drivebase::Output drive;
		Pump::Output pump;
	};
	std::ostream& operator<<(std::ostream&,Output);

	struct Subgoals{
		Subgoals();
		Lift::Goal lift_goal_tote;
		Lift::Goal lift_goal_can;
		Kicker::Goal kicker;
		Pump::Goal pump;
		Drivebase::Goal drive;
		//pump omitted because it currently only has one goal.
	};
	std::ostream& operator<<(std::ostream&,Subgoals);

	struct Status{
		Status();
		Drivebase::Status_detail drive_status;
		Lift::Status lift_status_can;
		Lift::Status lift_status_tote;
		Kicker::Status kicker;
		Pump::Status pump;
		float orientation;
	};
	bool operator==(Status,Status);
	bool operator!=(Status,Status);
	std::ostream& operator<<(std::ostream& o,Status);
	Maybe<Status> parse_status(std::string const&);

	class Estimator{
		//no estimate for collector
		Pump::Status pump;//for now just taking the sensor's measurement as gospel.
		float orientation;
		
		public:
		Estimator();
		void update(Time,bool enabled,Output,Pump::Status,float orientation,bool);
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
