#include "bist.h"
#include<iostream>
#include<vector>
#include<cassert>
#include "../util/util.h"

using namespace std;

//this will need to transition to returning all of the outputs instead of the subgoals to get finer-grained control.
vector< pair<Toplevel::Output,Time> > bist_steps(){
	vector< pair<Toplevel::Output,Time> > r;
	Toplevel::Output g;
	g.collector=Collector_mode::ON;
	r|=make_pair(g,1.0);

	g.collector=Collector_mode::OFF;
	r|=make_pair(g,1.0);
	
	g.collector=Collector_mode::REVERSE;
	r|=make_pair(g,1.0);

	g.collector=Collector_mode::OFF;
	r|=make_pair(g,1.0);

	g.collector_tilt=Collector_tilt::OUTPUT_UP;
	r|=make_pair(g,2.0);

	g.collector_tilt=Collector_tilt::OUTPUT_DOWN;
	r|=make_pair(g,2.0);

	g.injector=Injector::OUTPUT_UP;
	r|=make_pair(g,1.0);

	g.injector=Injector::OUTPUT_DOWN;
	r|=make_pair(g,2.0);

	g.injector_arms=Injector_arms::OUTPUT_OPEN;
	r|=make_pair(g,1.0);

	g.injector_arms=Injector_arms::OUTPUT_CLOSE;
	r|=make_pair(g,1.0);

	g.ejector=Ejector::OUTPUT_UP;
	r|=make_pair(g,1.0);

	g.ejector=Ejector::OUTPUT_DOWN;
	r|=make_pair(g,2.0);

/*	g.shooter_wheels.top=1000;
	r|=make_pair(g,1.0);

	g.shooter_wheels.top=0;
	r|=make_pair(g,1.0);

	g.shooter_wheels.bottom=1000;
	r|=make_pair(g,1.0);

	g.shooter_wheels.bottom=0;
	r|=make_pair(g,1.0);*/

	return r;
}

static const vector< pair<Toplevel::Output,Time> > STEPS=bist_steps();

BIST::BIST():state(0),auto_mode(1){}

unsigned BIST::next()const{
	unsigned n=state+1;
	return min(n,(unsigned)(STEPS.size()-1));
}

unsigned BIST::prev()const{
	if(state==0) return 0;
	return state-1;
}

void BIST::update(Time time,bool step_fwd,bool stop,bool step_back){
	if(step_fwd){
		auto_mode=0;
		state=next();
		return;
	}
	if(step_back){
		auto_mode=0;
		state=prev();
		return;
	}
	if(stop){
		auto_mode=0;
		return;
	}
	if(auto_mode){
		timer.update(time,0);
		if(timer.elapsed()>STEPS[state].second){
			state=next();
			timer.update(time,1);
		}
	}
}

Toplevel::Output BIST::out()const{
	assert(state<STEPS.size());
	return STEPS[state].first;
}

ostream& operator<<(ostream& o,BIST b){
	o<<"BIST(";
	o<<b.state<<" ";
	o<<b.timer;
	return o<<")";
}

void BIST_interface::update(Time time,bool step_fwd_button,bool stop_button,bool step_back_button){
	b.update(time,step_fwd(step_fwd_button),stop(stop_button),step_back(step_back_button));
}

Toplevel::Output BIST_interface::out()const{ return b.out(); }

ostream& operator<<(ostream& o,BIST_interface b){
	o<<"BIST_interface(";
	o<<b.b;
	return o<<")";
}

#ifdef BIST_TEST
int main(){
	BIST b;
	cout<<b<<"\n";
	for(unsigned i=0;i<100;i++){
		b.update(.5*i,0,0,0);
		cout<<b<<"\n";
	}
	BIST_interface b2;
	b2.update(4,1,0,1);
}
#endif
