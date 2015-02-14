#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "shooter_wheels.h"
#include "pump.h"
#include "drivebase.h"
#include "lift.h"

namespace Toplevel{
	struct Output{
		Output();

		Shooter_wheels::Output shooter_wheels;
		Pump::Output pump;
	};
	std::ostream& operator<<(std::ostream&,Output);

	struct Subgoals{
		Subgoals();
		Lift::Goal lift_goal_tote;
		Lift::Goal lift_goal_can;
		Shooter_wheels::Goal shooter_wheels;
		Pump::Goal pump;
		Drivebase::Goal drive;
		//pump omitted because it currently only has one goal.
	};
	std::ostream& operator<<(std::ostream&,Subgoals);

	struct Status{
		Status();

		Shooter_wheels::Status shooter_wheels;
		Pump::Status pump;
		float orientation;
	};
	bool operator==(Status,Status);
	bool operator!=(Status,Status);
	std::ostream& operator<<(std::ostream& o,Status);
	Maybe<Status> parse_status(std::string const&);

	class Estimator{
		//no estimate for collector
		Shooter_wheels::Status shooter_wheels;
		Pump::Status pump;//for now just taking the sensor's measurement as gospel.
		float orientation;
		
		public:
		Estimator();
		void update(Time,bool enabled,Output,Pump::Status,float orientation,Shooter_wheels::Status,bool);
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
