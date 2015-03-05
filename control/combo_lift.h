#ifndef COMBO_LIFT_H
#define COMBO_LIFT_H

#include "lift.h"

struct Combo_lift{
	struct Input{
		Lift::Input can,tote;
	};

	class Input_reader{
		Combo_lift *parent;

		public:
		explicit Input_reader(Combo_lift*);
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};
	Input_reader input_reader;

	struct Output{
		Lift::Output can,tote;
	};

	struct Goal{
		Lift::Goal can,tote;
		bool can_priority;

		Goal();
		Goal(Lift::Goal,Lift::Goal,bool);
	};

	struct Status_detail{
		Lift::Status_detail can,tote;
	};

	struct Status{
		Lift::Status can,tote;

		Status();
		Status(Lift::Status,Lift::Status);
	};

	Lift can,tote;

	class Estimator{
		Combo_lift *parent;

		public:
		Estimator(Combo_lift*);
		void update(Time,Input const&,Output const&);
		Status_detail get()const;
	};
	Estimator estimator;

	class Output_applicator{
		Combo_lift *parent;

		public:
		Output_applicator(Combo_lift*);
		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;

	Combo_lift();
};

bool operator==(Combo_lift::Input const&,Combo_lift::Input const&);
bool operator!=(Combo_lift::Input const&,Combo_lift::Input const&);
bool operator<(Combo_lift::Input const&,Combo_lift::Input const&);
std::ostream& operator<<(std::ostream&,Combo_lift::Input const&);

bool operator!=(Combo_lift::Output const&,Combo_lift::Output const&);
bool operator<(Combo_lift::Output const&,Combo_lift::Output const&);
std::ostream& operator<<(std::ostream&,Combo_lift::Output const&);

bool operator<(Combo_lift::Goal const&,Combo_lift::Goal const&);
std::ostream& operator<<(std::ostream&,Combo_lift::Goal const&);

bool operator<(Combo_lift::Status_detail const&,Combo_lift::Status_detail const&);
bool operator!=(Combo_lift::Status_detail const&,Combo_lift::Status_detail const&);
std::ostream& operator<<(std::ostream&,Combo_lift::Status_detail const&);

bool operator<(Combo_lift::Status const&,Combo_lift::Status const&);
bool operator!=(Combo_lift::Status const&,Combo_lift::Status const&);
std::ostream& operator<<(std::ostream&,Combo_lift::Status const&);

bool operator!=(Combo_lift::Estimator const&,Combo_lift::Estimator const&);

bool operator!=(Combo_lift const&,Combo_lift const&);
std::ostream& operator<<(std::ostream&,Combo_lift const&);

Combo_lift::Output control(Combo_lift::Status_detail const&,Combo_lift::Goal const&);
Combo_lift::Status status(Combo_lift::Status_detail const&);
bool ready(Combo_lift::Status const&,Combo_lift::Goal const&);

#endif
