#ifndef COLLECTOR_TILT_H
#define COLLECTOR_TILT_H

#include<iosfwd>
#include "../util/countup_timer.h"

namespace Collector_tilt{
	enum Goal{GOAL_UP,GOAL_DOWN};//could also add an option for "don't care", but not sure if it will be used.
	std::ostream& operator<<(std::ostream&,Goal);

	enum Output{OUTPUT_UP,OUTPUT_DOWN,OUTPUT_NEITHER};
	std::ostream& operator<<(std::ostream&,Output);

	enum Status{STATUS_UP,STATUS_DOWN,STATUS_RAISING,STATUS_LOWERING,STATUS_UNKNOWN};
	std::ostream& operator<<(std::ostream&,Status);
	Maybe<Status> parse_status(std::string const&);

	class Estimator{
		Status est;
		Countup_timer timer;

		public:
		Estimator();
		void update(Time,Output);
		Status estimate()const;
		void out(std::ostream&)const;

		friend bool operator==(Estimator,Estimator);
	};
	bool operator==(Estimator,Estimator);
	bool operator!=(Estimator,Estimator);
	std::ostream& operator<<(std::ostream&,Estimator);

	Output control(Goal);
	bool ready(Status,Goal);
}

#endif
