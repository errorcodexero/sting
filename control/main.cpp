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
	
	r.solenoid[0]=(a.collector_tilt==Collector_tilt::OUTPUT_DOWN);
	r.solenoid[1]=(a.collector_tilt==Collector_tilt::OUTPUT_UP);
	r.solenoid[2]=r.solenoid[3]=(a.injector==Injector::OUTPUT_DOWN);
	r.solenoid[4]=r.solenoid[5]=(a.injector==Injector::OUTPUT_UP);
	r.solenoid[6]=(a.ejector==Ejector::OUTPUT_UP);
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
	X(collector_tilt)
	X(injector)
	//X(injector_arms)
	X(ejector)
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

string fRel(bool field){
	if(field){
		return "True";
	}
	return "False";
}

void log_line(ostream& o,Robot_inputs in,Main m,Robot_outputs out){
	o<<in.now;
	#define X(name) o<<","<<name;
	X(in.robot_mode.autonomous)
	X(in.robot_mode.enabled)
	/*for(unsigned i=0;i<Joystick_data::AXES;i++){
		X(in.joystick[0].axis[i])
	}
	for(unsigned i=0;i<Joystick_data::BUTTONS;i++){
		X(in.joystick[0].button[i])
	}*/
	X(in.joystick[0])
	X(in.digital_io[0]) //pressure switch
	for(unsigned i=0;i<Robot_outputs::CAN_JAGUARS;i++){
		X(in.jaguar[i].speed)
	}
	/*for(unsigned i=0;i<Driver_station_input::ANALOG_INPUTS;i++){
		X(in.driver_station.analog[i])
	}
	for(unsigned i=0;i<Driver_station_input::DIGITAL_INPUTS;i++){
		X(in.driver_station.digital[i])
	}*/
	X(in.driver_station)

	//going to not put in the interpreted version of this in case need to diagnose a noise problem or something.  
	
	//put in the state stuff here...
	//skipping any force stuff because I don't think our drivers know how to use it.  Would be detectable via joystic inputs.
	X(m.control_status)
	Toplevel::Status e=m.est.estimate();
	X(e)
	/*X(e.collector_tilt)
	X(e.injector)
	X(e.injector_arms)
	X(e.ejector)
	X(e.shooter_wheels)*/
	//skip pump
	//skip orientation

	//X(m.wheel_calibration) -> this might be good to do
	#define Y(n) X(m.wheel_calibration.calib.n)
	Y(highgoal)
	Y(lowgoal)
	Y(overtruss)
	Y(passing)
	#undef Y

	for(unsigned i=0;i<4;i++){ //others aren't used.
		X((int)out.pwm[i])
	}
	for(unsigned i=0;i<Robot_outputs::SOLENOIDS;i++){
		X(out.solenoid[i])
	}
	for(unsigned i=0;i<Robot_outputs::RELAYS && i<2;i++){
		X(out.relay[i])
	}
	for(unsigned i=0;i<Robot_outputs::CAN_JAGUARS;i++){
		X(out.jaguar[i])
	}
	X(out.driver_station.digital[7])//ready light
	#undef X
	o<<"\n";
}

struct Log_entry{
	Time time;
	Robot_mode robot_mode;
	Joystick_data driver_joystick;
	bool pressure_switch;

	//Jaguar_input jaguar_in[Robot_outputs::CAN_JAGUARS];
	double jaguar_speed[Robot_outputs::CAN_JAGUARS];
	Driver_station_input driver_station;
	Control_status::Control_status control_status;
	Toplevel::Status toplevel_status;
	//wheelcalib wheel_calibration;

	Pwm_output pwm[4];
	Solenoid_output solenoid[Robot_outputs::SOLENOIDS];
	Relay_output relay[2];
	Jaguar_output jaguar_out[Robot_outputs::CAN_JAGUARS];
	bool ready_light;
};

ostream& operator<<(ostream& o,Log_entry a){
	o<<"Log_entry(";
	#define X(name) o<<""#name<<":"<<a.name<<" ";
	X(time)
	X(robot_mode)
	X(driver_joystick)
	X(pressure_switch)
	X(jaguar_speed)
	X(driver_station)
	X(control_status)
	X(toplevel_status)
	//X(wheel_calibration)
	for(unsigned i=0;i<4;i++){
		X(pwm[i])
	}
	for(unsigned i=0;i<Robot_outputs::SOLENOIDS;i++){
		X(solenoid[i])
	}
	for(unsigned i=0;i<2;i++) X(relay[i])
	for(unsigned i=0;i<Robot_outputs::CAN_JAGUARS;i++){
		assert(0);
	}
	return o<<")";
}

Maybe<Log_entry> parse_log_entry(string s){
	vector<string> v=split(s,',');
	for(unsigned i=0;i<v.size();i++){
		cout<<i<<":"<<v[i]<<endl;
	}
	Log_entry r;
	r.time=atof(v.at(0));
	r.robot_mode.autonomous=atoi(v.at(1).c_str());
	r.robot_mode.enabled=atoi(v.at(2).c_str());
	{
		Maybe<Joystick_data> m=Joystick_data::parse(v.at(3));
		if(!m) return Maybe<Log_entry>();
		r.driver_joystick=*m;
	}
	r.pressure_switch=atoi(v.at(4).c_str());
	for(unsigned i=0;i<Robot_outputs::CAN_JAGUARS;i++){
		string s=v.at(5+i);
		/*Maybe<Jaguar_input> m=Jaguar_input::parse(s);
		cout<<s<<endl;
		r.jaguar_in[i]=*m;*/
		r.jaguar_speed[i]=atof(s.c_str());
	}
	{
		Maybe<Driver_station_input> m=Driver_station_input::parse(v.at(9));
		if(!m){
			cout<<v.at(9)<<endl;
		}
		r.driver_station=*m;
	}
	{
		Maybe<Control_status::Control_status> m=Control_status::parse(v.at(10));
		r.control_status=*m;
	}
	{
		stringstream ss;
		ss<<v.at(11)<<","<<v.at(12);
		Maybe<Toplevel::Status> m=Toplevel::parse_status(ss.str());
		if(!m) cout<<v.at(11)<<endl;
		r.toplevel_status=*m;
	}
	cout<<r<<"\n";
	assert(0);
	/*r.wheel_calibration=
	r.pwm[i]=
	r.solenoid[i]=
	r.relay[i]=
	r.jaguar[i]=*/
	return Maybe<Log_entry>(r);
}

Robot_outputs Main::operator()(Robot_inputs in,ostream& cerr){
	gyro.update(in.now,in.analog[0]);
	perf.update(in.now);
	since_switch.update(in.now,0);
	Joystick_data gunner_joystick=in.joystick[1];
	Joystick_data main_joystick=in.joystick[0];
	force.update(
		main_joystick.button[Gamepad_button::A],
		main_joystick.button[Gamepad_button::LB],
		main_joystick.button[Gamepad_button::RB],
		main_joystick.button[Gamepad_button::BACK],
		main_joystick.button[Gamepad_button::B],
		main_joystick.button[Gamepad_button::X]
	);


	ball_collecter.update(main_joystick.button[5]);
	bool tanks_full=(in.digital_io[0]==DI_1);

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
	if(toplevel_status.injector!=Injector::Estimator::DOWN_IDLE&&toplevel_status.injector!=Injector::Estimator::DOWN_VENT){
		subgoals_now.injector_arms=Injector_arms::GOAL_CLOSE;
	}
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
		bool downsensor=in.digital_io[1]==DI_1;
		est.update(in.now,in.robot_mode.enabled,high_level_outputs,tanks_full?Pump::FULL:Pump::NOT_FULL,in.orientation,wheel,downsensor);
	}

        // Print out wheel RPMs:
		//static int rpm_update_cnt = 0;
/*	if (rpm_update_cnt == 100)
        {
            rpm_update_cnt = 0;
            cerr << "top wheel rpm = " << in.jaguar[JAG_TOP_FEEDBACK].speed
                 << "	bottom wheel rpm = " << in.jaguar[JAG_BOTTOM_FEEDBACK].speed
                 << endl;
        }
        else
        {
            rpm_update_cnt++;
        }*/

	// Turn on camera light in autonomous mode (kForward):
	light.update ( main_joystick.button[Gamepad_button::X] );
	bool ledOn = in.robot_mode.autonomous || light.get();
	r.relay[1] = r.relay[6] = (ledOn) ? Relay_output::_10 : Relay_output::_00;
	

	r=force(r);
	
	r.driver_station.lcd.line[0]=as_string(panel.auto_mode);
	r.driver_station.lcd.line[1]=as_string(r.jaguar[0]).substr(13, 20)+as_string(panel.pidselect);
	r.driver_station.lcd.line[2]=as_string(r.jaguar[1]).substr(13, 20);
	r.driver_station.lcd.line[3]=as_string(r.jaguar[2]).substr(13, 20);
	r.driver_station.lcd.line[4]=as_string(r.jaguar[3]).substr(13, 20);
	stringstream strin;
	strin<<toplevel_status.shooter_wheels;
	r.driver_station.lcd.line[5]="Speeds:"+strin.str().substr(22, 20);

	//r.driver_station.lcd=format_for_lcd(as_string(in.now)+as_string(in.driver_station));
	/*r.driver_station.lcd=format_for_lcd(
		//abbreviate_text(as_string(in.now)+"\n"+as_string(panel)+as_string(in.driver_station))
		as_string(subgoals_now)+as_string(toplevel_status)
	);*/
	//r.driver_station.lcd=format_for_lcd(as_string(panel));
	r.driver_station.digital[7]=ready(toplevel_status,subgoals_now);
	{
		static int i=0;
		if(i==0){
			stringstream ss;
			ss<<in<<"\r\n"<<*this<<mode<<"\r\n";
			ss<<r<<"\r\n"; 
			//ss<<panel<<"\r\n";
			//ss<<in.driver_station<<"\r\n";
			//ss<<"Field Relative?:"<<field_relative.get()<<"\n";
			//ss<<"Gyro ="<<in.orientation<<"\n";
			cerr<<ss.str();//putting this all together at once in hope that it'll show up at closer to the same time.  
			//cerr<<subgoals_now<<high_level_outputs<<"\n";
		}
		i=(i+1)%100;
	}
	//cerr<<subgoals_now<<"\r\n";
	//cerr<<toplevel_status<<"\r\n\r\n";
	//cerr<<"Waiting on:"<<not_ready(toplevel_status,subgoals_now)<<"\n";
	
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
		a.gyro==b.gyro && 
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
	o<<m.gyro;
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
		case JOY_LEFT: return Fire_control::TRUSS;
		case JOY_RIGHT: return Fire_control::AUTO_SHOT;
		case JOY_UP: return Fire_control::HIGH;
		case JOY_DOWN: return Fire_control::EJECT;
		default: break;
	}
	if(mode_buttons.truss_toss) return Fire_control::TRUSS;
	if(mode_buttons.shoot_high) return Fire_control::HIGH;
	if(mode_buttons.auto_shot) return Fire_control::AUTO_SHOT;
	if(mode_buttons.eject) return Fire_control::EJECT;
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
		fire_now=(vert==JOY_UP) || panel.fire;
		fire_when_ready=(vert==JOY_DOWN); //No equivalent on the switchpanel.
	}

	bool ready_to_shoot=ready(part_status,subgoals(Toplevel::SHOOT_HIGH_PREP,Drive_goal(),calib));
	bool ready_to_truss_toss=ready(part_status,subgoals(Toplevel::TRUSS_TOSS_PREP,Drive_goal(),calib));
	bool ready_to_collect=ready(part_status,subgoals(Toplevel::COLLECT,Drive_goal(),calib));
	bool ready_to_auto_shot=ready(part_status,subgoals(Toplevel::AUTO_SHOT_PREP,Drive_goal(),calib));
	bool took_shot=location_to_status(part_status.injector)==Injector::RECOVERY;
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
					return (part_status.collector_tilt==Collector_tilt::STATUS_UP)?A2_FIRE2:A2_MOVE;
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
		case EJECT: return (location_to_status(part_status.ejector)==Ejector::RECOVERY)?DRIVE_WO_BALL:EJECT;
		case EJECT_WHEN_READY:
		{
			bool ready_to_eject=(location_to_status(part_status.ejector)==Ejector::IDLE);
			return ready_to_eject?EJECT:EJECT_WHEN_READY;
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
	if(!approx_equal(a.gyro,b.gyro)) return 0;
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

void log_line_test(){
	stringstream ss;
	Robot_inputs in;
	Main m;
	Robot_outputs out;
	log_line(ss,in,m,out);
	cout<<ss.str()<<"\n";
	parse_log_entry(ss.str());
	exit(1);
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
	//log_line_test();
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
