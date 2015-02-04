#include "main.h"
#include<iostream>
#include<sstream>
#include<cassert>
#include <math.h>
#include "holonomic.h"
#include "toplevel.h"
#include "fire_control.h"
#include "control_status.h"
#include "../input/util.h"
#include "../input/panel2014.h"
#include "../util/util.h"
#include "toplevel_mode.h"
#include <stdlib.h>

using namespace std;

double convert_output(Collector_mode m){
	switch(m){
		case Collector_mode::ON: return -1; //Actually collecting
		case Collector_mode::OFF: return 0;
		case Collector_mode::REVERSE: return 1; //Ejecting
		default: assert(0);
	}
}

static const int JAG_TOP_FEEDBACK=1;
static const int JAG_BOTTOM_FEEDBACK=3;
static const int JAG_TOP_OPEN_LOOP=0;
static const int JAG_BOTTOM_OPEN_LOOP=2;

Robot_outputs convert_output(Toplevel::Output a){
	Robot_outputs r;
	r.pwm[0]=pwm_convert(a.drive.a);
	r.pwm[1]=pwm_convert(a.drive.b);
	r.pwm[2]=pwm_convert(a.drive.c);
	r.pwm[3]=convert_output(a.collector);
	
	r.relay[0]=(a.pump==Pump::OUTPUT_ON)?Relay_output::_10:Relay_output::_00;
	
	r.solenoid[7]=(a.injector_arms!=Injector_arms::OUTPUT_CLOSE);

	//pressure switch
	r.digital_io[0]=Digital_out::INPUT;

	//cerr<<a.shooter_wheels<<"\r\n";
	r.jaguar[JAG_TOP_FEEDBACK]=a.shooter_wheels.top[Shooter_wheels::Output::FEEDBACK];
	r.jaguar[JAG_BOTTOM_FEEDBACK]=a.shooter_wheels.bottom[Shooter_wheels::Output::FEEDBACK];
	r.jaguar[JAG_TOP_OPEN_LOOP]=a.shooter_wheels.top[Shooter_wheels::Output::OPEN_LOOP];
	r.jaguar[JAG_BOTTOM_OPEN_LOOP]=a.shooter_wheels.bottom[Shooter_wheels::Output::OPEN_LOOP];
	/*for(unsigned i=0;i<4;i++){
		cerr<<r.jaguar[i]<<"\r\n";
	}*/
	return r;
}

//todo: at some point, might want to make this whatever is right to start autonomous mode.
Main::Main():control_status(Control_status::DRIVE_W_BALL),autonomous_start(0){}

Control_status::Control_status next(Control_status::Control_status status,Toplevel::Status part_status,Joystick_data j,Panel,bool autonomous_mode,bool autonomous_mode_start,Time since_switch,Shooter_wheels::Calibration,Time autonomous_mode_left);

Drive_goal teleop_drive_goal(double joy_x,double joy_y,double joy_theta,double joy_throttle,bool field_relative){
	double throttle = .7+.3*fabs(joy_throttle); //Robot always drives at 70% power when throttle is not engaged //Adds the final 30% as a percentage of the throttle
	joy_x = clip(joy_x) * throttle;
	joy_y = -clip(joy_y) * throttle;//Invert Y
	joy_theta = clip(joy_theta) * 0.75; //Twist is only ever goes at 75% speed and is unaffected by throttle
	return Drive_goal(Pt(joy_x,joy_y,joy_theta),field_relative);
}

Drive_goal drive_goal(Control_status::Control_status control_status,
		double joy_x,
		double joy_y,
		double joy_theta,
		double joy_throttle,
		bool field_relative)
{
	if(teleop(control_status)){
		return teleop_drive_goal(joy_x,joy_y,joy_theta,joy_throttle,field_relative);
	}
	Drive_goal r;
	switch(control_status){
		case Control_status::AUTO_COLLECT:
		case Control_status::A2_MOVE:
			r.direction.y=-1; //Full power backwards
			r.direction.theta = -0.1; //Slight twist to coutner the natrual twist from Holonomic drive base
			break;
		case Control_status::A2_TO_COLLECT:
			r.direction.y=.4;
			break;
		default:
			//otherwise leave at the default, which is 0.
			break;
	}
	return r;
}

Toplevel::Output panel_override(Panel p,Toplevel::Output out){
	#define X(name) if(p.name) out.name=*p.name;
	X(collector)
	//X(injector_arms)
	#undef X
	if(p.force_wheels_off){
		out.shooter_wheels=Shooter_wheels::Output();
	}
	return out;
}

bool vowel(char c){
	c=tolower(c);
	return c=='a' || c=='e' || c=='i' || c=='o' || c=='u' || c=='y';
}

string abbreviate_text(string s){
	stringstream ss;
	//bool skipped_last=0;
	for(unsigned i=0;i<s.size();i++){
		if(s[i]==' ' || s[i]==':' || s[i]=='_') continue;
		if(vowel(s[i])){
			//skip
		}else{
			ss<<s[i];
		}
	}
	return ss.str();
}

Robot_outputs Main::operator()(Robot_inputs in,ostream&){
	perf.update(in.now);
	Joystick_data main_joystick=in.joystick[0];
	Joystick_data gunner_joystick=in.joystick[1];
	since_switch.update(in.now,0);
	force.update(
		main_joystick.button[Gamepad_button::A],
		main_joystick.button[Gamepad_button::LB],
		main_joystick.button[Gamepad_button::RB],
		main_joystick.button[Gamepad_button::BACK],
		main_joystick.button[Gamepad_button::B],
		main_joystick.button[Gamepad_button::X]
	);

	{
		Robot_outputs r;
		for(unsigned i=0;i<r.PWMS;i++){
			r.pwm[i]=0;
		}
		auto x=main_joystick.axis[0];
		auto y=main_joystick.axis[1];
		auto theta=main_joystick.axis[4]/2; //theta is /2 so rotation is reduced to prevent bin tipping.
		auto l1=y-theta;
		auto r1=y+theta;
		auto lim=max(1.0,max(l1,r1));
		r.pwm[0]=-(pow((l1/lim),3));//Chenge these "coefficients" for different movement behavior
		r.pwm[1]=pow((r1/lim),3);
		r.pwm[2]=x;
		r.pwm[3]=[&](){
			if(main_joystick.button[0]) return .5;
			if(main_joystick.button[1]) return -.5;
			return 0.0;
		}();
		r=force(r);
		return r;
	}

	ball_collecter.update(main_joystick.button[5]);
	bool tanks_full=(in.digital_io[0]==Digital_in::_1);

	Panel panel;//ignore since absent=interpret(in.driver_station);
	Shooter_wheels::Calibration calib=wheel_calibration.update(panel.learn,panel.speed,panel.target,panel.pidselect,panel.pidadjust);
	//Control_status::Control_status next(Control_status::Control_status status,Toplevel::Status part_status,Joystick_data j,bool autonomous_mode,Time since_switch){
	Toplevel::Status toplevel_status=est.estimate();
	
	//Autonomous 
	bool autonomous_start_now=autonomous_start(in.robot_mode.autonomous && in.robot_mode.enabled);
	since_auto_start.update(in.now,autonomous_start_now);
	static const Time AUTONOMOUS_MODE_LENGTH=10;
	Control_status::Control_status control_status_next=next(
		control_status,toplevel_status,gunner_joystick,panel,
		in.robot_mode.autonomous,
		autonomous_start_now,
		since_switch.elapsed(),
		calib,
		AUTONOMOUS_MODE_LENGTH-since_auto_start.elapsed()
	);
	if(control_status_next!=control_status){
		since_switch.update(in.now,1);
	}
	control_status=control_status_next;
	
	field_relative.update(main_joystick.button[Gamepad_button::X]);
	
	Toplevel::Mode mode=Toplevel::to_mode(control_status);
	Drive_goal drive_goal1=drive_goal(
		control_status,
		main_joystick.axis[Gamepad_axis::LEFTX],
		main_joystick.axis[Gamepad_axis::LEFTY],
		main_joystick.axis[Gamepad_axis::RIGHTX],
		main_joystick.axis[Gamepad_axis::TRIGGER],
		field_relative.get()
	);
	Toplevel::Subgoals subgoals_now=subgoals(mode,drive_goal1,calib);
	Toplevel::Output high_level_outputs=control(toplevel_status,subgoals_now);
	high_level_outputs=panel_override(panel,high_level_outputs);
	if(gunner_joystick.button[Gamepad_button::START]){
		high_level_outputs.collector=Collector_mode::REVERSE;
	}
	Robot_outputs r=convert_output(high_level_outputs);
	{
		Shooter_wheels::Status wheel;
		wheel.top=in.jaguar[JAG_TOP_FEEDBACK].speed;
		wheel.bottom=in.jaguar[JAG_BOTTOM_FEEDBACK].speed;
		bool downsensor=in.digital_io[1]==Digital_in::_1;
		est.update(in.now,in.robot_mode.enabled,high_level_outputs,tanks_full?Pump::FULL:Pump::NOT_FULL,in.orientation,wheel,downsensor);
	}

	stringstream strin;
	strin<<toplevel_status.shooter_wheels;
	{
		static int i=0;
		if(i==0){
			stringstream ss;
			ss<<in<<"\r\n"<<*this<<mode<<"\r\n";
			ss<<r<<"\r\n"; 
			//cerr<<ss.str();//putting this all together at once in hope that it'll show up at closer to the same time.  
			//cerr<<subgoals_now<<high_level_outputs<<"\n";
		}
		i=(i+1)%100;
	}
	//cerr<<subgoals_now<<"\r\n";
	//cerr<<toplevel_status<<"\r\n\r\n";
	//cerr<<"Waiting on:"<<not_ready(toplevel_status,subgoals_now)<<"\n";
	//cout<<"thoihewoi\n";
	
	if(print_button(main_joystick.button[Gamepad_button::LB])){
		cout<<in<<"\r\n";
		cout<<*this<<"\r\n";
		cout<<"\r\n";
	}
	//log_line(cout,in,*this,r);
	return r;
}

bool operator==(Main a,Main b){
	return a.force==b.force && 
		a.perf==b.perf && 
		a.est==b.est && 
		a.control_status==b.control_status && 
		a.since_switch==b.since_switch && 
		a.since_auto_start==b.since_auto_start &&
		a.ball_collecter==b.ball_collecter && 
		a.print_button==b.print_button && 
		a.field_relative==b.field_relative && 
		a.autonomous_start==b.autonomous_start && 
		a.wheel_calibration==b.wheel_calibration;
}

bool operator!=(Main a,Main b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Main m){
	o<<"Main(";
	o<<m.force;
	o<<m.perf;
	o<<m.est;
	o<<m.control_status;
	o<<m.since_switch;
	//since_auto_start
	o<<m.wheel_calibration;
	//o<<m.control;
	//ball collector
	//print button
	//relative
	//field relative
	return o<<")";
}

Fire_control::Target to_target(Joystick_section j,Mode_buttons mode_buttons){
	switch(j){
		case Joystick_section::LEFT: return Fire_control::TRUSS;
		case Joystick_section::RIGHT: return Fire_control::AUTO_SHOT;
		case Joystick_section::UP: return Fire_control::HIGH;
		case Joystick_section::DOWN: return Fire_control::EJECT;
		default: break;
	}
	if(mode_buttons.truss_toss) return Fire_control::TRUSS;
	if(mode_buttons.shoot_high) return Fire_control::HIGH;
	if(mode_buttons.auto_shot) return Fire_control::AUTO_SHOT;
	return Fire_control::NO_TARGET;
}

Control_status::Control_status next(
	Control_status::Control_status status,
	Toplevel::Status part_status,
	Joystick_data j,
	Panel panel,
	bool autonomous_mode,
	bool autonomous_mode_start,
	Time since_switch,
	Shooter_wheels::Calibration calib,
	Time autonomous_time_left
){
	using namespace Control_status;

	//if(autonomous_mode && !autonomous(status)){
	if(autonomous_mode_start){
		if(panel.auto_mode==Panel::DO_NOTHING)return DRIVE_W_BALL;
		if(panel.auto_mode==Panel::ONE_BALL)return A2_SPIN_UP;
		if(panel.auto_mode==Panel::TWO_BALL)return A2_SPIN_UP;
		if(panel.auto_mode==Panel::MOVE)return A2_MOVE;
		return DRIVE_W_BALL;
	}

	if(!autonomous_mode){
		//at the top here should deal with all the buttons that put you into a specific mode.
		if(j.button[Gamepad_button::A] || panel.mode_buttons.catch_mode) return Control_status::CATCH;
		if(j.button[Gamepad_button::B] || panel.mode_buttons.collect) return COLLECT;
		if(j.button[Gamepad_button::X] || panel.mode_buttons.drive_w_ball) return DRIVE_W_BALL;
		if(j.button[Gamepad_button::Y] || panel.mode_buttons.drive_wo_ball) return DRIVE_WO_BALL;
		//Changed so as not to accidentally time out the robot
		//if(j.button[Gamepad_button::Y] || panel.mode_buttons.drive_wo_ball) return Control_status::SHOOT_LOW;
		//todo: use some sort of constants rather than 0/1 for the axes
		{
			Joystick_section joy_section=joystick_section(j.axis[0],j.axis[1]);
			Fire_control::Target target=to_target(joy_section,panel.mode_buttons);
			if(Fire_control::target(status)!=target && !autonomous_mode){
				switch(target){
					case Fire_control::TRUSS: return TRUSS_TOSS_PREP;
					case Fire_control::HIGH: return SHOOT_HIGH_PREP;
					case Fire_control::EJECT: return EJECT_PREP;
					case Fire_control::AUTO_SHOT:return AUTO_SHOT_PREP;
					default: break;
				}
			}
		}
	}
	bool fire_now,fire_when_ready;
	{
		Joystick_section vert=divide_vertical(j.axis[Gamepad_axis::RIGHTY]);
		fire_now=(vert==Joystick_section::UP) || panel.fire;
		fire_when_ready=(vert==Joystick_section::DOWN); //No equivalent on the switchpanel.
	}

	bool ready_to_shoot=ready(part_status,subgoals(Toplevel::SHOOT_HIGH_PREP,Drive_goal(),calib));
	bool ready_to_truss_toss=ready(part_status,subgoals(Toplevel::TRUSS_TOSS_PREP,Drive_goal(),calib));
	bool ready_to_collect=ready(part_status,subgoals(Toplevel::COLLECT,Drive_goal(),calib));
	bool ready_to_auto_shot=ready(part_status,subgoals(Toplevel::AUTO_SHOT_PREP,Drive_goal(),calib));
	bool took_shot=1;
	bool have_collected_question = false;

	static const Time AUTO_DRIVE_TIME=1.5;
	bool auto_almost_done=AUTO_DRIVE_TIME>=autonomous_time_left;

	switch(status){
		case A2_SPIN_UP:
			if(autonomous_mode){
				if(auto_almost_done) return A2_FIRE2;
				return ready_to_auto_shot?A2_FIRE:A2_SPIN_UP;
			}
			return TRUSS_TOSS_PREP;
		case A2_FIRE:
			if(autonomous_mode){
				if(auto_almost_done) return A2_FIRE2;
				return took_shot?A2_TO_COLLECT:A2_FIRE;
			}
			return AUTO_SHOT;
		case A2_TO_COLLECT:
			if(autonomous_mode){
				if(auto_almost_done) return A2_MOVE;
				return ready_to_collect?A2_COLLECT:A2_TO_COLLECT;
			}
			return COLLECT;
		case A2_COLLECT:
			if(autonomous_mode){
				if(auto_almost_done) return A2_MOVE;
				return (since_switch>1.8)?A2_SPIN_UP2:A2_COLLECT;
			}
			return COLLECT;
		case A2_SPIN_UP2:
			if(autonomous_mode){
				if(auto_almost_done){
					return A2_MOVE;
				}
				return ready_to_auto_shot?A2_FIRE2:A2_SPIN_UP2;
			}
			return AUTO_SHOT_PREP;
		case A2_FIRE2:
			if(autonomous_mode){
				if(auto_almost_done) return A2_MOVE;
				return took_shot?A2_MOVE:A2_FIRE2;
			}
			return AUTO_SHOT;
		case A2_MOVE:
			if(autonomous_mode){
				return (since_switch>AUTO_DRIVE_TIME)?DRIVE_WO_BALL:A2_MOVE;
			}
			return DRIVE_WO_BALL;
		case AUTO_SPIN_UP:
			if(autonomous_mode){
				return ready_to_auto_shot?AUTO_FIRE:AUTO_SPIN_UP;
			}
			return TRUSS_TOSS_PREP;
		case AUTO_FIRE:
			if(autonomous_mode){
				return took_shot?AUTO_TO_COLLECT:AUTO_FIRE;
			}
			return TRUSS_TOSS;
		case AUTO_TO_COLLECT:
			if(autonomous_mode){
				return ready_to_collect?AUTO_COLLECT:AUTO_TO_COLLECT;
			}
			return COLLECT;
		case AUTO_COLLECT:
			if(autonomous_mode){
				//this is a very non-scientific way of driving, and not really the right way to do this.
				return (since_switch>AUTO_DRIVE_TIME)?/*AUTO_SPIN_UP2*/DRIVE_WO_BALL:AUTO_COLLECT;
			}
			return COLLECT;
		case AUTO_SPIN_UP2:
			if(autonomous_mode){
				return ready_to_auto_shot?AUTO_FIRE2:AUTO_SPIN_UP2;
			}
			return TRUSS_TOSS_PREP;
		case AUTO_FIRE2:
			if(autonomous_mode){
				return took_shot?DRIVE_WO_BALL:AUTO_FIRE2;
			}
			return TRUSS_TOSS;
		case DRIVE_W_BALL:
		case DRIVE_WO_BALL:
			return status;
		case COLLECT:
			return have_collected_question?DRIVE_W_BALL:COLLECT;
		case SHOOT_HIGH_PREP:
			if(fire_now){
				return SHOOT_HIGH;
			}
			if(fire_when_ready){
				return SHOOT_HIGH_WHEN_READY;
			}
			return SHOOT_HIGH_PREP;
		case SHOOT_HIGH: return took_shot?DRIVE_WO_BALL:SHOOT_HIGH;
		case SHOOT_HIGH_WHEN_READY:
			if(fire_now){
				return SHOOT_HIGH;
			}
			if(!fire_when_ready){
				return SHOOT_HIGH_PREP;
			}
			return ready_to_shoot?SHOOT_HIGH:SHOOT_HIGH_WHEN_READY;
		case TRUSS_TOSS_PREP: 
			if(fire_now){
				return TRUSS_TOSS;
			}
			if(fire_when_ready){
				return TRUSS_TOSS_WHEN_READY;
			}
			return TRUSS_TOSS_PREP;
		case TRUSS_TOSS: return took_shot?DRIVE_WO_BALL:TRUSS_TOSS;
		case TRUSS_TOSS_WHEN_READY: 
			if(fire_now){
				return TRUSS_TOSS;
			}
			if(!fire_when_ready){
				return TRUSS_TOSS_PREP;
			}
			return ready_to_truss_toss?TRUSS_TOSS:TRUSS_TOSS_WHEN_READY;
		case EJECT_PREP: 
			if(fire_now){
				return EJECT;
			}
			if(fire_when_ready){
				return EJECT_WHEN_READY;
			}
			return EJECT_PREP;
		case EJECT: return DRIVE_WO_BALL;
		case EJECT_WHEN_READY:
		{
			return EJECT;
		}
		case AUTO_SHOT_PREP: 
			if(fire_now){
				return AUTO_SHOT;
			}
			if(fire_when_ready){
				return AUTO_SHOT_WHEN_READY;
			}
			return AUTO_SHOT_PREP;
		case AUTO_SHOT: 
			return took_shot?DRIVE_WO_BALL:AUTO_SHOT;
		case AUTO_SHOT_WHEN_READY: 
			if(fire_now){
				return AUTO_SHOT;
			}
			if(!fire_when_ready){
				return AUTO_SHOT_PREP;
			}
			return ready_to_auto_shot?AUTO_SHOT:AUTO_SHOT_WHEN_READY;
		case CATCH: return status;
		default:
			assert(0);
	}
}

bool approx_equal(Main a,Main b){
	//cout<<"a\n";
	if(a.force!=b.force) return 0;
	//cout<<"b\n";
	//if(!approx_equal(a.perf,b.perf)) return 0;
	//cout<<"1\n";
	if(!approx_equal(a.est,b.est)) return 0;
	//cout<<"a\n";
	if(a.control_status!=b.control_status) return 0;
	if(a.ball_collecter!=b.ball_collecter) return 0;
	//cout<<"b\n";
	if(a.print_button!=b.print_button) return 0;
	if(a.field_relative!=b.field_relative) return 0;
	return 1;
}

#ifdef MAIN_TEST
#include<fstream>
#include "wheel_sim.h"
#include "monitor.h"

Jaguar_input jag_at_speed(double speed){
	Jaguar_input r;
	r.speed=speed;
	return r;
}

Shooter_wheels::Output shooter_output(Robot_outputs out){
	Shooter_wheels::Output r;
	r.top[Shooter_wheels::Output::FEEDBACK]=out.jaguar[JAG_TOP_FEEDBACK];
	r.top[Shooter_wheels::Output::OPEN_LOOP]=out.jaguar[JAG_TOP_OPEN_LOOP];
	r.bottom[Shooter_wheels::Output::FEEDBACK]=out.jaguar[JAG_BOTTOM_FEEDBACK];
	r.bottom[Shooter_wheels::Output::OPEN_LOOP]=out.jaguar[JAG_BOTTOM_OPEN_LOOP];
	return r;
}

vector<Control_status::Control_status> auto_test(ostream& o,double automodeknob){
	vector<Control_status::Control_status> v;
	Main m;
	Monitor<Robot_inputs> inputs;
	Monitor<Main> state;
	Monitor<Robot_outputs> outputs;
	Shooter_sim shooter_sim;
	for(unsigned i=0;i<1500;i++){
		Robot_inputs in;
		in.driver_station.analog[0]=automodeknob;
		in.now=i/100.0;
		in.robot_mode.autonomous=1;
		in.robot_mode.enabled=1;
		//in.jaguar[JAG_TOP_OPEN_LOOP]=leave at 0
		in.jaguar[JAG_TOP_FEEDBACK]=jag_at_speed(shooter_sim.estimate().top);
		//in.jaguar[JAG_BOTTOM_OPEN_LOOP]=
		in.jaguar[JAG_BOTTOM_FEEDBACK]=jag_at_speed(shooter_sim.estimate().bottom);
		stringstream ss;
		auto out_now=m(in,ss);
		shooter_sim.update(in.now,shooter_output(out_now),out_now.solenoid[4]);
		string change;
		change+=inputs.update(in);
		change+=state.update(m);
		change+=outputs.update(out_now);
		if(change.size()){
			o<<"Now="<<in.now<<"\n";
			o<<change<<"\n";
		}
		v.push_back(m.control_status);
	}
	return v;
}

void mode_table(){
	static ofstream f("control_modes.html");
	struct Tag{
		string s;
		Tag(string s1):s(s1){
			f<<"<"<<s<<">";
			s=split(s1)[0];
		}

		~Tag(){
			f<<"</"<<s<<">";
		}
	};
	Tag t("html");

	Tag b("body");
	Tag a("table border");
	{
		Tag a("tr");
		{
			Tag b("th");
			f<<"Control status";
		}
		Tag c("th");
		f<<"Mode";
	}
	for(auto control_status:Control_status::all()){
		Tag r("tr");
		{
			Tag d("td");
			f<<control_status;
		}
		Tag d("td");
		f<<Toplevel::to_mode(control_status);
	}
}

void mode_diagram(){
	ofstream f("control_modes.dot");
	f<<"digraph G{\n";
	for(auto a:Control_status::all()){
		string x=as_string(a),y=as_string(Toplevel::to_mode(a));
		if(x!=y){
			f<<"\t"<<a<<"->"<<Toplevel::to_mode(a)<<"\n";
		}
	}
	f<<"}\n";
}

void check_auto_modes_end(){
	for(auto control_status:Control_status::all()){
		if(teleop(control_status)) continue;
		auto n=next(control_status,Toplevel::Status(),Joystick_data(),Panel(),0,0,0,Shooter_wheels::Calibration(),10);
		cout<<control_status<<"	"<<n<<endl;
		assert(teleop(n));
	}
}

template<typename T>
vector<T> uniq(vector<T> v){
	vector<T> r;
	for(auto a:v){
		if( !(r.size() && r[r.size()-1]==a) ){
			r|=a;
		}
	}
	return r;
}

int main(){
	check_auto_modes_end();
	/*Main m;
	cout<<m<<"\n";
	cout<<m(Robot_inputs())<<"\n";
	cout<<m<<"\n";
	float a;
	getDistance(2);
	for(float i = 0.26; i <= 3; i = i+0.5){
		cout<<i<<" \n"; 
		getDistance(i);
	}*/
	/*cout<<timetowall(2.5,1)<<"\n";
	cout<<newdistance(.2,3,.7,2,.5)<<"\n";
	for(float input=0;input<3;input=input+0.1){
		a=converttodistance(input);
		cout<<input<<"	"<<a<<"\n";
	}*/
	/*
	cout<<func(0, 1, 0)<<"\n";
	cout<<func(0, -1, 0)<<"\n";
	cout<<func(0, 0, 1)<<"\n";
	cout<<func(0, 0, -1)<<"\n";
	cout<<func(1, 0, 0)<<"\n";
	cout<<func(-1, 0, 0)<<"\n";
	cout<<func(0, 0, 0)<<"\n";
	*/
	bool test_control_status=1;
	if(test_control_status){
		for(Control_status::Control_status control_status:Control_status::all()){
			cout<<control_status<<" "<<Toplevel::to_mode(control_status)<<"\n";
		}
	}
	auto a=auto_test(cout,.18);
	auto b=auto_test(cout,.51);
	auto c=auto_test(cout,.84);
	auto d=auto_test(cout,1.17);
	cout<<uniq(a)<<"\n";
	cout<<uniq(b)<<"\n";
	cout<<uniq(c)<<"\n";
	cout<<uniq(d)<<"\n";
	mode_table();
	mode_diagram();
}
#endif
