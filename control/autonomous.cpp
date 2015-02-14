//Author(s): Logan Traffas, Matthew Macovsky
//A program that path finds for future use in autonomous
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>
#include <utility>
#include <fstream>
#include <assert.h>

using namespace std;

const int X_LIMIT=3;//The limits for the field -- robot can operate within this (i.e. one less)
const int Y_LIMIT=3;

enum class Action{FORWARD, BACKWARD, RIGHT, LEFT, LIFT, DROP};//Types of actions the robot can preform during autonomous

ostream& operator<<(ostream & o, Action in){//Sets how type "Action" prints
	if(in==Action::FORWARD){
		o<<"Instruction: FORWARD  ";
	}
	else if(in==Action::BACKWARD){
		o<<"Instruction: BACKWARD ";
	}
	else if(in==Action::RIGHT){
		o<<"Instruction: RIGHT    ";
	}
	else if(in==Action::LEFT){
		o<<"Instruction: LEFT     ";
	}
	else if(in==Action::LIFT){
		o<<"Instruction: LIFT     ";
	}
	else if(in==Action::DROP){
		o<<"Instruction: DROP     ";
	}
	return o;
}

typedef pair<int, int> Location;//A type for storing the locations of things

struct Environment_state{//Type to store the states of the environment
	struct Robot_state{//Type to store the current states of the robot
		Location location;
		bool with_tote;
		Robot_state():with_tote(0){}
	};
	Robot_state robot;
	Location tote_location;//The location of the tote (assuming only one tote for now)
};

template<typename T>
ostream & operator<<(ostream & o, vector<T> in){//Sets hot type vector<type> prints
	o<<"[";
	for(unsigned int i=0; i<in.size(); i++){
		o<<in[i];
	}
	o<<"]";
	return o;
}

ostream & operator<<(ostream & o, Location location){//Sets how type "Location" prints
	o<<"("<<location.first<<","<<location.second<<")";
	return o;
}

ostream & operator<<(ostream & o, Environment_state in){//Sets how type "Environment_state" prints
	o<<"Robot location: "<<in.robot.location<<"   Tote location: "<<in.tote_location<<"   Robot with tote: "<<in.robot.with_tote;
	return o;
}

bool operator==(pair<int, int> a, pair<int, int> b) {//Sets operator "==" for pairs of ints
	return a.first==b.first && a.second==b.second;
}

bool operator!=(pair<int, int> a, pair<int, int> b) {//Sets operator "!=" for pairs of ints
	return !(a==b);
}

bool operator==(Environment_state::Robot_state a, Environment_state::Robot_state b){//Sets the operator "==" for type Robot_state
	return a.location==b.location && a.with_tote==b.with_tote;
}

bool operator==(Environment_state a, Environment_state b){//Sets the operator "==" for type Environment_state
	return a.robot==b.robot && a.tote_location==b.tote_location;
}

bool operator!=(Environment_state a, Environment_state b){//Sets the operator "!=" for type Environment_state
	return !(a==b);
}

Environment_state update_environment(Action instruction, Environment_state a){//Updates the environment with the robot following the instruction
	if(instruction==Action::LEFT){
		assert(a.robot.location.first>0);
		a.robot.location.first--;
	}
	if(instruction==Action::RIGHT){
		assert(a.robot.location.first+1<X_LIMIT);
		a.robot.location.first++;
	}
	if(instruction==Action::BACKWARD){
		assert(a.robot.location.second>0);
		a.robot.location.second--;
	}
	if(instruction==Action::FORWARD){
		assert(a.robot.location.second+1<Y_LIMIT);
		a.robot.location.second++;
	}
	if(instruction==Action::LIFT){
		assert(a.robot.location==a.tote_location);
		assert(a.robot.with_tote==0);
		a.robot.with_tote=1;
	}
	if(instruction==Action::DROP){
		assert(a.robot.with_tote==1);
		a.robot.with_tote=0;
	}
	if(a.robot.with_tote==1)a.tote_location=a.robot.location;
	return a;
}

vector<Action> get_possible_moves(Environment_state environment){//Determines all the possible actions a robot can preform within a given environment
	vector<Action> possible_moves;
	if(environment.robot.location.first>0){
		possible_moves.push_back(Action::LEFT);
	}
	if(environment.robot.location.first+1<X_LIMIT){
		possible_moves.push_back(Action::RIGHT);
	}
	if(environment.robot.location.second>0){
		possible_moves.push_back(Action::BACKWARD);
	}
	if(environment.robot.location.second+1<Y_LIMIT){
		possible_moves.push_back(Action::FORWARD);
	}
	if(environment.robot.location==environment.tote_location && environment.robot.with_tote==0){
		possible_moves.push_back(Action::LIFT);
	}
	if(environment.robot.with_tote==1){
		possible_moves.push_back(Action::DROP);
	}
	return possible_moves;
}

vector<Environment_state> get_all_states(){//Makes a vector of all possible environments given the X/Y limitations
	Environment_state a;
	a.robot.with_tote=0;
	vector<Environment_state> r;
	for(int x=0; x<X_LIMIT; x++){
		for(int y=0; y<Y_LIMIT; y++){
			a.robot.location=make_pair(x,y);
			for(int z=0; z<X_LIMIT; z++){
				for(int w=0; w<Y_LIMIT; w++){
					a.tote_location=make_pair(z,w);
					r.push_back(a);
				}
			}
			a.robot.with_tote=1;
			a.tote_location=a.robot.location;
			r.push_back(a);
			a.robot.with_tote=0;
		}
	}
	return r;
}

pair<bool, vector<Action>> find_path(Environment_state a, Environment_state b, int max_moves=0){//Tries to find the actions the robot must preform to reach the target environment 
	pair<bool, vector<Action>> path_return;
	if(a==b){
		path_return.first=1;
		return path_return;
	}
	if(a!=b && max_moves==0){
		path_return.first=0;
		return path_return;
	}
	max_moves--;
	for(Action move:get_possible_moves(a)){
		path_return=find_path(update_environment(move, a), b, max_moves);
		if(path_return.first==1){
			path_return.second.insert(path_return.second.begin(), move);
			return path_return;
		}
	}
	return path_return;
}

vector<Action> get_instructions(Environment_state a, Environment_state b){//Searches for instructions (of length "i") for the robot to reach the target environment
	cout<<"STARTING:     "<<a<<endl<<"TARGET:       "<<b<<endl;
	vector<Action> instructions;
	pair<bool, vector<Action>> path_return;
	for(unsigned int i=0; i<1000; i++){
		cout<<"Testing paths of length "<<i<<"."<<endl;
		path_return=find_path(a, b, i);
		if(path_return.first){
			instructions=path_return.second;
			break;
		}
	}
	cout<<"INSTRUCTIONS: "<<instructions<<endl;
	return instructions;
}

int main(){//This is main. If you don't know what this is, then you probably shouldn't be looking at any of this (probably)
	Environment_state current, target;//Sets current and target environments for testing
	current.robot.location=make_pair(0,0);
	current.tote_location=make_pair(2,2);
	current.robot.with_tote=0;
	target.robot.location=make_pair(0,0);
	target.tote_location=make_pair(0,0);
	target.robot.with_tote=0;
	get_instructions(current, target);
	return 0;
}