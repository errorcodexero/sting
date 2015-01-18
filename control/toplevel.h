#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include<vector>
#include "collector.h"
#include "collector_tilt.h"
#include "injector.h"
#include "injector_arms.h"
#include "shooter_wheels.h"
#include "ejector.h"
#include "pump.h"
#include "holonomic.h"

namespace Toplevel{
	struct Output{
		Output();

		Collector_mode collector;
		Collector_tilt::Output collector_tilt;
		Injector::Output injector;
		Injector_arms::Output injector_arms;
		Ejector::Output ejector;
		Shooter_wheels::Output shooter_wheels;
		Pump::Output pump;
		Drive_motors drive;
	};
	std::ostream& operator<<(std::ostream&,Output);

	struct Subgoals{
		Subgoals();

		Collector_mode collector;
		Collector_tilt::Goal collector_tilt;
		Injector::Goal injector;
		Injector_arms::Goal injector_arms;
		Ejector::Goal ejector;
		Shooter_wheels::Goal shooter_wheels;
		Pump::Goal pump;
		Drive_goal drive;
		//pump omitted because it currently only has one goal.
	};
	std::ostream& operator<<(std::ostream&,Subgoals);

	struct Status{
		Status();

		//collector has no state
		Collector_tilt::Status collector_tilt;
		Injector::Estimator::Location injector;
		Injector_arms::Status injector_arms;
		Ejector::Estimator::Location ejector;
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
		Collector_tilt::Estimator collector_tilt;
		Injector::Estimator injector;
		Injector_arms::Estimator injector_arms;
		Ejector::Estimator ejector;
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
