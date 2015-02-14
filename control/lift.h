#ifndef LIFT_H
#define LIFT_H

#include<set>
#include "../util/interface.h"

struct Lift{
	struct Input{
		bool top,bottom;
		unsigned ticks;//this might not be the right thing here
	};

	typedef double Output;//motor power, in -1 to 1, assume 1=up

	class Status_detail{
		public:
		enum class Type{TOP,BOTTOM,MID,ERRORS};

		private:
		Status_detail();

		Type type_;
		double height;

		public:
		Type type()const;
		double inches_off_ground()const;

		static Status_detail top();
		static Status_detail bottom();
		static Status_detail mid(double);
		static Status_detail error();
	};

	typedef Status_detail Status;

	struct Estimator{
		Status_detail last;

		Estimator();

		void update(Time,Input,Output);
		Status_detail get()const;
	};
	Estimator estimator;

	struct Output_applicator{
		int can_address;

		explicit Output_applicator(int);

		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;

	enum class Goal{UP,DOWN,STOP};

	explicit Lift(int);
};

std::ostream& operator<<(std::ostream&,Lift::Input const&);
bool operator<(Lift::Input const&,Lift::Input const&);
std::set<Lift::Input> examples(Lift::Input*);

std::set<Lift::Output> examples(Lift::Output*);

std::ostream& operator<<(std::ostream&,Lift::Status_detail::Type);
std::ostream& operator<<(std::ostream&,Lift::Status_detail const&);
bool operator<(Lift::Status_detail const&,Lift::Status_detail const&);
bool operator==(Lift::Status_detail const&,Lift::Status_detail const&);
std::set<Lift::Status_detail> examples(Lift::Status_detail*);

std::ostream& operator<<(std::ostream&,Lift::Goal);
std::set<Lift::Goal> examples(Lift::Goal*);

std::ostream& operator<<(std::ostream&,Lift const&);

Lift::Status status(Lift::Status_detail const&);
Lift::Output control(Lift::Status_detail const&, Lift::Goal const&);
bool ready(Lift::Status,Lift::Goal);

#endif
