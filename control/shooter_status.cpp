#include"shooter_status.h"
#include<iostream>
#include<cassert>
#include<cmath>
#include<stdlib.h>
#include "../util/util.h"

using namespace std;

namespace Shooter_wheels{
	Status::Status():top(0),bottom(0){}
	
	Status::Status(RPM a,RPM b):top(a),bottom(b){}

	bool operator==(Status a,Status b){
		return a.top==b.top && a.bottom==b.bottom;
	}

	bool operator!=(Status a,Status b){
		return !(a==b);
	}

	Status& operator-=(Status& a,Status b){
		a.top-=b.top;
		a.bottom-=b.bottom;
		return a;
	}
	
	ostream& operator<<(ostream& o,Status s){
		return o<<"Shooter_wheels::Status("<<s.top<<","<<s.bottom<<")";
	}

	bool approx_equal(Status a,Status b){
		//this threshold is totally arbitrary.
		static const double THRESHOLD=240;
		return fabs(a.top-b.top)<THRESHOLD && fabs(a.bottom-b.bottom)<THRESHOLD;
	}

	Maybe<Status> parse_status(string const& s){
		vector<string> v=split(inside_parens(s),',');
		if(v.size()!=2) return Maybe<Status>();
		return Maybe<Status>(Status(atoi(v[0].c_str()),atoi(v[1].c_str())));
	}
}

#ifdef SHOOTER_STATUS_TEST
int main(){
	Shooter_wheels::Status a;
	auto b=Shooter_wheels::parse_status(as_string(a));
	assert(a==b);
}
#endif
