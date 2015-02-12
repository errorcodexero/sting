#include "height.h"
#include <iostream>
#include "../util/util.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Lift_position const& a){
	o<<"Lift_position( ";
	#define X(NAME) o<<""#NAME<<":"<<a.NAME<<" ";
	X(pickup) X(is_can) X(on_step) X(placed_on_scoring) X(stacked_bins)
	#undef X
	return o<<")";
}

vector<unsigned> range(unsigned lim){
	vector<unsigned> r;
	for(unsigned i=0;i<lim;i++) r|=i;
	return r;
}

vector<Lift_position> lift_positions(){
	vector<bool> bools{0,1};
	vector<Lift_position> r;
	for(auto pickup:bools){
		for(auto is_can:bools){
			for(auto on_step:bools){
				vector<bool> placed_on_scoring_options;
				placed_on_scoring_options|=false;
				if(!on_step) placed_on_scoring_options|=true;
				for(auto placed_on_scoring:placed_on_scoring_options){
					for(auto bins:range(6)){
						r|=Lift_position{pickup,is_can,on_step,placed_on_scoring,bins};
					}
				}
			}
		}
	}
	return r;
}

//returns inches
//std::array<float,3> findHeight(bool pickup,bool is_can, bool on_step,bool placed_on_scoring,int stacked_bins){
std::array<float,3> findHeight(Lift_position const& a){
	const float HEIGHT_OF_SCORING_PLATFORM = 1.96;
	const float HEIGHT_OF_BIN = 12.1;
	//const float HEIGHT_OF_CAN = 29;//18
	const float HEIGHT_OF_STEP = 6.25;
	const float TO_CAN_RIB=20;//total guess
	float target = 0;
	if(a.placed_on_scoring){
		target += HEIGHT_OF_SCORING_PLATFORM;
	}else if(a.on_step){
		target += HEIGHT_OF_STEP;
	}
	target+=(a.stacked_bins* HEIGHT_OF_BIN);
	float positive_tolerance=2;
	float negative_tolerance=2;
	if(a.is_can){
		target+=TO_CAN_RIB;
		if(a.pickup){
			static const float CAN_PICKUP_MARGIN=1.5;
			target -= CAN_PICKUP_MARGIN;
			positive_tolerance=CAN_PICKUP_MARGIN;
		}else{
			static const float CAN_HOLD_MARGIN=3;
			target+=CAN_HOLD_MARGIN;
		}
	}else{
		static const float TO_BIN_HANDLE=10;//measured
		target+=TO_BIN_HANDLE;
		if(a.pickup){
			static const float BIN_PICKUP_MARGIN=1;
			target-=BIN_PICKUP_MARGIN;
			positive_tolerance=BIN_PICKUP_MARGIN;
		}else{
			static const float BIN_HOLD_MARGIN=4;
			target+=BIN_HOLD_MARGIN;
		}
	}
	return std::array<float,3>{target-negative_tolerance,target,target+positive_tolerance};
}

#ifdef HEIGHT_TEST
pair<int,float> to_feet(float inches){
	assert(inches>=0);
	int feet=0;
	while(inches>=12){
		feet++;
		inches-=12;
	}
	return make_pair(feet,inches);
}

template<typename T>
T min(vector<T> v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::min(r,elem);
	}
	return r;
}

template<typename T>
T max(vector<T> v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::max(r,elem);
	}
	return r;
}

int main(){
	Lift_position l{1,true,false,false,1};
	std::array<float,3> x = findHeight(l);
	std::cout<<"Min: "<<x[0]<<" Target: "<<x[1]<<" Max: "<<x[2]<<"\n";

	vector<float> heights;
	for(auto pos:lift_positions()){
		auto h=findHeight(pos);
		cout<<pos<<h<<"\n";
		heights|=h;
	}

	cout<<min(heights)<<" "<<max(heights)<<"\n";
	cout<<to_feet(max(heights))<<"\n";
}
#endif
