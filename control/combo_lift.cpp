#include "combo_lift.h"
#include "formal.h"

using namespace std;

#define COMPARE(NAME)\
	bool operator<(Combo_lift::NAME const& a,Combo_lift::NAME const& b){\
		if(a.can<b.can) return 1;\
		if(b.can<a.can) return 0;\
		return a.tote<b.tote;\
	}

#define OUT(NAME) ostream& operator<<(ostream& o,Combo_lift::NAME const& a){\
	return o<<"("<<a.can<<" "<<a.tote<<")";\
}

#define EXAMPLES(NAME)\
	set<Combo_lift::NAME> examples(Combo_lift::NAME*){\
		set<Combo_lift::NAME> r;\
		auto x=examples((Lift::NAME*)0);\
		for(auto elem:x){\
			for(auto y:x){\
				r|=Combo_lift::NAME{elem,y};\
			}\
		}\
		return r;\
	}

#define EQ(NAME) \
	bool operator==(Combo_lift::NAME const& a,Combo_lift::NAME const& b){\
		return a.can==b.can && a.tote==b.tote; \
	}

#define NEQ(NAME) bool operator!=(Combo_lift::NAME const& a,Combo_lift::NAME const& b){ return !(a==b); }

EQ(Input)
NEQ(Input)
COMPARE(Input)
OUT(Input)
EXAMPLES(Input)

Combo_lift::Input_reader::Input_reader(Combo_lift *p):parent(p){
	assert(p);
}

Combo_lift::Input Combo_lift::Input_reader::operator()(Robot_inputs a)const{
	return Input{parent->can.input_reader(a),parent->tote.input_reader(a)};
}

Robot_inputs Combo_lift::Input_reader::operator()(Robot_inputs all,Input in)const{
	all=parent->can.input_reader(all,in.can);
	all=parent->tote.input_reader(all,in.tote);
	return all;
}

EQ(Output)
NEQ(Output)
COMPARE(Output)
OUT(Output)
EXAMPLES(Output)

Combo_lift::Goal::Goal():can(Lift::Goal::stop()),tote(Lift::Goal::stop()),can_priority(1){}
Combo_lift::Goal::Goal(Lift::Goal a,Lift::Goal b,bool c):can(a),tote(b),can_priority(c){}

bool operator<(Combo_lift::Goal const& a,Combo_lift::Goal const& b){
	if(a.can<b.can) return 1;
	if(b.can<a.can) return 0;
	if(a.tote<b.tote) return 1;
	if(b.tote<a.tote) return 0;
	return a.can_priority<b.can_priority;
}

ostream& operator<<(ostream& o,Combo_lift::Goal const& a){
	return o<<"("<<a.can<<" "<<a.tote<<" "<<a.can_priority<<")";
}

set<Combo_lift::Goal> examples(Combo_lift::Goal*){
	set<Combo_lift::Goal> r;
	auto opt=examples((Lift::Goal*)0);
	for(auto a:opt) for(auto b:opt){
		r|={a,b,0};
		r|={a,b,1};
	}
	return r;
}

EQ(Status_detail)
NEQ(Status_detail)
COMPARE(Status_detail)
OUT(Status_detail)
EXAMPLES(Status_detail)

Combo_lift::Status::Status():can(Lift::Status::error()),tote(Lift::Status::error()){}

Combo_lift::Status::Status(Lift::Status a,Lift::Status b):can(a),tote(b){}

EQ(Status)
NEQ(Status)
COMPARE(Status)
OUT(Status)
EXAMPLES(Status)

Combo_lift::Estimator::Estimator(Combo_lift *a):parent(a){
	assert(a);
}

void Combo_lift::Estimator::update(Time time,Combo_lift::Input const& in,Combo_lift::Output const& out){
	parent->can.estimator.update(time,in.can,out.can);
	parent->tote.estimator.update(time,in.tote,out.tote);
}

Combo_lift::Status_detail Combo_lift::Estimator::get()const{
	return Combo_lift::Status_detail{
		parent->can.estimator.get(),
		parent->tote.estimator.get()
	};
}
bool operator==(Combo_lift::Estimator const&,Combo_lift::Estimator const&){ return 1; }
NEQ(Estimator)

Combo_lift::Output_applicator::Output_applicator(Combo_lift *p):parent(p){
	assert(p);
}

Robot_outputs Combo_lift::Output_applicator::operator()(Robot_outputs r,Combo_lift::Output out)const{
	r=parent->can.output_applicator(r,out.can);
	return parent->tote.output_applicator(r,out.tote);
}

Combo_lift::Output Combo_lift::Output_applicator::operator()(Robot_outputs a)const{
	return Combo_lift::Output{
		parent->can.output_applicator(a),
		parent->tote.output_applicator(a)
	};
}

Combo_lift::Combo_lift():input_reader(this),can(1),tote(0),estimator(this),output_applicator(this){}

bool operator==(Combo_lift const& a,Combo_lift const& b){
	return a.can==b.can && a.tote==b.tote;
}

bool operator!=(Combo_lift const& a,Combo_lift const& b){ return !(a==b); }

ostream& operator<<(ostream& o,Combo_lift const& a){
	o<<"Combo_lift(";
	o<<a.can<<a.tote;
	return o<<")";
}

static const double MAX_LIFT_HEIGHT=70;//made up

double status_height(Lift::Status_detail status){
	switch(status.type()){
		case Lift::Status_detail::Type::TOP: return MAX_LIFT_HEIGHT;
		case Lift::Status_detail::Type::BOTTOM: return 0;
		case Lift::Status_detail::Type::MID: return status.inches_off_ground();
		case Lift::Status_detail::Type::ERRORS: return 0;//this number is sort of arbitrary
		default: assert(0);
	}
}

//returns inches
double goal_height(Lift::Status_detail status,Lift::Goal goal){
	switch(goal.mode()){
		case Lift::Goal::Mode::GO_TO_HEIGHT: return goal.height()[2];
		case Lift::Goal::Mode::UP: return MAX_LIFT_HEIGHT;
		case Lift::Goal::Mode::DOWN: return 0;
		case Lift::Goal::Mode::STOP: return status_height(status);
		default: assert(0);
	}
}

Combo_lift::Goal interfere(Combo_lift::Status_detail status,Combo_lift::Goal goal){
	auto c=goal_height(status.can,goal.can);
	auto t=goal_height(status.tote,goal.tote);

	static const auto TOTE_LIFT_SPEED=10;//inches per second, this is made up
	static const auto CAN_LIFT_SPEED=27;//inches per second, this is made up

	if(goal.can.mode()==Lift::Goal::Mode::UP){
		goal.can=Lift::Goal::go_to_height(std::array<double,3>{60,61,65});
	}

	if(goal.can_priority){	
		//auto keepout_limit=max(status_height(status.can)-LIFT_SPEED,c);
		auto keepout_limit=status_height(status.can)-CAN_LIFT_SPEED;
		Lift::Goal limit_goal = (keepout_limit > .5) ? Lift::Goal::go_to_height(std::array<double,3>{keepout_limit-2,keepout_limit,keepout_limit+2}): Lift::Goal::down(); 
		
		return Combo_lift::Goal{
			goal.can,
			((keepout_limit<t)?limit_goal:goal.tote),
			goal.can_priority
		};
	}
	//auto keepout_limit=min(status_height(status.tote)+LIFT_SPEED,t);
	auto keepout_limit=status_height(status.tote)+TOTE_LIFT_SPEED;
	Lift::Goal limit_goal = (keepout_limit < 55.5) ? Lift::Goal::go_to_height(std::array<double,3>{keepout_limit-2,keepout_limit,keepout_limit+2}): Lift::Goal::go_to_height(std::array<double,3>{60,61,65}); 
	return Combo_lift::Goal{
		(keepout_limit>c)?limit_goal:goal.can,
		goal.tote,
		goal.can_priority
	};
}

Combo_lift::Output control(Combo_lift::Status_detail const& a,Combo_lift::Goal const& b){
	auto b2=b;
	//uncomment the following line to enable the non-crashing logic
	b2=interfere(a,b);
	return Combo_lift::Output{control(a.can,b2.can),control(a.tote,b2.tote)};
}

Combo_lift::Status status(Combo_lift::Status_detail const& a){
	return Combo_lift::Status{status(a.can),status(a.tote)};
}

bool ready(Combo_lift::Status const& a,Combo_lift::Goal const& b){
	return ready(a.can,b.can) && ready(a.tote,b.tote);
}

#ifdef COMBO_LIFT_TEST
int main(){
	Tester_mode m;
	m.check_outputs_exhaustive=0;
	tester(Combo_lift(), m);
}
#endif
