#ifndef KICKER_H
#define KICKER_H

#include<set>
#include "../util/interface.h"

struct Kicker{
	struct Input{
	};

	enum class Output{IN,OUT};
	typedef Output Goal;

	enum class Status_detail{IN,HALF_OUT,OUT};

	typedef Status_detail Status;

	struct Estimator{
		Output last_output;
		float output_start;
		Status_detail last;
		Estimator();

		void update(Time,Input,Output);
		Status_detail get()const;
	};
	Estimator estimator;
	
	struct Output_applicator{
		int kicker_address;

		explicit Output_applicator(int);

		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;
	
	explicit Kicker(int);
};

std::ostream& operator<<(std::ostream&,Kicker::Output);
std::ostream& operator<<(std::ostream&,Kicker::Status_detail);
std::ostream& operator<<(std::ostream&,Kicker::Estimator);
std::ostream& operator<<(std::ostream&,Kicker const&);

//std::set<Kicker::Output> examples(Kicker::Output*);
Kicker::Status status(Kicker::Status_detail const&);
Kicker::Output control(Kicker::Status,Kicker::Goal);
bool ready(Kicker::Status,Kicker::Goal);

#endif
