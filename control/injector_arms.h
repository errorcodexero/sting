#ifndef INJECTOR_ARMS_H
#define INJECTOR_ARMS_H

#include "../util/countup_timer.h"

namespace Injector_arms{
	enum Goal{GOAL_OPEN,GOAL_CLOSE,GOAL_X};
	std::ostream& operator<<(std::ostream&,Goal);

	enum Output{OUTPUT_OPEN,OUTPUT_CLOSE};
	std::ostream& operator<<(std::ostream&,Output);

	enum Status{STATUS_OPEN,STATUS_CLOSED,STATUS_OPENING,STATUS_CLOSING};
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
	};
	bool operator==(Estimator,Estimator);
	bool operator!=(Estimator,Estimator);
	std::ostream& operator<<(std::ostream&,Estimator);

	Output control(Status,Goal);
	bool ready(Status,Goal);
}

#endif
