#include "panel2015.h"
#include <iostream>
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

ostream& operator<<(ostream& o,Panel::Auto_mode a){
	o<<" Auto_Mode(";
	if(a==Panel::Auto_mode::DO_NOTHING)o<<"do_nothing";
	if(a==Panel::Auto_mode::MOVE)o<<"move";
	if(a==Panel::Auto_mode::FULL_RUN)o<<"full_run";
	o<<")";
	return o;
}

ostream& operator<<(ostream& o,Panel::Level_buttons a){
	o<<" Level(";
	if(a==Panel::Level_buttons::LEVEL0)o<<"0";
	if(a==Panel::Level_buttons::LEVEL1)o<<"1";
	if(a==Panel::Level_buttons::LEVEL2)o<<"2";
	if(a==Panel::Level_buttons::LEVEL3)o<<"3";
	if(a==Panel::Level_buttons::LEVEL4)o<<"4";
	if(a==Panel::Level_buttons::LEVEL5)o<<"5";	
	if(a==Panel::Level_buttons::LEVEL6)o<<"6";
	o<<")";
	return o;
}

enum class Operation_buttons{
		COLLECT_CURRENT,DROP_CURRENT,MOVE_COLLECT,MOVE_DROP
	};
ostream& operator<<(ostream& o,Panel::Operation_buttons a){
	o<<" Operation_buttons(";
	if(a==Panel::Operation_buttons::COLLECT_CURRENT)o<<"collect_current";
	if(a==Panel::Operation_buttons::DROP_CURRENT)o<<"drop_current";
	if(a==Panel::Operation_buttons::MOVE_COLLECT)o<<"move_collect";
	if(a==Panel::Operation_buttons::MOVE_DROP)o<<"move_drop";
	o<<")";
	return o;
}

ostream& operator<<(ostream& o,Panel p){
	o<<"Panel(";
	o<<" "<<p.auto_mode;
	o<<", "<<p.level_buttons;
	o<<", "<<p.operation_buttons;
	o<<", slide_pos:"<<p.slide_pos;
	o<<", Buttons(";
	o<<" move_arm_to_pos:"<<p.move_arm_to_pos;
	o<<", collect_current:"<<p.current_collect;
	o<<", current_drop:"<<p.current_drop;
	o<<", move_drop:"<<p.move_drop;
	o<<", move_collect:"<<p.move_collect;
	o<<", chute_collect:"<<p.chute_collect;
	o<<", lifter_off:"<<p.lifter_off;
	o<<", piston_aligner:"<<p.piston_aligner;
	o<<", kicker_activate:"<<p.kicker_activate<<")";
	o<<", 3_pos_switches(";
	o<<" target_type:"<<p.target_type;
	o<<", move_arm_one"<<p.move_arm_one;
	o<<", move_arm_cont"<<p.move_arm_cont;
	o<<", collect_mode"<<p.collect_mode;
	o<<", drop_mode"<<p.drop_mode<<")";
	o<<")";
    return o;
}

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
	float op=d.analog[1];
	if(op<=1&&op>=.5)panel.level_buttons=Panel::Level_buttons::LEVEL0;//-.75
	else if(op>1&&op<=1.8)panel.level_buttons=Panel::Level_buttons::LEVEL1;//-.5
	else if(op>1.8&&op<=2.5)panel.level_buttons=Panel::Level_buttons::LEVEL2;//-.25
	else if(op>2.5&&op<=3)panel.level_buttons=Panel::Level_buttons::LEVEL3;//0
	else if(op>3&&op<=3.8)panel.level_buttons=Panel::Level_buttons::LEVEL4;//.32
	else if(op>3.8&&op<4.5)panel.level_buttons=Panel::Level_buttons::LEVEL5;//.65
	else if(op>4.5&&op<5)panel.level_buttons=Panel::Level_buttons::LEVEL6;//1
	}
	return panel;
}

Driver_station_input driver_station_input_rand(){//Copied over from hammer. Adrian update this if needed.
	Driver_station_input r;
	for(unsigned i=0;i<r.ANALOG_INPUTS;i++){
		r.analog[i]=(0.0+rand()%101)/100;
	}
	for(unsigned i=0;i<r.DIGITAL_INPUTS;i++){
		r.digital[i]=rand()%2;
	}
	return r;
}

#ifdef PANEL2015_TEST
int main(){
	Panel p;
	for(unsigned i=0;i<50;i++){
		interpret(driver_station_input_rand());
	}
	cout<<p<<"\n";
	return 0;
}
#endif
