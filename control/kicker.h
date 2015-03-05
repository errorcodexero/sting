#ifndef KICKER_H
#define KICKER_H

#include<set>
#include "../util/interface.h"
#include "../util/quick.h"

struct Kicker{
	#define KICKER_INPUT(X)
	DECLARE_STRUCT(Input,KICKER_INPUT)
	/*struct Input{
	};*/

	struct Input_reader{
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};
	Input_reader input_reader;

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

std::ostream& operator<<(std::ostream&,Kicker::Input const&);
std::ostream& operator<<(std::ostream&,Kicker::Output);
std::ostream& operator<<(std::ostream&,Kicker::Status_detail);
std::ostream& operator<<(std::ostream&,Kicker::Estimator);
std::ostream& operator<<(std::ostream&,Kicker const&);

bool operator!=(Kicker const&,Kicker const&);
bool operator<(Kicker::Input,Kicker::Input);
bool operator==(Kicker::Input,Kicker::Input);
bool operator!=(Kicker::Input,Kicker::Input);
bool operator==(Kicker::Estimator const&,Kicker::Estimator const&);
bool operator!=(Kicker::Estimator const&,Kicker::Estimator const&);

//std::set<Kicker::Output> examples(Kicker::Output*);
Kicker::Status status(Kicker::Status_detail const&);
Kicker::Output control(Kicker::Status,Kicker::Goal);
bool ready(Kicker::Status,Kicker::Goal);

#endif
