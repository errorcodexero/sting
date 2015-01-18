#include "pump_advanced.h"
#include "../util/util.h"

/*
Important note to anyone reading this code:
Use the pressure estimates to control the pump at your own risk.  As far as I can tell, it follows the letter of the law as far as the 2014 FIRST regulations, but it _will not_ pass inspection.  This is because the inspectors check to see that the pump is following the sensor rather that it stops at the right level.  
*/

using namespace std;

double square(double x){ return x*x; }
static const double PI=3.14159;

typedef double Inch;
typedef double In3;

In3 cylinder_volume(Inch bore,Inch stroke){
	//ignoring the size of the shaft
	return PI*square(bore/2)*stroke;
}

//use constexpr when available
In3 injector_volume(){
	return 2*cylinder_volume(1.0625,12); //sizes are total guesses
}

In3 injector_arms_volume(){
	return cylinder_volume(.75,2.5);
}

In3 ejector_volume(){
	return 2*cylinder_volume(.75,2);
}

In3 collector_tilt_volume(){
	return 2*cylinder_volume(1.5,3);
}

In3 tank_volume(){
	//am-2477 per the AndyMark website, (.5 L); we should double check that this is what we're using.
	static const In3 SINGLE_TANK=66;

	return SINGLE_TANK*3;
}

typedef double CFM;
typedef double PSI;

CFM pump_speed_cfm(PSI pressure){
/*
 * From http://www.viaircorp.com/92C.html#tabs-2
 * TODO: Check that this is the right compressor
PSI 	CFM 	A 	BAR 	LPM 	A
0 	1.03 	7 	0 	29.0 	7
10 	0.71 	8 	1.0 	19.0 	8
20 	0.67 	8 	2.0 	18.0 	9
30 	0.64 	9 	3.0 	16.5 	9
40 	0.60 	9 	4.0 	15.0 	10
50 	0.57 	9 	5.0 	13.5 	10
60 	0.53 	10 	6.0 	12.5 	10
70 	0.48 	10 	7.0 	11.0 	9
80 	0.45 	10 	8.0 	10.0 	9
90 	0.43 	10 	  	  	 
100 	0.39 	9 	  	  	 
110 	0.36 	9 	  	  	 
120 	0.34 	9 	* Supply Voltage: 13.8 Volts
*/
	if(pressure<5) return 1.03;
	if(pressure<15) return .71;
	if(pressure<25) return .67;
	if(pressure<35) return .64;
	if(pressure<45) return .6;
	if(pressure<55) return .57;
	if(pressure<65) return .53;
	if(pressure<75) return .48;
	if(pressure<85) return .45;
	if(pressure<95) return .43;
	if(pressure<105) return .39;
	if(pressure<115) return .36;
	return .34;
}

typedef double PSI_per_second;

/*PSI_per_second pump_speed_psi_s(PSI current){
	double in3_per_s=pump_speed_cfm()/60*12*12*12;
	double tanks_per_s=in3_per_s/tank_volume();
	PSI atmosphere=14.7;
	
	assert(0);//not implemented
}*/
/*
0.5 Gallon Tank 	Fill Rate
0 to 80 PSI 	48 sec.
55 to 80 PSI 	16 sec.
0 to 105 PSI 	1 min. 08 sec.
85 to 105 PSI 	14 sec.
0 to 120 PSI 	1 min. 23 sec.
90 to 120 PSI 	25 sec.
1.0 Gallon Tank 	Fill Rate
0 to 80 PSI 	1 min. 48 sec.
55 to 80 PSI 	35 sec.
0 to 105 PSI 	2 min. 20 sec.
85 to 105 PSI 	30 sec.
0 to 120 PSI 	3 min. 04 sec.
90 to 120 PSI 	54 sec.
*/
struct Data_point{
	PSI start,end;
	Time time;

	Data_point(PSI a,PSI b,Time c):start(a),end(b),time(c){}
};

ostream& operator<<(ostream& o,Data_point a){
	return o<<"Data_point("<<a.start<<","<<a.end<<","<<a.time<<")";
}

vector<Data_point> half_gallon(){
	vector<Data_point> r;
	r|=Data_point(0,80,48);
	r|=Data_point(55,80,16);
	r|=Data_point(0,105,60+8);
	r|=Data_point(85,105,14);
	r|=Data_point(0,120,1*60+23);
	r|=Data_point(90,120,25);
	return r;
}

vector<Data_point> gallon(){
	vector<Data_point> r;
	r|=Data_point(0,80,1*60+48);
	r|=Data_point(55,80,35);
	r|=Data_point(0,105,2*60+20);
	r|=Data_point(85,105,30);
	r|=Data_point(0,120,3*60+4);
	r|=Data_point(90,120,54);
	return r;
}

namespace Pump_advanced{
	static const PSI SWITCH_CHANGE_LOW=85;
	static const PSI SWITCH_CHANGE_HIGH=110;

	struct Pressure_switch_sim{
		Pump::Status est;

		Pressure_switch_sim():est(Pump::NOT_FULL){}

		void update(PSI psi){
			//If these are >/< rather than >=/<= then feeding the sim back into itself will cause it to be confused.  This should not matter on the robot.
			if(psi>=SWITCH_CHANGE_HIGH) est=Pump::FULL;
			if(psi<=SWITCH_CHANGE_LOW) est=Pump::NOT_FULL;
		}

		Pump::Status estimate()const{ return est; }
	};
	ostream& operator<<(ostream& o,Pressure_switch_sim a){
		return o<<"Pressure_switch_sim("<<a.est<<")";
	}

	Estimator::Estimator():last_time(-1),last_reading(Pump::NOT_FULL),psi(0){} //or could initially start at full.
	
	void Estimator::update(Time now,Collector_tilt::Output collector_tilt_out,Injector::Output injector_out,Injector_arms::Output injector_arms_out,Ejector::Output ejector_out,Pump::Output pump_out,Pump::Status pressure_switch){
		if(last_time==-1) last_time=now;
		//cout<<"ch:"<<collector_tilt_out<<collector_tilt<<"\n";

		In3 air_use=0;
		if(collector_tilt_out!=collector_tilt){
			air_use+=collector_tilt_volume();
			cout<<"c\n";
			collector_tilt=collector_tilt_out;
			cout<<"d\n";
		}
		if(injector_out!=injector){
			air_use+=injector_volume();
			injector=injector_out;
		}
		if(injector_arms_out!=injector_arms){
			air_use+=injector_arms_volume();
			injector_arms=injector_arms_out;
		}
		if(ejector_out!=ejector){
			air_use+=ejector_volume();
			ejector=ejector_out;
		}
		//could add something here to account for leaks in the system.

		psi-=60*(air_use/tank_volume());
		if(psi<0) psi=0;

		//cout<<"m="<<psi<<"\n";

		Time elapsed=now-last_time;
		last_time=now;

		//cout<<"elapsed:"<<elapsed<<"\n";

		static const double PUMP_SPEED=1.8/2;//psi per second.  This is agrees with the manufacturer's documentation.
		if(pump_out==Pump::OUTPUT_ON){
			psi+=elapsed*PUMP_SPEED;
		}

		//cout<<"n="<<psi<<"\n";

		//pressure releif valve
		if(psi>125) psi=125;

		if(pressure_switch==Pump::NOT_FULL){
			if(last_reading==Pump::FULL){
				psi=SWITCH_CHANGE_LOW;
			}
			if(psi>SWITCH_CHANGE_HIGH) psi=SWITCH_CHANGE_HIGH;
		}else{
			if(last_reading==Pump::NOT_FULL){
				psi=SWITCH_CHANGE_HIGH;
			}
			if(psi<SWITCH_CHANGE_LOW) psi=SWITCH_CHANGE_LOW;
		}
		last_reading=pressure_switch;
	}

	PSI Estimator::estimate()const{ return psi; }

	ostream& operator<<(ostream& o,Estimator a){
		o<<"Pump_advanced::Estimator(";
		#define X(name) o<<""#name<<":"<<a.name<<" ";
		X(last_time)
		X(last_reading)
		X(psi)
		X(collector_tilt)
		X(injector)
		X(injector_arms)
		X(ejector)
		#undef X
		return o<<")";
	}

	Pump::Output control(PSI psi){
		return (psi>=120)?Pump::OUTPUT_OFF:Pump::OUTPUT_ON;
	}
}

#ifdef PUMP_ADVANCED_TEST

void est_test(){
	using namespace Pump_advanced;
	Estimator est;
	Pressure_switch_sim sensor;
	cout<<est<<"\n";
	for(Time t=0;t<120;t++){
		Pump::Output out=control(est.estimate());
		sensor.update(est.estimate());
		est.update(t,Collector_tilt::OUTPUT_UP,Injector::OUTPUT_DOWN,Injector_arms::OUTPUT_CLOSE,Ejector::OUTPUT_DOWN,out,sensor.estimate());
		cout<<est.estimate()<<" "<<out<<" "<<sensor.estimate()<<"\n";
	}
}

void switch_sim_test(){
	using namespace Pump_advanced;
	Pressure_switch_sim s;
	for(PSI p=0;p<130;p++){
		s.update(p);
		cout<<p<<":"<<s.estimate()<<"\n";
	}
	for(PSI p=130;p>70;p--){
		s.update(p);
		cout<<p<<":"<<s.estimate()<<"\n";
	}
}

void print_volumes(){
	#define X(name) cout<<""#name<<":"<<name()<<"\n";
	X(injector_volume)
	X(injector_arms_volume)
	X(ejector_volume)
	X(collector_tilt_volume)
	X(tank_volume)
	#undef X
}

typedef double Gal;//gallons

double gal_to_cu_in(Gal a){
	return a*231;
}

void show(vector<Data_point> v,Gal vol){
	cout<<v<<"\n";
	for(auto a:v){
		PSI change=a.end-a.start;
		double rate=change/a.time;
		double rate_localized=rate*gal_to_cu_in(vol)/tank_volume();
		cout<<a<<" "<<change<<" "<<rate_localized<<"\n";
	}
}

void print_speeds(){
	show(gallon(),1);
	show(half_gallon(),.5);
}

int main(){
	//switch_sim_test();
	est_test();
	//print_volumes();
	//print_speeds();
}
#endif
