#ifndef KICKER_H
#define KICKER_H

#include<set>
#include "../util/interface.h"

struct Kicker{
	struct Input{
	};

	typedef enum class Output{IN,OUT};

	enum class Status_detail{IN,HALF_OUT,OUT};

	typedef Status_detail Status;

	struct Estimator{
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

	enum class Mode{IN,OUT};
	
	explicit Kicker(int);
};
/*
std::ostream& operator<<(std::ostream&,Lift::Input const&);
bool operator<(Lift::Input const&,Lift::Input const&);
std::set<Lift::Input> examples(Lift::Input*);

std::set<Lift::Output> examples(Lift::Output*);

std::ostream& operator<<(std::ostream&,Lift::Status_detail::Type);
std::ostream& operator<<(std::ostream&,Lift::Status_detail const&);
bool operator<(Lift::Status_detail const&,Lift::Status_detail const&);
bool operator==(Lift::Status_detail const&,Lift::Status_detail const&);
std::set<Lift::Status_detail> examples(Lift::Status_detail*);

std::ostream& operator<<(std::ostream&,Lift::Goal::Mode);
std::ostream& operator<<(std::ostream&,Lift::Goal);
bool operator==(Lift::Goal,Lift::Goal);
bool operator!=(Lift::Goal,Lift::Goal);
bool operator<(Lift::Goal,Lift::Goal);
std::set<Lift::Goal> examples(Lift::Goal*);

std::ostream& operator<<(std::ostream&,Lift const&);

Lift::Status status(Lift::Status_detail const&);
Lift::Output control(Lift::Status_detail const&, Lift::Goal const&);
bool ready(Lift::Status,Lift::Goal::Mode);
*/
#endif
