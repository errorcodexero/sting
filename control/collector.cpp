#include "collector.h"
#include <iostream>
#include <stdio.h>

using namespace std;

Collector_mode collecting(Collector_mode state,bool press_on, bool press_reverse, bool release_reverse) {
	if (press_on && state == Collector_mode::OFF) {
		return Collector_mode::ON;
	}
	if (press_on && state == Collector_mode::ON) {
		return Collector_mode::OFF;
	}
	if ((press_reverse && state == Collector_mode::OFF) || (press_reverse && state == Collector_mode::ON)) {
		return Collector_mode::REVERSE;
	}
	if (release_reverse && state == Collector_mode::REVERSE){
		return Collector_mode::OFF;
	}
	if (state == Collector_mode::OFF) {
		return Collector_mode::OFF;
	}
	if (state == Collector_mode::ON) {
		return Collector_mode::ON;
	}
	if (state == Collector_mode::REVERSE){
		return Collector_mode::REVERSE;
	}
	throw 5;
}

ostream& operator << (ostream& o, Collector_mode c) {
	if (c == Collector_mode::ON) {
		o << "ON";
	}
	else if (c == Collector_mode::OFF) {
		o << "OFF";
	}
	if (c == Collector_mode::REVERSE) {
		o << "REVERSE";
	}
	return o;
}

Collector::Collector() {
	mode=Collector_mode::OFF;
}

#ifdef COLLECTOR_TEST
int state_machine_test() {
	for(int j=0;j<2;j++){
		for(int k=0;k<2;k++){
			for(int i=0;i<2;i++){
				cout<< "j;"<<j<<k<<i<<"\n";
				cout.flush();
				cout<<"ON:"<< collecting(Collector_mode::ON,j,k,i);
				cout.flush();
				cout<<"REVERSE:"<< collecting(Collector_mode::REVERSE,j,k,i);
				cout.flush();
				cout<<"OFF:"<< collecting(Collector_mode::OFF,j,k,i);
				cout.flush();
			}
		}
	}
	return 0;
}

int main() {
	Collector testvar;
}
#endif
