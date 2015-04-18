#include "panel2015.h"
#include <iostream>
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"
#include <cmath>

using namespace std;

Panel::Panel():
	in_use(0),
	auto_mode(Auto_mode::DO_NOTHING),
	level_button(Level_button::DEFAULT),
	operation_buttons(Operation_buttons::DEFAULT),
	slide_pos(0.0),
	override_height(0),
	move_arm_to_pos(0),
	can_nudge_small(0),
	engage_kicker_height(0),
	tote_nudge(0),
	can_nudge(0),
	stop(0),
	target_type(0),
	move_arm_one(0),
	move_arm_cont(0),
	bottom_mode(1)
{}

ostream& operator<<(ostream& o,Panel::Auto_mode a){
	o<<" Auto_Mode(";
	if(a==Panel::Auto_mode::DO_NOTHING)o<<"do_nothing";
	else if(a==Panel::Auto_mode::MOVE)o<<"move";
	else if(a==Panel::Auto_mode::CAN_GRAB)o<<"can_grab";
	else assert(0);
	o<<")";
	return o;
}

ostream& operator<<(ostream& o,Panel::Level_button a){
	o<<" Level_button(";
	if(a==Panel::Level_button::DEFAULT)o<<"default";
	else if(a==Panel::Level_button::LEVEL0)o<<"0";
	else if(a==Panel::Level_button::LEVEL1)o<<"1";
	else if(a==Panel::Level_button::LEVEL2)o<<"2";
	else if(a==Panel::Level_button::LEVEL3)o<<"3";
	else if(a==Panel::Level_button::LEVEL4)o<<"4";
	else if(a==Panel::Level_button::LEVEL5)o<<"5";	
	else if(a==Panel::Level_button::LEVEL6)o<<"6";
	else assert(0);
	o<<")";
	return o;
}

ostream& operator<<(ostream& o,Panel::Operation_buttons a){
	o<<" Operation_buttons(";
	if(a==Panel::Operation_buttons::CAN_NUDGE_SMALL)o<<"can_nudge_small";
	else if(a==Panel::Operation_buttons::ENGAGE_KICKER_HEIGHT)o<<"engage_kicker_height";
	else if(a==Panel::Operation_buttons::TOTE_NUDGE)o<<"tote_nudge";
	else if(a==Panel::Operation_buttons::CAN_NUDGE)o<<"can_nudge";
	else if(a==Panel::Operation_buttons::DEFAULT)o<<"default";
	else assert(0);
	o<<")";
	return o;
}

ostream& operator<<(ostream& o,Panel p){
	o<<"Panel(";
	o<<p.in_use;
	o<<" "<<p.auto_mode;
	o<<", "<<p.level_button;
	o<<", "<<p.operation_buttons;
	o<<", slide_pos:"<<p.slide_pos;
	o<<", override_height:"<<p.override_height;
	o<<", Buttons(";
	o<<" move_arm_to_pos:"<<p.move_arm_to_pos;
	o<<", can_nudge_small:"<<p.can_nudge_small;
	o<<", engage_kicker_height:"<<p.engage_kicker_height;
	o<<", tote_nudge:"<<p.tote_nudge;
	o<<", can_nudge:"<<p.can_nudge;
	o<<", stop:"<<p.stop;
	o<<", 3_pos_switches(";
	o<<" target_type:"<<p.target_type;
	o<<", move_arm_one:"<<p.move_arm_one;
	o<<", move_arm_cont:"<<p.move_arm_cont;
	o<<", bottom_mode:"<<p.bottom_mode<<")";
	o<<")";
	return o;
}

Panel::Auto_mode auto_mode_convert(int potin){
	switch(potin) {
		case 1:
			return Panel::Auto_mode::MOVE;
		case 2:
			return Panel::Auto_mode::CAN_GRAB;
		default:
			return Panel::Auto_mode::DO_NOTHING;
	}
}

Panel interpret(Joystick_data d){
	Panel panel;
	{
		panel.in_use=[&](){
			for(int i=0;i<JOY_AXES;i++) {
				if(d.axis[i]!=0)return 1;
			}
			for(int i=0;i<JOY_BUTTONS;i++) {
				if(d.button[i]!=0)return 1;
			}
			return 0;
		}();
	}
	{
		Volt auto_mode=d.axis[5];
		panel.auto_mode=auto_mode_convert(interpret_10_turn_pot(auto_mode));
	}
	{
		float lev=d.axis[1];//Default=-1
		static const float DEFAULT=-1,LEVEL0=-.75,LEVEL1=-.5,LEVEL2=-.25,LEVEL3=0,LEVEL4=.32,LEVEL5=.65,LEVEL6=1;
		if(1/*!d.button[1]*/){//tests if override is being pushed
			if(lev<DEFAULT+.1)panel.level_button=Panel::Level_button::DEFAULT;
			else if(lev>LEVEL0-(LEVEL0-DEFAULT)/2 && lev<LEVEL0+(LEVEL1-LEVEL0)/2)panel.level_button=Panel::Level_button::LEVEL0;//-.75
			else if(lev>LEVEL1-(LEVEL1-LEVEL0)/2 && lev<LEVEL1+(LEVEL2-LEVEL1)/2)panel.level_button=Panel::Level_button::LEVEL1;//-.5
			else if(lev>LEVEL2-(LEVEL2-LEVEL1)/2 && lev<LEVEL2+(LEVEL3-LEVEL2)/2)panel.level_button=Panel::Level_button::LEVEL2;//-.25
			else if(lev>LEVEL3-(LEVEL3-LEVEL2)/2 && lev<LEVEL3+(LEVEL4-LEVEL3)/2)panel.level_button=Panel::Level_button::LEVEL3;//0
			else if(lev>LEVEL4-(LEVEL4-LEVEL3)/2 && lev<LEVEL4+(LEVEL5-LEVEL4)/2)panel.level_button=Panel::Level_button::LEVEL4;//.32
			else if(lev>LEVEL5-(LEVEL5-LEVEL4)/2 && lev<LEVEL5+(LEVEL6-LEVEL5)/2)panel.level_button=Panel::Level_button::LEVEL5;//.65
			else if(lev>LEVEL6-(LEVEL6-LEVEL5)/2 && lev<LEVEL6+.25)panel.level_button=Panel::Level_button::LEVEL6;//1
			else panel.level_button=Panel::Level_button::DEFAULT;
		}else{//This sets it to the SlipnSlide
			panel.override_height=(d.axis[2]+1)*((65-5)/2);
		}
	}
	{
		float op=d.axis[0];//default: -1
		static const float CAN_NUDGE_SMALL=1,TOTE_NUDGE=.65,CAN_NUDGE=.32,ENGAGE_KICKER_HEIGHT=0, DEFAULT=-1;
		if(op>ENGAGE_KICKER_HEIGHT-(ENGAGE_KICKER_HEIGHT-DEFAULT)/2 && op<ENGAGE_KICKER_HEIGHT+(CAN_NUDGE-ENGAGE_KICKER_HEIGHT)/2)panel.operation_buttons=Panel::Operation_buttons::ENGAGE_KICKER_HEIGHT;//0
		else if(op>CAN_NUDGE-(CAN_NUDGE-ENGAGE_KICKER_HEIGHT)/2 && op<CAN_NUDGE+(TOTE_NUDGE-CAN_NUDGE)/2)panel.operation_buttons=Panel::Operation_buttons::CAN_NUDGE;//.32
		else if(op>TOTE_NUDGE-(TOTE_NUDGE-CAN_NUDGE)/2 && op<TOTE_NUDGE+(CAN_NUDGE_SMALL-TOTE_NUDGE)/2)panel.operation_buttons=Panel::Operation_buttons::TOTE_NUDGE;//.65
		else if(op>CAN_NUDGE_SMALL-(CAN_NUDGE_SMALL-TOTE_NUDGE)/2 && op<CAN_NUDGE_SMALL+.25)panel.operation_buttons=Panel::Operation_buttons::CAN_NUDGE_SMALL;
		else panel.operation_buttons=Panel::Operation_buttons::DEFAULT;
	}
	panel.can_nudge=panel.operation_buttons==Panel::Operation_buttons::CAN_NUDGE;
	panel.tote_nudge=(panel.operation_buttons==Panel::Operation_buttons::TOTE_NUDGE);
	panel.can_nudge_small=(panel.operation_buttons==Panel::Operation_buttons::CAN_NUDGE_SMALL);
	panel.engage_kicker_height=panel.operation_buttons==Panel::Operation_buttons::ENGAGE_KICKER_HEIGHT;
	//panel.slide_pos=(d.analog[2]+1)*((65-5)/2);//May be useless due to previous things
	panel.stop=d.button[3];
	{	
		static const float DOWN=1, UP=.48, DEFAULT=-1;
		float updowncontrol=d.axis[4];
		if(updowncontrol>UP-(UP-DEFAULT)/2 && updowncontrol<UP+(DOWN-UP)/2) panel.move_arm_cont=1;
		else if(updowncontrol>DOWN-(DOWN-UP)/2 && updowncontrol<DOWN+.25 ) panel.move_arm_cont=-1;
		else panel.move_arm_cont=0;		
	}
	{
		static const float DOWN=0,UP=-.5,DEFAULT=-1;;
		float level_up_down_control=d.axis[4];
		if(level_up_down_control>UP-(UP-DEFAULT)/2 && level_up_down_control<UP+(DOWN-UP)/2)panel.move_arm_one=1;
		if(level_up_down_control>DOWN-(DOWN-UP)/2 && level_up_down_control<DOWN+.25)panel.move_arm_one=-1;
		else panel.move_arm_one=0;
	}
	panel.bottom_mode=round(d.axis[6]);
	panel.target_type=round(d.axis[3]);
	return panel;
}

#ifdef PANEL2015_TEST
Joystick_data driver_station_input_rand(){//Copied over from hammer. Adrian update this if needed.
	Joystick_data r;
	for(unsigned i=0;i<JOY_AXES;i++){
		r.axis[i]=(0.0+rand()%101)/100;
	}
	for(unsigned i=0;i<JOY_BUTTONS;i++){
		r.button[i]=rand()%2;
	}
	return r;
}

int main(){
	Panel p;
	for(unsigned i=0;i<50;i++){
		interpret(driver_station_input_rand());
	}
	cout<<p<<"\n";
	return 0;
}
#endif
