#include "combo_lift.h"
#include "formal.h"

using namespace std;

#define COMPARE(NAME)\
	bool operator<(Combo_lift::NAME const& a,Combo_lift::NAME const& b){\
		if(a.can<b.can) return 1;\
		if(b.can<a.can) return 0;\
		return a.tote<b.tote;\
	}
COMPARE(Input)

#define OUT(NAME) ostream& operator<<(ostream& o,Combo_lift::NAME const& a){\
	return o<<"("<<a.can<<" "<<a.tote<<")";\
}
OUT(Input)

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
EXAMPLES(Input)

#define EQ(NAME) \
	bool operator==(Combo_lift::NAME const& a,Combo_lift::NAME const& b){\
		return a.can==b.can && a.tote==b.tote; \
	}
#define NEQ(NAME) bool operator!=(Combo_lift::NAME const& a,Combo_lift::NAME const& b){ return !(a==b); }
EQ(Output)

bool operator!=(Combo_lift::Output const& a,Combo_lift::Output const& b){ return !(a==b); }

COMPARE(Output)
OUT(Output)
EXAMPLES(Output)

Combo_lift::Goal::Goal():can(Lift::Goal::stop()),tote(Lift::Goal::stop()){}
Combo_lift::Goal::Goal(Lift::Goal a,Lift::Goal b):can(a),tote(b){}
COMPARE(Goal)
OUT(Goal)
EXAMPLES(Goal)

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

Combo_lift::Combo_lift():can(1),tote(0),estimator(this),output_applicator(this){}

bool operator==(Combo_lift const& a,Combo_lift const& b){
	return a.can==b.can && a.tote==b.tote;
}

bool operator!=(Combo_lift const& a,Combo_lift const& b){ return !(a==b); }

ostream& operator<<(ostream& o,Combo_lift const& a){
	o<<"Combo_lift(";
	o<<a.can<<a.tote;
	return o<<")";
}

Combo_lift::Output control(Combo_lift::Status_detail const& a,Combo_lift::Goal const& b){
	return Combo_lift::Output{control(a.can,b.can),control(a.tote,b.tote)};
}

Combo_lift::Status status(Combo_lift::Status_detail const& a){
	return Combo_lift::Status{status(a.can),status(a.tote)};
}

bool ready(Combo_lift::Status const& a,Combo_lift::Goal const& b){
	return ready(a.can,b.can) && ready(a.tote,b.tote);
}

#ifdef COMBO_LIFT_TEST
int main(){
	tester(Combo_lift());
}
#endif
