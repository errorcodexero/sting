#include "calibration_target.h"
#include "../util/util.h"

using namespace std;

Calibration_target::Calibration_target():target(Fire_control::NO_TARGET),top(0),direct_mode(0){}

Calibration_target::Calibration_target(Fire_control::Target a,bool b):target(a),top(b){}

vector<Calibration_target> Calibration_target::all(){
	vector<Calibration_target> r;
	vector<Fire_control::Target> t=Fire_control::targets();

	for(unsigned i=0;i<Fire_control::targets().size();i++){
		for(unsigned j=0;j<2;j++){
			r|=Calibration_target(Fire_control::targets()[i],j);
		}
	}
	assert(r.size());
	return r;
}

ostream& operator<<(ostream& o,Calibration_target a){
	o<<"Cal_target(";
	o<<a.target<<" ";
	if(a.top){
		o<<"top";
	}else{
		o<<"bottom";
	}
	o<<"direct:"<<a.direct_mode;
	return o<<")";
}

#ifdef CALIBRATION_TARGET_TEST
int main(){
	for(auto a:Calibration_target::all()){
		cout<<a<<"\n";
	}
}
#endif