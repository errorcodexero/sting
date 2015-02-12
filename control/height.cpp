#include "height.h"
#include <array>
#include <iostream>

//returns inches
std::array<float,3> findHeight(bool pickup,bool is_can, bool on_step,bool placed_on_scoring,int stacked_bins){
	const float HEIGHT_OF_SCORING_PLATFORM = 1.96;
	const float HEIGHT_OF_BIN = 12.1;
	//const float HEIGHT_OF_CAN = 29;//18
	const float HEIGHT_OF_STEP = 6.25;
	const float TO_CAN_RIB=20;//total guess
	float target = 0;
	if(placed_on_scoring){
		target += HEIGHT_OF_SCORING_PLATFORM;
	}else if(on_step){
		target += HEIGHT_OF_STEP;
	}
	target+=(stacked_bins* HEIGHT_OF_BIN);
	float positive_tolerance=2;
	float negative_tolerance=2;
	if(is_can){
		target+=TO_CAN_RIB;
		if(pickup){
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
		if(pickup){
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
int main(){
	std::array<float,3> x = findHeight(1,true,false,false,1);
	std::cout<<"Min: "<<x[0]<<" Target: "<<x[1]<<" Max: "<<x[2]<<"\n";
}
#endif
