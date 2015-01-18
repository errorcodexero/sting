#include "octocanum.h"
#include<iostream>
#include<math.h>
#include<algorithm>
#include"../util/util.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Traction_output a){
	return o<<"Traction_output("<<a.l<<","<<a.r<<")";
}

Traction_mode_goal::Traction_mode_goal():y(0),theta(0){}
Traction_mode_goal::Traction_mode_goal(double a,double b):y(a),theta(b){}

std::ostream& operator<<(std::ostream& o,Traction_mode_goal g){
	return o<<"Traction_mode_goal("<<g.y<<" "<<g.theta<<")";
}

Traction_output calculate(Traction_mode_goal g){
	Traction_output r;
	r.l=g.y+g.theta;
	r.r=g.y-g.theta;
	double m=max(fabs(r.l),fabs(r.r));
	if(m>1){
		r.l/=m;
		r.r/=m;
	}
	return r;
}

Mecanum_output::Mecanum_output():lf(0),rf(0),lr(0),rr(0){}
Mecanum_output::Mecanum_output(double a,double b,double c,double d):lf(a),rf(b),lr(c),rr(d){}

std::ostream& operator<<(std::ostream& o,Mecanum_output a){
	return o<<"Mecanum_output("<<a.lf<<" "<<a.rf<<" "<<a.lr<<" "<<a.rr<<")";
}

Mecanum_goal::Mecanum_goal():x(0),y(0),theta(0){}
Mecanum_goal::Mecanum_goal(double a,double b,double c):x(a),y(b),theta(c){}

std::ostream& operator<<(std::ostream& o,Mecanum_goal g){
	return o<<"Mecanum_goal("<<g.x<<" "<<g.y<<" "<<g.theta<<")";
}

Mecanum_output calculate(Mecanum_goal m){
	Mecanum_output r;
	//this is untested and probably wrong.  Should look at the existing implementation for reference.
	r.lf=m.y+m.x+m.theta;
	r.rf=m.y-m.x-m.theta;
	r.lr=m.y-m.x+m.theta;
	r.rr=m.y+m.x-m.theta;
	double lim=max(max(fabs(r.lf),fabs(r.rf)),max(fabs(r.lr),fabs(r.rr)));
	if(lim>1){
		r.lf/=lim;
		r.rf/=lim;
		r.lr/=lim;
		r.rr/=lim;
	}
	return r;
}

Octocanum_output::Octocanum_output():traction_mode(1){}
Octocanum_output::Octocanum_output(bool a,Mecanum_output b):traction_mode(a),wheels(b){}

std::ostream& operator<<(std::ostream& o,Octocanum_output a){
	o<<"Octocanum_output("<<a.wheels;
	return o<<" "<<a.traction_mode<<")";
}

Octocanum_goal::Octocanum_goal():traction_mode(1){}

Octocanum_goal::Octocanum_goal(Traction_mode_goal t1):traction_mode(1),t(t1){}

Octocanum_goal::Octocanum_goal(Mecanum_goal g):traction_mode(0),mec(g){}

Octocanum_goal& Octocanum_goal::operator=(Traction_mode_goal a){
	traction_mode=1;
	t=a;
	return *this;
}

Octocanum_goal& Octocanum_goal::operator=(Mecanum_goal a){
	traction_mode=0;
	mec=a;
	return *this;
}

std::ostream& operator<<(std::ostream& o,Octocanum_goal g){
	o<<"Octocanum_goal(";
	o<<g.traction_mode<<g.mec<<g.t;
	o<<")";
	return o;
}

Mecanum_output as_mecanum(Traction_output t){
	return Mecanum_output(t.l,t.r,t.l,t.r);
}

std::ostream& operator<<(std::ostream& o,Octocanum_mode m){
	switch(m){
		#define X(name) case name: return o<<""#name;
		X(TRACTION)
		X(MECH)
		X(TO_TRACTION)
		X(TO_MECH)
		#undef X
		default: return o<<"ERROR";
	}
}

double portion_done(Octocanum_mode mode,bool /*goal_traction*/,Time elapsed){
	switch(mode){
		case TO_TRACTION: return elapsed/.5;
		case TO_MECH: return elapsed/.75;
		default: return 1;
	}
}

Octocanum_mode next_mode(Octocanum_mode mode,bool goal_traction,Time elapsed){
	bool done=portion_done(mode,goal_traction,elapsed)>=1;
	switch(mode){
		default:
		case TRACTION:
			return goal_traction?TRACTION:TO_MECH;
		case MECH:
			return goal_traction?TO_TRACTION:MECH;
		case TO_TRACTION:
			if(goal_traction){
				return done?TRACTION:TO_TRACTION;
			}else{
				return TO_MECH;
			}
		case TO_MECH:
			if(goal_traction){
				return TO_TRACTION;
			}else{
				return done?MECH:TO_MECH;
			}
	}
}

Octocanum_output calculate(Octocanum_mode mode,Octocanum_goal goal){
	switch(mode){
		default:
		case TRACTION:
		case TO_TRACTION:
			return Octocanum_output(1,as_mecanum(calculate(goal.t)));
		case TO_MECH:
			return Octocanum_output(0,as_mecanum(calculate(goal.t)));
		case MECH:
			return Octocanum_output(0,calculate(goal.mec));
	}
}

Octocanum_state::Octocanum_state():mode(TRACTION),last_change_time(0){}

std::ostream& operator<<(std::ostream& o,Octocanum_state a){
	return o<<"Octocanum_state("<<a.mode<<" "<<a.last_change_time<<")";
}

//not tested yet.
double linear_to_sine(double x){
	if(x<0) return 0;
	if(x>1) return 1;
	return 1-(cos(M_PI*x)+1)/2;
}

std::pair<Octocanum_state,Octocanum_output> run(Octocanum_state state,Octocanum_goal goal,Time now){
	Octocanum_mode n=next_mode(state.mode,goal.traction_mode,now-state.last_change_time);
	if(n!=state.mode){
		state.last_change_time=now;
		state.mode=n;
	}
	return std::make_pair(state,calculate(state.mode,goal));
}

#ifdef OCTOCANUM_TEST
int main(){
	Octocanum_goal g(Mecanum_goal(1,1,1));
	Octocanum_state state;
	cout<<g<<"\n";
	cout<<state<<"\n";
	auto a=run(state,g,1);
	cout<<a.first<<"\n";
	cout<<a.second<<"\n";
}
#endif
