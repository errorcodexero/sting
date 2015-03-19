#include "panel2015.h"
#include <stdlib.h> 
#include "util.h"
#include "../util/util.h"

using namespace std;

Panel::Panel():
	auto_mode(Auto_mode::DO_NOTHING),
	level_buttons(Level_buttons::LEVEL0),
	operation_buttons(Operation_buttons::DROP_CURRENT),
	slide_pos(0.0),
	move_arm_to_pos(0),
	current_collect(0),
	move_drop(0),
	move_collect(0),
	chute_collect(0),
	lifter_off(0),
	piston_aligner(0),
	kicker_activate(0),
	target_type(0),
	move_arm_one(0),
	move_arm_cont(0),
	collect_mode(0),
	drop_mode(0)
{}

ostream& operator<<(ostream& o, Panel::Auto_mode a){
	if(a==Panel::Auto_mode::DO_NOTHING)o<<"Auto-Mode:DO_NOTHING ";
	if(a==Panel::Auto_mode::MOVE)o<<"Auto-Mode:MOVE ";
	if(a==Panel::Auto_mode::FULL_RUN)o<<"Auto-Mode:FULL_RUN ";
	return o;
}

ostream& operator<<(ostream& o, Panel::Level_buttons a){
	o<<"Level:";
	if(a==Panel::Level_buttons::LEVEL0)o<<"0 ";
	if(a==Panel::Level_buttons::LEVEL1)o<<"1 ";
	if(a==Panel::Level_buttons::LEVEL2)o<<"2 ";
	if(a==Panel::Level_buttons::LEVEL3)o<<"3 ";
	if(a==Panel::Level_buttons::LEVEL4)o<<"4 ";
	if(a==Panel::Level_buttons::LEVEL5)o<<"5 ";	
	if(a==Panel::Level_buttons::LEVEL6)o<<"6 ";
	return o;
}
/*ostream& operator<<(ostream& o, Panel::Slide_pos a){
	o<<"Arm Slider="<<a*20<<"% ";
	return o;
}*/

Panel::Auto_mode automodeconvert(int potin){
	if(potin==0)return Panel::Auto_mode::DO_NOTHING;
	if(potin==1)return Panel::Auto_mode::MOVE;
	if(potin==2)return Panel::Auto_mode::FULL_RUN;
	return Panel::Auto_mode::DO_NOTHING;
}

Panel interpret(Driver_station_input d){
	Panel panel;
	{
	Volt volt=d.analog[0]/3.3*5;
	int i=interpret_10_turn_pot(volt);
	panel.auto_mode=automodeconvert(i);
	}
	{
	//Operation Buttons
	float op=d.analog[2];
	if(op<=1&&op>=.5)panel.level_buttons=Panel::Level_buttons::LEVEL0;
	if(op>1&&op<=1.8)panel.level_buttons=Panel::Level_buttons::LEVEL1;
	if(op>1.8&&op<=2.5)panel.level_buttons=Panel::Level_buttons::LEVEL2;
	if(op>2.5&&op<=3)panel.level_buttons=Panel::Level_buttons::LEVEL3;
	if(op>3&&op<=3.8)panel.level_buttons=Panel::Level_buttons::LEVEL4;
	if(op>3.8&&op<4.5)panel.level_buttons=Panel::Level_buttons::LEVEL5;
	if(op>4.5&&op<5)panel.level_buttons=Panel::Level_buttons::LEVEL6; 
    else panel.level_buttons=panel.level_buttons;
	}
	return panel;
}

#ifdef PANEL2015_TEST
int main(){

	return 0;
}
#endif
