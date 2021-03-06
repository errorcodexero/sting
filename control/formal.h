#ifndef FORMAL_H
#define FORMAL_H

#include<iostream>
#include<set>
#include<sstream>
#include<vector>
#include <stdlib.h>
#include "../util/interface.h"
#include "../util/util.h"

template<
	typename Part,
	typename Input,
	typename Output,
	typename Goal
>
auto run(Part &p,Time t,Input in,Output out,Goal goal)->std::pair<Output,decltype(status(p.estimator.get()))>{
	p.estimator.update(t,in,out);
	auto st_detail=p.estimator.get();
	return std::make_pair(control(st_detail,goal),status(st_detail));
}

#define NYI { assert(0); }

template<typename T>
void test_ostream(std::string heading,T* t){
	std::cout<<heading<<":";
	std::set<std::string> used;
	auto ex=examples(t);

	if(ex.empty()){
		std::cout<<"Error: No examples\n";
		exit(1);
	}

	for(auto a:ex){
		std::stringstream ss;
		ss<<a;
		auto s=ss.str();
		std::cout<<" "<<s;
		if(used.find(s)!=used.end()){
			//duplicate
			std::cout<<"duplicate\n";
			exit(1);
		}
		used|=s;
	}
	std::cout<<"\n";
}

struct Tester_mode{
	bool check_outputs_exhaustive=1;
	bool check_multiple_outputs=1;
};

template<typename Part>
void tester(Part p,Tester_mode mode=Tester_mode{}){
	using namespace std;

	typedef typename Part::Input Input;
	typedef typename Part::Goal Goal;
	typedef decltype(p.estimator.get()) Status_detail;
	typedef typename Part::Output Output;
	typedef typename Part::Status Status;

	cout<<p<<"\n";

	test_ostream("inputs",(Input*)0);
	test_ostream("outputs",(Output*)0);
	test_ostream("status details",(Status_detail*)0);
	test_ostream("statuses",(Status*)0);
	test_ostream("goals",(Goal*)0);

	for(auto a:examples((Input*)0)){
		Robot_inputs all;
		auto x=p.input_reader(p.input_reader(all,a));
		assert(x==a);
	}

	{
		set<Output> used;
		for(auto status_detail:examples((Status_detail*)0)){
			for(auto goal:examples((Goal*)0)){
				used|=control(status_detail,goal);
			}
		}
		if(mode.check_multiple_outputs && used.size()<2){
			assert(0);
		}
		if(mode.check_outputs_exhaustive && used!=examples((Output*)0)){
			cout<<"used:"<<used<<"\n";
			cout<<"examples:"<<examples((Output*)0)<<"\n";
			//test failed
			assert(0);
		}
	}

	{
		set<Status> used;
		for(auto status_detail:examples((Status_detail*)0)){
			used|=status(status_detail);
		}
		if(used!=examples((Status*)0)){
			//test failed
			cout<<"used:"<<used<<"\n";
			cout<<"examples:"<<examples((Status*)0)<<"\n";
			assert(0);//NYI
		}
	}

	{
		set<bool> used;
		for(auto status:examples((Status*)0)){
			for(auto goal:examples((Goal*)0)){
				used|=ready(status,goal);
			}
		}
		auto f=used.find(1);
		if(f==used.end()){
			//test failed
			NYI
		}
	}

	//this part could be more sophisticated to try to explore all the state space better.
	{
		set<Status_detail> used;
		for(Time t:vector<Time>{0,.01,1,10}){
			for(auto in:examples((Input*)0)){
				for(auto out:examples((Output*)0)){
					p.estimator.update(t,in,out);
					Part const& p1=p;
					used|=p1.estimator.get();
				}
			}
		}
		//do nothing w/ "used"
	}

	{
		set<Robot_outputs> outputs;
		for(auto output:examples((Output*)0)){
			Part const& p1=p;
			auto out=p1.output_applicator(Robot_outputs{},output);
			outputs|=out;
			auto recovered=p1.output_applicator(out);
			if(recovered!=output){
				//the value recovery didn't work
				cout<<"recovered:"<<recovered<<" "<<"output:"<<output<<"\n";
				NYI
			}
		}
		if(mode.check_multiple_outputs && outputs.size()<2){
			cout<<"found:"<<outputs<<"\n";
			//test fail becuase it doesn't actually control anything
			NYI
		}
	}
}

#endif
