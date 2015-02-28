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

using namespace std;

//todo: at some point, might want to make this whatever is right to start autonomous mode.
Main::Main():mode(Mode::TELEOP),autonomous_start(0),sticky_can_goal(Sticky_can_goal::STOP),sticky_tote_goal(Sticky_tote_goal::STOP),can_priority(1){}

double set_drive_speed(Joystick_data joystick, int axis, double boost, double slow){
	static const float DEFAULT_SPEED=.55;//Change these value to change the default speed
	static const float SLOW_BY=.5;//Change this value to change the percentage of the default speed the slow button slows
	return pow(joystick.axis[axis], 3)*((DEFAULT_SPEED+(1-DEFAULT_SPEED)*boost)-(DEFAULT_SPEED*SLOW_BY)*slow);
}

Lift::Goal tote_lifter(Toplevel::Goal &goals,float level,float ENGAGE_KICKER_HEIGHT,Toplevel::Status_detail &toplevel_status,bool kick_and_lift=1){
	if(kick_and_lift && toplevel_status.combo_lift.tote.inches_off_ground()==ENGAGE_KICKER_HEIGHT) goals.kicker=Kicker::Goal::OUT;
	return Lift::Goal::go_to_height(level);
}

void Main::teleop(
	Robot_inputs const& in,
	Joystick_data const& main_joystick,
	Joystick_data const& gunner_joystick,
	Toplevel::Goal &goals,
	Toplevel::Status_detail &toplevel_status
){
	static const float X_NUDGE_POWER=.45;//Change these nudge values to adjust the nudge speeds/amounts
	static const float Y_NUDGE_POWER=.2;
	static const float ROTATE_NUDGE_POWER=.15;

	static const float BACK_TURN_POWER=.2;
	static const float BACK_MOVE_POWER=.5;

	Drivebase::Goal &goal=goals.drive;
	if (!nudges[0].timer.done()) goal.x=-X_NUDGE_POWER;
	else if (!nudges[1].timer.done()) goal.x=X_NUDGE_POWER;
	else goal.x=main_joystick.axis[Gamepad_axis::LEFTX];

	if (!nudges[2].timer.done()) goal.y=-Y_NUDGE_POWER;
	else if (!nudges[3].timer.done()) goal.y=Y_NUDGE_POWER;
	else if (!back_turns[0].timer.done() || !back_turns[1].timer.done()) goal.y=BACK_MOVE_POWER;
	else goal.y=set_drive_speed(main_joystick, 1, main_joystick.axis[Gamepad_axis::LTRIGGER], main_joystick.axis[Gamepad_axis::RTRIGGER]);

	if (!nudges[4].timer.done()) goal.theta=-ROTATE_NUDGE_POWER;
	else if (!nudges[5].timer.done()) goal.theta=ROTATE_NUDGE_POWER;
	else if (!back_turns[0].timer.done()) goal.theta=BACK_TURN_POWER;
	else if (!back_turns[1].timer.done()) goal.theta=-BACK_TURN_POWER;
	else goal.theta=-set_drive_speed(main_joystick, 4, main_joystick.axis[Gamepad_axis::LTRIGGER], main_joystick.axis[Gamepad_axis::RTRIGGER]);//theta is /2 so rotation is reduced to prevent bin tipping.
			
	static const unsigned int nudge_buttons[6]={Gamepad_button::X,Gamepad_button::B,Gamepad_button::Y,Gamepad_button::A,Gamepad_button::RB,Gamepad_button::LB};
	for (int i=0;i<6;i++) {
		bool start=nudges[i].trigger(main_joystick.button[nudge_buttons[i]]);
		if (start) nudges[i].timer.set(.1);
		nudges[i].timer.update(in.now,1);
	}
			
	static const unsigned int back_turn_buttons[2]={Gamepad_button::BACK,Gamepad_button::START};
	for (int i=0;i<2;i++) {
		bool start=back_turns[i].trigger(main_joystick.button[back_turn_buttons[i]]);
		if (start) back_turns[i].timer.set(1);
		back_turns[i].timer.update(in.now,1);
	}
	
	goals.drive=goal;
	static const double LEVEL = 13.5;
	goals.combo_lift.can=[&](){
		if(gunner_joystick.button[Gamepad_button::B]){
			sticky_can_goal=Sticky_can_goal::STOP;
			can_priority=1;
		}
		
		if(gunner_joystick.button[Gamepad_button::R_JOY]){
			sticky_can_goal=Sticky_can_goal::BOTTOM;
			can_priority=1;
		}
		Joystick_section section = joystick_section(gunner_joystick.axis[Gamepad_axis::RIGHTX],gunner_joystick.axis[Gamepad_axis::RIGHTY]);
		switch (section) {
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
		if(gunner_joystick.axis[Gamepad_axis::RTRIGGER]>0){
			can_priority=1;
			sticky_can_goal=Sticky_can_goal::TOP;
		}
		if(sticky_can_goal==Sticky_can_goal::STOP) return Lift::Goal::stop();
		if(sticky_can_goal==Sticky_can_goal::BOTTOM) return Lift::Goal::down();
		if(sticky_can_goal==Sticky_can_goal::LEVEL1) return Lift::Goal::go_to_height(LEVEL);
		if(sticky_can_goal==Sticky_can_goal::LEVEL2) return Lift::Goal::go_to_height((2*LEVEL));
		if(sticky_can_goal==Sticky_can_goal::LEVEL3) return Lift::Goal::go_to_height((3*LEVEL));
		if(sticky_can_goal==Sticky_can_goal::LEVEL4) return Lift::Goal::go_to_height((4*LEVEL));
		if(sticky_can_goal==Sticky_can_goal::LEVEL5) return Lift::Goal::go_to_height((5*LEVEL));
		if(sticky_can_goal==Sticky_can_goal::TOP) return Lift::Goal::up();
		return Lift::Goal::stop();
	}();
	goals.combo_lift.tote=[&](){
		static const float ENGAGE_KICKER_HEIGHT=2.9;
		if(gunner_joystick.button[Gamepad_button::B]){
			sticky_tote_goal=Sticky_tote_goal::STOP;
			can_priority=0;
		}
		/*if(gunner_joystick.button[Gamepad_button::LB]){
			sticky_tote_goal=Sticky_tote_goal::STOP;
			return Lift::Goal::up();
		}*/
		if(gunner_joystick.button[Gamepad_button::RB]){
			sticky_tote_goal=Sticky_tote_goal::ENGAGE_KICKER;
			can_priority=0;
			//return Lift::Goal::up();
		}
		if(gunner_joystick.button[Gamepad_button::L_JOY]){
			can_priority=0;
			sticky_tote_goal=Sticky_tote_goal::BOTTOM;
		}
		Joystick_section section = joystick_section(gunner_joystick.axis[Gamepad_axis::LEFTX],gunner_joystick.axis[Gamepad_axis::LEFTY]);
		switch (section) {
			case Joystick_section::DOWN:
				sticky_tote_goal=Sticky_tote_goal::LEVEL1;
				can_priority=0;
				break;
			case Joystick_section::LEFT:
				sticky_tote_goal=Sticky_tote_goal::LEVEL2;
				can_priority=0;
				break;
			case Joystick_section::RIGHT:
				sticky_tote_goal=Sticky_tote_goal::LEVEL3;
				can_priority=0;
				break;
			case Joystick_section::UP:
				sticky_tote_goal=Sticky_tote_goal::LEVEL4;
				can_priority=0;
				break;
			case Joystick_section::CENTER:
				break;
			default:
				assert(0);
		}
		if(gunner_joystick.button[Gamepad_button::BACK]){
			sticky_tote_goal=Sticky_tote_goal::LEVEL5;
			can_priority=0;
		}
		if(gunner_joystick.axis[Gamepad_axis::LTRIGGER]>0){
			sticky_tote_goal=Sticky_tote_goal::TOP;
			can_priority=0;
		}
		if(sticky_tote_goal==Sticky_tote_goal::STOP) return Lift::Goal::stop();
		if(sticky_tote_goal==Sticky_tote_goal::BOTTOM) return Lift::Goal::down();
		if(sticky_tote_goal==Sticky_tote_goal::ENGAGE_KICKER) return Lift::Goal::go_to_height(ENGAGE_KICKER_HEIGHT);
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL1) return tote_lifter(goals,(1*LEVEL),ENGAGE_KICKER_HEIGHT,toplevel_status);
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL2) return tote_lifter(goals,(2*LEVEL),ENGAGE_KICKER_HEIGHT,toplevel_status);
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL3) return tote_lifter(goals,(3*LEVEL),ENGAGE_KICKER_HEIGHT,toplevel_status);
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL4) return tote_lifter(goals,(4*LEVEL),ENGAGE_KICKER_HEIGHT,toplevel_status);
		if(sticky_tote_goal==Sticky_tote_goal::LEVEL5) return tote_lifter(goals,(5*LEVEL),ENGAGE_KICKER_HEIGHT,toplevel_status);
		if(sticky_tote_goal==Sticky_tote_goal::TOP) return Lift::Goal::up();
		return Lift::Goal::stop();
	}();
	goals.combo_lift.can_priority=can_priority;
	piston.update(gunner_joystick.button[Gamepad_button::Y]);
	goals.kicker=[&](){
		if(piston.get()){
			return Kicker::Goal::OUT;
		}
		return Kicker::Goal::IN;
	}();
}

Robot_outputs Main::operator()(Robot_inputs in,ostream&){
	perf.update(in.now);
	Joystick_data main_joystick=in.joystick[Gamepad_axis::LEFTX];
	Joystick_data gunner_joystick=in.joystick[Gamepad_axis::LEFTY];
	since_switch.update(in.now,0);
	force.update(
		main_joystick.button[Gamepad_button::A],
		main_joystick.button[Gamepad_button::LB],
		main_joystick.button[Gamepad_button::RB],
		main_joystick.button[Gamepad_button::BACK],
		main_joystick.button[Gamepad_button::B],
		main_joystick.button[Gamepad_button::X]
	);
	
	Toplevel::Status_detail toplevel_status=toplevel.estimator.get();
	
	if (in.digital_io.encoder[0]) cout<<"Wheel 1: "<<*in.digital_io.encoder[0]<<endl;
	if (in.digital_io.encoder[1]) cout<<"Wheel 2: "<<*in.digital_io.encoder[1]<<endl;
	if (in.digital_io.encoder[2]) cout<<"Wheel 3: "<<*in.digital_io.encoder[2]<<endl;
	bool autonomous_start_now=autonomous_start(in.robot_mode.autonomous && in.robot_mode.enabled);
	if(autonomous_start_now) mode=Mode::AUTO_MOVE;
	since_auto_start.update(in.now,autonomous_start_now);
	//static const Time AUTONOMOUS_MODE_LENGTH=10;
		
	Toplevel::Goal goals;
	//Drivebase::Status_detail status_detail = drivebase.estimator.get();
	
	if(!in.robot_mode.enabled || in.robot_mode.autonomous) sticky_tote_goal=Sticky_tote_goal::STOP;
	if(1 || mode==Mode::TELEOP){
		teleop(in,main_joystick,gunner_joystick,goals,toplevel_status);
	}
	else if(mode==Mode::AUTO_MOVE){
		goals.drive.x=0;
		goals.drive.y=0;
		goals.drive.theta=0;
	}
	//cout<<"Can: "<<lift_can<<endl;
	//cout<<"Tote: "<<lift_tote<<endl;	
	//Drivebase::Output out = control(status_detail, goal);

	Toplevel::Output r_out=control(toplevel_status,goals); 

	auto r=toplevel.output_applicator(Robot_outputs{},r_out);
	r=force(r);

	Lift::Input can_input;
	can_input.top=in.talon_srx[1].fwd_limit_switch;
	can_input.bottom=in.talon_srx[1].rev_limit_switch;
	can_input.ticks=in.talon_srx[1].encoder_position;
	Lift::Input tote_input;
	tote_input.top=in.talon_srx[0].fwd_limit_switch;
	tote_input.bottom=in.talon_srx[0].rev_limit_switch;
	tote_input.ticks=in.talon_srx[0].encoder_position;

	toplevel.estimator.update(
		in.now,
		Toplevel::Input{
			can_input,
			tote_input,
			Kicker::Input{},
			Drivebase::Input{},
			Pump::Input::NOT_FULL,
			Can_grabber::Input{1} //todo: make this actually ready from a digital io
		},
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
		if( !(r.size() && r[r.size()-1]==a) ){
			r|=a;
		}
	}
	return r;
}

int main(){
}
#endif
