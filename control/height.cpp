#include "height.h"
#include <iostream>
#include "../util/util.h"

using namespace std;

Lift_position::Lift_position():
	pickup(0),
	is_can(0),
	on_step(0),
	placed_on_scoring(0),
	engage_kicker(0),
	add_half(0),
	stacked_bins(0.0)
{}

std::ostream& operator<<(std::ostream& o,Lift_position const& a){
	o<<"Lift_position( ";
	#define X(NAME) o<<""#NAME<<":"<<a.NAME<<" ";
	X(pickup) X(is_can) X(on_step) X(placed_on_scoring) X(stacked_bins) X(engage_kicker) X(add_half)
	#undef X
	return o<<")";
}

vector<unsigned> range(unsigned lim){
	vector<unsigned> r;
	for(unsigned i=0;i<lim;i++) r|=i;
	return r;
}

vector<Lift_position> examples(){
	vector<bool> bools{0,1};
	vector<Lift_position> r;
	for(auto pickup:bools){
		for(auto is_can:bools){
			for(auto on_step:bools){
				vector<bool> placed_on_scoring_options;
				placed_on_scoring_options|=false;
				if(!on_step) placed_on_scoring_options|=true;
				for(auto placed_on_scoring:placed_on_scoring_options){
					for(auto engage_kicker:bools){
						for(auto add_half:bools){
							for(auto bins:range(6)){
								Lift_position l;
								l.pickup=pickup;
								l.is_can=is_can;
								l.on_step=on_step;
								l.placed_on_scoring=placed_on_scoring;
								l.stacked_bins=bins;
								l.engage_kicker=engage_kicker;
								l.add_half=add_half;
								r|=l;
							}
						}
					}
				}
			}
		}
	}
	return r;
}

//returns inches
//everything is in inches
std::array<float,3> find_height(Lift_position const& a){
	//const float HEIGHT_OF_CAN = 29;//18
	float target=0.0;
	if(!a.engage_kicker){
		const float HEIGHT_OF_BIN=13.5;
		target=a.stacked_bins*HEIGHT_OF_BIN;
		if(a.add_half) target+=HEIGHT_OF_BIN*.5;
	}else{
		static const float ENGAGE_KICKER_HEIGHT=2.9;
		target=ENGAGE_KICKER_HEIGHT;
	}
	if(a.placed_on_scoring){
		const float HEIGHT_OF_SCORING_PLATFORM=1.96;
		target+=HEIGHT_OF_SCORING_PLATFORM;
	}else if(a.on_step){
		const float HEIGHT_OF_STEP=6.25;
		target+=HEIGHT_OF_STEP;
	}
	float positive_tolerance=2;
	float negative_tolerance=2;
	if(a.is_can){
		const float TO_CAN_RIB=21;//used ratio of picture pixels to get height
		target+=TO_CAN_RIB;
		if(a.pickup){
			static const float CAN_PICKUP_MARGIN=1.5;
			target-=CAN_PICKUP_MARGIN;
			positive_tolerance=CAN_PICKUP_MARGIN;
		}else{
			static const float CAN_HOLD_MARGIN=3;
			target+=CAN_HOLD_MARGIN;
		}
	}else{
		/*static const float TO_BIN_HANDLE=10;//measured
		target+=TO_BIN_HANDLE;*/
		if(a.pickup){
			static const float BIN_PICKUP_MARGIN=1;
			target-=BIN_PICKUP_MARGIN;
			positive_tolerance=BIN_PICKUP_MARGIN;
		}/*else{
			static const float BIN_HOLD_MARGIN=4;
			target+=BIN_HOLD_MARGIN;
		}*/
	}
	return std::array<float,3>{{target-negative_tolerance,target,target+positive_tolerance}};
}

double LiftToBar(double liftHeight) {
	return liftHeight<58?(1.06*liftHeight)+3.25: (3.12*liftHeight)-116;
}

double BarToLift(double barHeight) {
	return barHeight<58?(barHeight-3.25)/1.06: (barHeight+116)/3.12;
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
	Lift_position l;
	l.pickup=1;
	l.is_can=1;
	l.on_step=0;
	l.placed_on_scoring=0;
	l.stacked_bins=1;
	std::array<float,3> x =find_height(l);
	std::cout<<"Min: "<<x[0]<<" Target: "<<x[1]<<" Max: "<<x[2]<<"\n";

	vector<float> heights;
	for(auto pos:examples()){
		auto h=find_height(pos);
		cout<<pos<<h<<"\n";
		heights|=h;
	}

	cout<<min(heights)<<" "<<max(heights)<<"\n";
	cout<<to_feet(max(heights))<<"\n";
}
#endif
