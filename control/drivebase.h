#ifndef DRIVEBASE_H
#define DRIVEBASE_H

#include <iosfwd>
#include <set>
#include "../util/interface.h"

struct Drivebase{
	struct Input{};
	
	struct Output{
		double l,r,c; //power levels for the left, right, and center wheels

		Output();
		Output(double,double,double);
	};

	struct Status{};
	typedef Status Status_detail;

	struct Estimator{
		void update(Time,Input,Output);
		Status_detail get()const;
	};
	Estimator estimator;

	struct Output_applicator{
		Robot_outputs operator()(Robot_outputs,Output)const;
		Output operator()(Robot_outputs)const;
	};
	Output_applicator output_applicator;

	struct Goal{
		//speeds
		double x,y,theta;
	};
};

std::ostream& operator<<(std::ostream&,Drivebase::Input const&);
bool operator<(Drivebase::Input const&,Drivebase::Input const&);
std::set<Drivebase::Input> examples(Drivebase::Input*);

std::ostream& operator<<(std::ostream&,Drivebase::Output const&);
bool operator<(Drivebase::Output const&,Drivebase::Output const&);
bool operator==(Drivebase::Output const&,Drivebase::Output const&);
bool operator!=(Drivebase::Output const&,Drivebase::Output const&);
std::set<Drivebase::Output> examples(Drivebase::Output*);

std::ostream& operator<<(std::ostream&,Drivebase::Status const&);
bool operator<(Drivebase::Status,Drivebase::Status);
bool operator==(Drivebase::Status,Drivebase::Status);
bool operator!=(Drivebase::Status,Drivebase::Status);
std::set<Drivebase::Status> examples(Drivebase::Status*);

std::ostream& operator<<(std::ostream&,Drivebase::Goal const&);
std::set<Drivebase::Goal> examples(Drivebase::Goal*);
bool operator<(Drivebase::Goal const&,Drivebase::Goal const&);

Drivebase::Status status(Drivebase::Status_detail);
Drivebase::Output control(Drivebase::Status_detail,Drivebase::Goal);
bool ready(Drivebase::Status,Drivebase::Goal);

std::ostream& operator<<(std::ostream&,Drivebase const&);

#endif
