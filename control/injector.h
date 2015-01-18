#ifndef INJECTOR_H
#define INJECTOR_H

#include "../util/countup_timer.h"

namespace Injector{
	enum Status{IDLE,SHOOTING,RECOVERY};
	std::ostream& operator<<(std::ostream&,Status);

	enum Output{OUTPUT_UP,OUTPUT_DOWN,OUTPUT_VENT};
	std::ostream& operator<<(std::ostream&,Output);

	enum Goal{START,WAIT,X};
	std::ostream& operator<<(std::ostream&,Goal);

	class Estimator{
		public:
		enum Location{GOING_UP,UP,GOING_DOWN,DOWN_VENT,DOWN_IDLE,X};

		private:
		Location location;
		Countup_timer timer;

		public:
		Estimator();
		void update(Time,Output,bool downsensor);
		Location estimate()const;
		Status status()const;
		void out(std::ostream&)const;

		friend bool operator==(Estimator,Estimator);
	};
	bool operator==(Estimator,Estimator);
	bool operator!=(Estimator,Estimator);
	std::ostream& operator<<(std::ostream&,Estimator::Location);
	std::ostream& operator<<(std::ostream&,Estimator);
	Maybe<Estimator::Location> parse_location(std::string const&);

	Status location_to_status(Estimator::Location);
	Output control(Estimator::Location,Goal);

	bool ready(Estimator::Location,Goal);
	bool ready(Status,Goal);
}

#endif
