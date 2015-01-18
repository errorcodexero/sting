#ifndef PUMP_ADVANCED_H
#define PUMP_ADVANCED_H

#include "../util/maybe.h"
#include "collector_tilt.h"
#include "injector.h"
#include "injector_arms.h"
#include "ejector.h"
#include "pump.h"

namespace Pump_advanced{
	typedef double PSI;

	class Estimator{
		Time last_time;
		Pump::Status last_reading;
		PSI psi;

		Maybe<Collector_tilt::Output> collector_tilt;
		Maybe<Injector::Output> injector;
		Maybe<Injector_arms::Output> injector_arms;
		Maybe<Ejector::Output> ejector;

		public:
		Estimator();

		void update(Time,Collector_tilt::Output,Injector::Output,Injector_arms::Output,Ejector::Output,Pump::Output,Pump::Status);
		PSI estimate()const;

		friend std::ostream& operator<<(std::ostream&,Estimator);
	};
	std::ostream& operator<<(std::ostream&,Estimator);

	Pump::Output control(PSI);
}

#endif
