#include "main.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <stdlib.h>
#include "toplevel.h"
#include "../util/util.h"
#include "toplevel_mode.h"
#include "../input/util.h"
#include "height.h"

using namespace std;

//todo: at some point, might want to make this whatever is right to start autonomous mode.
Main::Main():mode(Mode::TELEOP),autonomous_start(0),sticky_can_goal(Sticky_can_goal::STOP),sticky_tote_goal(Sticky_tote_goal::STOP),can_priority(1){}

double set_drive_speed(Joystick_data joystick,int axis,double boost,double slow,bool turning=0){
	static const float DEFAULT_SPEED=.55;//Change these value to change the default speed
	static const float TURNING_SLOW=.33;
	static const float SLOW_BY=.5;//Change this value to change the percentage of the default speed the slow button slows
	return pow(joystick.axis[axis],3)*((DEFAULT_SPEED+(1-DEFAULT_SPEED)*boost)-(((DEFAULT_SPEED*SLOW_BY)*slow)+((DEFAULT_SPEED*TURNING_SLOW)*turning)));
}

template<typename T>//Compares two types to see if one is within a range
bool in_range(T a,T b,T c){
	return a<b+c&&a>b-c;
}

Lift::Goal tote_lifter(Lift_position& tote_lift_pos,float ENGAGE_KICKER_HEIGHT,Main::Sticky_tote_goal pre_sticky_tote_goal,Posedge_toggle& piston,bool kick_and_lift=1){//Auto kicking code 
	static const float ABOVE_ENGAGE_KICKER_HEIGHT=ENGAGE_KICKER_HEIGHT+3;
	if(kick_and_lift && pre_sticky_tote_goal==Main::Sticky_tote_goal::ENGAGE_KICKER && !piston.get() && find_height(tote_lift_pos)[2]>=ABOVE_ENGAGE_KICKER_HEIGHT) piston.update(1);
	return Lift::Goal::go_to_height(std::array<double,3>{find_height(tote_lift_pos)[0],find_height(tote_lift_pos)[1],find_height(tote_lift_pos)[2]});
}

float round_to_level(float tote_height,float height){
	static const unsigned int NUMBER_OF_LEVELS=6;
	for(unsigned int i=0;i<NUMBER_OF_LEVELS;i++){
		if(in_range(height,tote_height*i,tote_height/2))return i;
	}
	return 0;
}

Main::Sticky_tote_goal convert_level(Panel::Level_button level_button) {
	switch (level_button) {
		case Panel::Level_button::DEFAULT:
			return Main::Sticky_tote_goal::STOP;
		case Panel::Level_button::LEVEL0:
			return Main::Sticky_tote_goal::BOTTOM;
		case Panel::Level_button::LEVEL1:
			return Main::Sticky_tote_goal::LEVEL1;
		case Panel::Level_button::LEVEL2:
			return Main::Sticky_tote_goal::LEVEL2;
		case Panel::Level_button::LEVEL3:
			return Main::Sticky_tote_goal::LEVEL3;
		case Panel::Level_button::LEVEL4:
			return Main::Sticky_tote_goal::LEVEL4;
		case Panel::Level_button::LEVEL5:
			return Main::Sticky_tote_goal::LEVEL5;
		case Panel::Level_button::LEVEL6:
			return Main::Sticky_tote_goal::TOP;
		default: assert(0);
	}
}

Toplevel::Goal Main::teleop(
	Robot_inputs const& in,
	Joystick_data const& main_joystick,
	Joystick_data const& gunner_joystick,
	Panel const&  oi_panel,
	Toplevel::Status_detail& toplevel_status
){
	cout<<toplevel_status<<"\n";
	Toplevel::Goal goals;

	static const float X_NUDGE_POWER=.45;//Change these nudge values to adjust the nudge speeds/amounts
	static const float Y_NUDGE_POWER=.2;

	static const float ROTATE_NUDGE_POWER=.5;

	static const float BACK_TURN_POWER=.2;
	static const float BACK_MOVE_POWER=.5;
	static const bool SLOW_TURNING=0;
	
	Drivebase::Goal &goal=goals.drive;
	if(!nudges[0].timer.done())goal.x=-X_NUDGE_POWER;
	else if(!nudges[1].timer.done())goal.x=X_NUDGE_POWER;
	else goal.x=main_joystick.axis[Gamepad_axis::LEFTX];

	const double turbo_button=main_joystick.axis[Gamepad_axis::LTRIGGER];

	if(!nudges[2].timer.done())goal.y=-Y_NUDGE_POWER;
	else if(!nudges[3].timer.done())goal.y=Y_NUDGE_POWER;
	else if(!back_turns[0].timer.done()||!back_turns[1].timer.done())goal.y=BACK_MOVE_POWER;
	else goal.y=set_drive_speed(main_joystick,1,turbo_button,main_joystick.axis[Gamepad_axis::RTRIGGER]);

	if(!nudges[4].timer.done())goal.theta=-ROTATE_NUDGE_POWER;
	else if(!nudges[5].timer.done()) goal.theta=ROTATE_NUDGE_POWER;
	else if(!back_turns[0].timer.done())goal.theta=BACK_TURN_POWER;
	else if(!back_turns[1].timer.done())goal.theta=-BACK_TURN_POWER;
	else goal.theta=-set_drive_speed(main_joystick,4,turbo_button,main_joystick.axis[Gamepad_axis::RTRIGGER],SLOW_TURNING);//theta is /2 so rotation is reduced to prevent bin tipping.

	const bool normal_nudge_enable=turbo_button<.25;			
	static const auto NUDGE_LEFT_BUTTON=Gamepad_button::X,NUDGE_RIGHT_BUTTON=Gamepad_button::B;
	static const auto NUDGE_CCW_BUTTON=Gamepad_button::RB,NUDGE_CW_BUTTON=Gamepad_button::LB;
	static const auto NUDGE_FWD_BUTTON=Gamepad_button::Y,NUDGE_BACK_BUTTON=Gamepad_button::A;
	static const unsigned int nudge_buttons[6]={NUDGE_LEFT_BUTTON,NUDGE_RIGHT_BUTTON,NUDGE_FWD_BUTTON,NUDGE_BACK_BUTTON,NUDGE_CCW_BUTTON,NUDGE_CW_BUTTON};
	for(int i=0;i<6;i++){
		bool start=nudges[i].trigger(normal_nudge_enable&&main_joystick.button[nudge_buttons[i]]);
		if(start)nudges[i].timer.set(.1);
		nudges[i].timer.update(in.now,1);
	}
	bool kick_and_lift=1;
	//auto nudge!
	if(!normal_nudge_enable){
		/*todo: add the part where we actually read the sensors
		if(	
			(main_joystick.button[NUDGE_LEFT_BUTTON]&&in.digital_io.in[8]==Digital_in::_0)||
			(main_joystick.button[NUDGE_CW_BUTTON]&&in.digital_io.in[9]==Digital_in::_0)
		){
			goal.x=X_NUDGE_POWER;
		}
		if(
			(main_joystick.button[NUDGE_RIGHT_BUTTON]&&in.digital_io.in[7]==Digital_in::_0)||
			(main_joystick.button[NUDGE_CCW_BUTTON]&&in.digital_io.in[9]==Digital_in::_0)
		){
			goal.x=-X_NUDGE_POWER;
		}*/
		if(main_joystick.button[NUDGE_FWD_BUTTON]){
			//kick_and_lift=0;
			if(piston.get()){
				piston.update(1);
			}
			bool left=in.digital_io.in[7]==Digital_in::_1;
			bool right=in.digital_io.in[8]==Digital_in::_1;
			if(!left&&!right){
				goal.y=-Y_NUDGE_POWER*1.5;
			}else{
				if(!left){
					goal.theta=ROTATE_NUDGE_POWER/2;
					goal.y=-Y_NUDGE_POWER/2;
				}
				if(!right){
					goal.theta=-ROTATE_NUDGE_POWER/2;
					goal.y=-Y_NUDGE_POWER/2;
				}
			}
			if(left && right){
				sticky_tote_goal=Sticky_tote_goal::ENGAGE_KICKER;
			}
		}
	}
	
	static const unsigned int back_turn_buttons[2]={Gamepad_button::BACK,Gamepad_button::START};
	for(int i=0;i<2;i++){
		bool start=back_turns[i].trigger(main_joystick.button[back_turn_buttons[i]]);
		if(start)back_turns[i].timer.set(1);
		back_turns[i].timer.update(in.now,1);
	}
	
	goals.drive=goal;
	
	piston.update(gunner_joystick.button[Gamepad_button::Y]);
	
	Lift_position can_lift_pos;
	can_lift_pos.is_can=1;
	Lift_position tote_lift_pos;

	bool down2=gunner_joystick.button[Gamepad_button::LB];
	
	static const double TOTE_HEIGHT=12.1;
	pre_sticky_tote_goal=sticky_tote_goal;
	goals.combo_lift.can=[&](){
		if(!gunner_joystick.button[Gamepad_button::BACK]){
			if(gunner_joystick.button[Gamepad_button::B]){
				sticky_can_goal=Sticky_can_goal::STOP;
				can_priority=1;
			}
			if(gunner_joystick.button[Gamepad_button::R_JOY]){
				sticky_can_goal=Sticky_can_goal::BOTTOM;
				can_priority=1;
			}
			if(gunner_joystick.axis[Gamepad_axis::RTRIGGER]>0){
				can_priority=1;
				sticky_can_goal=Sticky_can_goal::TOP;
			}
			Joystick_section section=joystick_section(gunner_joystick.axis[Gamepad_axis::RIGHTX],gunner_joystick.axis[Gamepad_axis::RIGHTY]);
			switch(section){
				case Joystick_section::DOWN:
					sticky_can_goal=Sticky_can_goal::LEVEL1;
					can_priority=1;
					break;
				case Joystick_section::LEFT:
					sticky_can_goal=Sticky_can_goal::LEVEL2;
					can_priority=1;
					break;
				case Joystick_section::RIGHT:
					sticky_can_goal=Sticky_can_goal::LEVEL3;
					can_priority=1;
					break;
				case Joystick_section::UP:
					sticky_can_goal=Sticky_can_goal::LEVEL4;
					can_priority=1;
					break;
				case Joystick_section::CENTER:
					break;
				default: assert(0);
			}
			if(gunner_joystick.button[Gamepad_button::START]){
				sticky_can_goal=Sticky_can_goal::LEVEL5;
				can_priority=1;
			}
		}else{
			sticky_can_goal=Sticky_can_goal::STOP;
		}
		if(sticky_can_goal==Sticky_can_goal::STOP) return Lift::Goal::stop();
		if(sticky_can_goal==Sticky_can_goal::BOTTOM) return Lift::Goal::down();
		if(sticky_can_goal==Sticky_can_goal::TOP) return Lift::Goal::up();
		if(sticky_can_goal==Sticky_can_goal::UP_LEVEL) can_lift_pos.stacked_bins=round_to_level(TOTE_HEIGHT,toplevel_status.combo_lift.can.inches_off_ground())+1;
		if(sticky_can_goal==Sticky_can_goal::DOWN_LEVEL) can_lift_pos.stacked_bins=round_to_level(TOTE_HEIGHT,toplevel_status.combo_lift.can.inches_off_ground())-1;
		if(sticky_can_goal==Sticky_can_goal::LEVEL1) can_lift_pos.stacked_bins=1;
		if(sticky_can_goal==Sticky_can_goal::LEVEL2) can_lift_pos.stacked_bins=2;
		if(sticky_can_goal==Sticky_can_goal::LEVEL3) can_lift_pos.stacked_bins=3;
		if(sticky_can_goal==Sticky_can_goal::LEVEL4) can_lift_pos.stacked_bins=4;
		if(sticky_can_goal==Sticky_can_goal::LEVEL5) can_lift_pos.stacked_bins=5;
		//if(sticky_can_goal==Sticky_can_goal::LEVEL6) can_lift_pos.stacked_bins=6;
		double offset=down2?-5:0;
		#define X(name) if(sticky_can_goal==Sticky_can_goal::name){ \
			return Lift::Goal::go_to_height(std::array<double,3>{find_height(can_lift_pos)[0]+offset,find_height(can_lift_pos)[1]+offset,find_height(can_lift_pos)[2]+offset}); \
		}
		X(LEVEL1) X(LEVEL2) X(LEVEL2) X(LEVEL3) X(LEVEL4) X(LEVEL5) /*X(LEVEL6)*/  X(DOWN_LEVEL) X(UP_LEVEL)
		#undef X
		return Lift::Goal::stop();

	}();
	goals.combo_lift.tote=[&](){
		static const float ENGAGE_KICKER_HEIGHT=2.9;
		if(!gunner_joystick.button[Gamepad_button::BACK]){
			if(gunner_joystick.button[Gamepad_button::B]){
				sticky_tote_goal=Sticky_tote_goal::STOP;
				can_priority=0;
			}
			if(gunner_joystick.button[Gamepad_button::L_JOY]){
				can_priority=0;
				sticky_tote_goal=Sticky_tote_goal::BOTTOM;
			}
			/*if(gunner_joystick.button[Gamepad_button::BACK]){
				sticky_tote_goal=Sticky_tote_goal::TOP;
				can_priority=0;
			}*/
			if(gunner_joystick.button[Gamepad_button::RB]){
				sticky_tote_goal=Sticky_tote_goal::ENGAGE_KICKER;
				can_priority=0;
			}
			Joystick_section section=joystick_section(gunner_joystick.axis[Gamepad_axis::LEFTX],gunner_joystick.axis[Gamepad_axis::LEFTY]);
			switch (section){
				case Joystick_section::DOWN:
					{
					sticky_tote_goal=Sticky_tote_goal::LEVEL1;
					can_priority=0;
					break;
					}
				case Joystick_section::LEFT:
					{
					sticky_tote_goal=Sticky_tote_goal::LEVEL2;
					can_priority=0;
					break;
					}
				case Joystick_section::RIGHT:
					{
					sticky_tote_goal=Sticky_tote_goal::LEVEL3;
					can_priority=0;
					break;
					}
					case Joystick_section::UP:
					{
					sticky_tote_goal=Sticky_tote_goal::LEVEL4;
					can_priority=0;
					}
					break;
				case Joystick_section::CENTER:
					{
					bool input=[&](){
						for(int i=0;i<JOY_AXES;i++) {
							if(oi_panel.values.axis[i]!=0)return 1;
						}
						for(int i=0;i<JOY_BUTTONS;i++) {
							if(oi_panel.values.button[i]!=0)return 1;
						}
						return 0;
					}();
					if(input) {
						Main::Sticky_tote_goal temp_level=convert_level(oi_panel.level_button);
						if(temp_level!=Main::Sticky_tote_goal::STOP) {
							sticky_tote_goal=temp_level;
							can_priority=0;
						}
					}	
					break;
					}
				default: assert(0);
			}
			if(gunner_joystick.axis[Gamepad_axis::LTRIGGER]>0){
				sticky_tote_goal=Sticky_tote_goal::LEVEL5;
				can_priority=0;
			}
		}else{
			sticky_tote_goal=Sticky_tote_goal::STOP;
		}
		if(sticky_tote_goal==Sticky_tote_goal::STOP) return Lift::Goal::stop();
		if(sticky_tote_goal==Sticky_tote_goal::BOTTOM) return Lift::Goal::down();
		if(sticky_tote_goal==Sticky_tote_goal::TOP) return Lift::Goal::up();
		if(sticky_tote_goal==Sticky_tote_goal::UP_LEVEL) tote_lift_pos.stacked_bins=round_to_level(TOTE_HEIGHT,toplevel_status.combo_lift.tote.inches_off_ground())+1;
		if(sticky_tote_goal==Sticky_tote_goal::DOWN_LEVEL) tote_lift_pos.stacked_bins=round_to_level(TOTE_HEIGHT,toplevel_status.combo_lift.tote.inches_off_ground())-1;
		if(sticky_tote_goal==Sticky_tote_goal::ENGAGE_KICKER) tote_lift_pos.engage_kicker=1;
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL1) tote_lift_pos.stacked_bins=1;
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL2) tote_lift_pos.stacked_bins=2;
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL3) tote_lift_pos.stacked_bins=3;
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL4) tote_lift_pos.stacked_bins=4;
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL5) tote_lift_pos.stacked_bins=5;
		//if(sticky_tote_goal==Sticky_tote_goal::LEVEL6) tote_lift_pos.stacked_bins=6;
		cout<<endl<<" 2: "<<(pre_sticky_tote_goal==Main::Sticky_tote_goal::ENGAGE_KICKER)<<" 3: "<<(!piston.get())<<" 4: "<<(find_height(tote_lift_pos)[2]>=ENGAGE_KICKER_HEIGHT+1);
		#define X(name) if(sticky_tote_goal==Sticky_tote_goal::name) return tote_lifter(tote_lift_pos,ENGAGE_KICKER_HEIGHT,pre_sticky_tote_goal,piston,kick_and_lift);
		X(ENGAGE_KICKER) X(LEVEL1) X(LEVEL2) X(LEVEL2) X(LEVEL3) X(LEVEL4) X(LEVEL5) /*X(LEVEL6)*/  X(DOWN_LEVEL)  X(UP_LEVEL)
		#undef X
		return Lift::Goal::stop();
		
	}();	
	goals.combo_lift.can_priority=can_priority;
	goals.kicker=[&](){
		if(piston.get()){
			return Kicker::Goal::OUT;
		}
		return Kicker::Goal::IN;
	}();

	return goals;
}

unsigned pdb_location1(Drivebase::Motor m){
	#define X(NAME,INDEX) if(m==Drivebase::NAME) return INDEX;
	X(LEFT1,12)
	X(LEFT2,13)
	X(RIGHT1,14)
	X(RIGHT2,15)
	X(CENTER1,2)
	X(CENTER2,3)
	#undef X
	assert(0);
	//assert(m>=0 && m<Drivebase::MOTORS);
}

Main::Mode next_mode(Main::Mode m,bool autonomous,bool autonomous_start,Toplevel::Status_detail status,Time since_switch, Panel::Auto_mode auto_mode){
	switch(m){
		case Main::Mode::TELEOP:
			if(autonomous_start){
				//todo: make this depend on a switch or something.
				switch(auto_mode){ 
					case Panel::Auto_mode::CAN_GRAB:
						return Main::Mode::AUTO_GRAB;
					case Panel::Auto_mode::MOVE:
						return Main::Mode::AUTO_MOVE;
					case Panel::Auto_mode::DO_NOTHING:
						return Main::Mode::TELEOP;
					default: assert(0);
				}
			}
			return m;
		case Main::Mode::AUTO_MOVE:
			//encoders? going to use time for now
			if(!autonomous || since_switch>1.7) return Main::Mode::TELEOP;
			return m;
		case Main::Mode::AUTO_GRAB:
			if(!autonomous) return Main::Mode::TELEOP;
			if(status.can_grabber.status==Can_grabber::Status::DOWN) return Main::Mode::AUTO_BACK;
			return m;
		case Main::Mode::AUTO_BACK:
			if(!autonomous) return Main::Mode::TELEOP;
			//timer is up - could use encoders once those work
			if(since_switch>2) return Main::Mode::AUTO_RELEASE;
			return m;
		case Main::Mode::AUTO_RELEASE:
			if(status.can_grabber.status==Can_grabber::Status::STUCK_UP || !autonomous) return Main::Mode::TELEOP;
			return m;
		default: assert(0);
	}
}

Robot_outputs Main::operator()(Robot_inputs in,ostream&){
	perf.update(in.now);
	Joystick_data main_joystick=in.joystick[0];
	Joystick_data gunner_joystick=in.joystick[1];
	Panel oi_panel=interpret(in.joystick[2]);
	force.update(
		main_joystick.button[Gamepad_button::A],
		main_joystick.button[Gamepad_button::LB],
		main_joystick.button[Gamepad_button::RB],
		main_joystick.button[Gamepad_button::BACK],
		main_joystick.button[Gamepad_button::B],
		main_joystick.button[Gamepad_button::X]
	);
	
	Toplevel::Status_detail toplevel_status=toplevel.estimator.get();
	
	if (in.digital_io.encoder[0]) cout<<"Wheel 1: "<<*in.digital_io.encoder[0]<<"\n";
	if (in.digital_io.encoder[1]) cout<<"Wheel 2: "<<*in.digital_io.encoder[1]<<"\n";
	if (in.digital_io.encoder[2]) cout<<"Wheel 3: "<<*in.digital_io.encoder[2]<<"\n";
	bool autonomous_start_now=autonomous_start(in.robot_mode.autonomous && in.robot_mode.enabled);
	since_auto_start.update(in.now,autonomous_start_now);
	//static const Time AUTONOMOUS_MODE_LENGTH=10;
		
	if(!in.robot_mode.enabled || in.robot_mode.autonomous) sticky_tote_goal=Sticky_tote_goal::STOP;

	Toplevel::Goal goals;
	//Drivebase::Status_detail status_detail = drivebase.estimator.get();
	
	switch(mode){
		case Mode::TELEOP:
			goals=teleop(in,main_joystick,gunner_joystick,oi_panel,toplevel_status);
			break;
		case Mode::AUTO_MOVE:
			goals.drive.x=0;
			goals.drive.y=-.6;
			goals.drive.theta=0;
			break;
		case Mode::AUTO_GRAB:
			goals.can_grabber=Can_grabber::Goal::BOTTOM;
			break;
		case Mode::AUTO_BACK:
			goals.can_grabber=Can_grabber::Goal::BOTTOM;
			goals.drive.y=.6;
			break;
		case Mode::AUTO_RELEASE:
			goals.can_grabber=Can_grabber::Goal::TOP;
			break;
		default: assert(0);
	}

	auto next=next_mode(mode,in.robot_mode.autonomous,autonomous_start_now,toplevel_status,since_switch.elapsed(),oi_panel.auto_mode);
	since_switch.update(in.now,mode!=next);
	mode=next;
	//cout<<"Can: "<<lift_can<<endl;
	//cout<<"Tote: "<<lift_tote<<endl;	
	//Drivebase::Output out = control(status_detail, goal);

	Toplevel::Output r_out=control(toplevel_status,goals); 

	auto r=toplevel.output_applicator(Robot_outputs{},r_out);
	r=force(r);

	/*Lift::Input can_input;
	can_input.top=in.talon_srx[1].fwd_limit_switch;
	can_input.bottom=in.talon_srx[1].rev_limit_switch;
	can_input.ticks=in.talon_srx[1].encoder_position;
	Lift::Input tote_input;
	tote_input.top=in.talon_srx[0].fwd_limit_switch;
	tote_input.bottom=in.talon_srx[0].rev_limit_switch;
	tote_input.ticks=in.talon_srx[0].encoder_position;

	Drivebase::Input drive_in{[&](){
		array<double,Drivebase::MOTORS> r;
		for(unsigned i=0;i<Drivebase::MOTORS;i++){
			Drivebase::Motor m=(Drivebase::Motor)i;
			r[i]=in.current[pdb_location1(m)];
		}
		return r;
	}()};*/

	auto input=toplevel.input_reader(in);

	toplevel.estimator.update(
		in.now,
		input,
		/*Toplevel::Input{
			{can_input,tote_input},
			Kicker::Input{},
			drive_in,
			Pump::Input::NOT_FULL,
			Can_grabber::Input{1}, //todo: make this actually ready from a digital io
			Tote_sensors::Input{0,0,0}
		},*/
		toplevel.output_applicator(r)
	);
	return r;
}

bool operator==(Main a,Main b){
	return a.force==b.force && 
		a.perf==b.perf && 
		a.toplevel==b.toplevel && 
		a.since_switch==b.since_switch && 
		a.since_auto_start==b.since_auto_start &&
		a.autonomous_start==b.autonomous_start;
}

bool operator!=(Main a,Main b){
	return !(a==b);
}

ostream& operator<<(ostream& o,Main m){
	o<<"Main(";
	o<<m.force;
	o<<m.perf;
	o<<m.toplevel;
	o<<m.since_switch;
	//since_auto_start
	//o<<m.control;
	//ball collector
	//print button
	//relative
	//field relative
	return o<<")";
}

bool approx_equal(Main a,Main b){
	if(a.force!=b.force) return 0;
	if(a.toplevel!=b.toplevel) return 0;
	return 1;
}

#ifdef MAIN_TEST
#include<fstream>
#include "monitor.h"

template<typename T>
vector<T> uniq(vector<T> v){
	vector<T> r;
	for(auto a:v){
		if(!(r.size()&&r[r.size()-1]==a)){
			r|=a;
		}
	}
	return r;
}

int main(){
}
#endif
