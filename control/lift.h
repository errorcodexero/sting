#ifndef LIFT_H
#define LIFT_H

#include<set>
#include<utility>
#include "../util/countdown_timer.h"
#include "../util/interface.h"
#include "../util/quick.h"

struct Lift{
	#define LIFT_INPUT(X) X(bool,top) X(bool,bottom) X(int,ticks) X(double,current)
	struct Input{
		bool top,bottom;
		int ticks;
		double current;
	};

	struct Input_reader{
		int can_address;

		explicit Input_reader(int);
		Input operator()(Robot_inputs)const;
		Robot_inputs operator()(Robot_inputs,Input)const;
	};
	Input_reader input_reader;

	typedef double Output;//motor power, in -1 to 1, assume 1=up

	class Status_detail{
		public:
		enum class Type{TOP,BOTTOM,MID,ERRORS};
		std::pair<bool,bool> reached_ends;//first is the bottom and second is the top
		bool stalled;
		
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
		Maybe_inline<int> top,bottom;

		Maybe_inline<double> range()const;

		public:
		Estimator();

		Countdown_timer stall_timer; 
		float timer_start_height;
		
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

	class Goal{
		public:
		enum class Mode{GO_TO_HEIGHT,UP,DOWN,STOP};
		
		private:
		Goal();
		
		Mode mode_;
		double height_;
		
		public:
		Mode mode()const;
		double height()const;
		
		static Goal go_to_height(double);
		static Goal up();
		static Goal down();
		static Goal stop();
	};
	
	explicit Lift(int);
};

CMP1(Lift::Input)
std::set<Lift::Input> examples(Lift::Input*);

std::set<Lift::Output> examples(Lift::Output*);

std::ostream& operator<<(std::ostream&,Lift::Status_detail::Type);
std::ostream& operator<<(std::ostream&,Lift::Status_detail const&);
bool operator<(Lift::Status_detail const&,Lift::Status_detail const&);
bool operator==(Lift::Status_detail const&,Lift::Status_detail const&);
bool operator!=(Lift::Status_detail const&,Lift::Status_detail const&);
std::set<Lift::Status_detail> examples(Lift::Status_detail*);

std::ostream& operator<<(std::ostream&,Lift::Goal::Mode);
std::ostream& operator<<(std::ostream&,Lift::Goal);
bool operator==(Lift::Goal,Lift::Goal);
bool operator!=(Lift::Goal,Lift::Goal);
bool operator<(Lift::Goal,Lift::Goal);
std::set<Lift::Goal> examples(Lift::Goal*);

std::ostream& operator<<(std::ostream&,Lift const&);

bool operator!=(Lift::Estimator const&,Lift::Estimator const&);
bool operator!=(Lift const&,Lift const&);
bool operator==(Lift const&,Lift const&);

Lift::Status status(Lift::Status_detail const&);
Lift::Output control(Lift::Status_detail const&, Lift::Goal const&);
bool ready(Lift::Status,Lift::Goal);

#endif
