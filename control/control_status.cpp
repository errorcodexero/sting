#include "control_status.h"
#include<iostream>
#include<cassert>
#include "../util/util.h"

using namespace std;

namespace Control_status{
	ostream& operator<<(ostream& o,Control_status c){
		#define X(name) if(c==name) return o<<""#name;
		X(AUTO_SPIN_UP)
		X(AUTO_FIRE)
		X(AUTO_TO_COLLECT)
		X(AUTO_COLLECT)
		X(AUTO_SPIN_UP2)
		X(AUTO_FIRE2)
		X(A2_SPIN_UP)
		X(A2_FIRE)
		X(A2_TO_COLLECT)
		X(A2_COLLECT)
		X(A2_SPIN_UP2)
		X(A2_FIRE2)
		X(A2_MOVE)
		X(DRIVE_W_BALL)
		X(DRIVE_WO_BALL)
		X(COLLECT)
		X(SHOOT_HIGH_PREP)
		X(SHOOT_HIGH)
		X(SHOOT_HIGH_WHEN_READY)
		X(TRUSS_TOSS_PREP)
		X(TRUSS_TOSS)
		X(TRUSS_TOSS_WHEN_READY)
		X(EJECT_PREP)
		X(EJECT)
		X(EJECT_WHEN_READY)
		X(AUTO_SHOT_PREP)
		X(AUTO_SHOT)
		X(AUTO_SHOT_WHEN_READY)
		X(CATCH)
		//X(SHOOT_LOW)
		#undef X
		assert(0);
}

	vector<Control_status> all(){
		vector<Control_status> r;
		#define X(name) r.push_back(name);
		X(AUTO_SPIN_UP)
		X(AUTO_FIRE)
		X(AUTO_TO_COLLECT)
		X(AUTO_COLLECT)
		X(AUTO_SPIN_UP2)
		X(AUTO_FIRE2)
		X(A2_SPIN_UP)
		X(A2_FIRE)
		X(A2_TO_COLLECT)
		X(A2_COLLECT)
		X(A2_SPIN_UP2)
		X(A2_FIRE2)
		X(A2_MOVE)
		X(DRIVE_W_BALL)
		X(DRIVE_WO_BALL)
		X(COLLECT)
		X(SHOOT_HIGH_PREP)
		X(SHOOT_HIGH)
		X(SHOOT_HIGH_WHEN_READY)
		X(TRUSS_TOSS_PREP)
		X(TRUSS_TOSS)
		X(TRUSS_TOSS_WHEN_READY)
		X(EJECT_PREP)
		X(EJECT)
		X(EJECT_WHEN_READY)
		X(AUTO_SHOT_PREP)
		X(AUTO_SHOT)
		X(AUTO_SHOT_WHEN_READY)
		X(CATCH)
		//X(SHOOT_LOW)
		#undef X
		return r;
	}

	Maybe<Control_status> parse(string const& s){
		return parse_enum<Control_status>(all(),s);
	}

	bool autonomous(Control_status s){
		return s==AUTO_SPIN_UP || s==AUTO_FIRE || s==AUTO_TO_COLLECT || 
			s==AUTO_COLLECT || s==AUTO_SPIN_UP2 || s==AUTO_FIRE2 || 
			s==A2_SPIN_UP || s==A2_FIRE || s==A2_TO_COLLECT || 
			s==A2_COLLECT || s==A2_SPIN_UP2 || s==A2_FIRE2 || 
			s==A2_MOVE;
	}

	bool teleop(Control_status s){
		return s==DRIVE_W_BALL || s==DRIVE_WO_BALL || s==COLLECT || 
			s==SHOOT_HIGH_PREP || s==SHOOT_HIGH || s==SHOOT_HIGH_WHEN_READY || 
			s==TRUSS_TOSS_PREP || s==TRUSS_TOSS || s==TRUSS_TOSS_WHEN_READY ||
			s==EJECT_PREP || s==EJECT || s==EJECT_WHEN_READY ||
			s==AUTO_SHOT_PREP||s==AUTO_SHOT||s==AUTO_SHOT_WHEN_READY||
			s==CATCH;
	}
}

#ifdef CONTROL_STATUS_TEST
int main(){
	for(auto a:Control_status::all()){
		cout<<a<<" "<<autonomous(a)<<"\n";
		int x=0;
		if(autonomous(a)) x++;
		if(teleop(a)) x++;
		assert(x==1);

		assert(a==Control_status::parse(as_string(a)));
	}
}
#endif
