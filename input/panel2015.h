#ifndef PANEL2015_H
#define PANEL2015_H 

#include "../util/maybe.h"
#include "../control/lift.h"
#include "../control/kicker.h"
#include "../util/interface.h"
#include "../control/combo_lift.h"

struct Panel{
	bool in_use;
	enum class Auto_mode{DO_NOTHING,MOVE,CAN_GRAB};//Added
	Auto_mode auto_mode;//Added
	enum class Level_button{DEFAULT,LEVEL0,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5,LEVEL6,ENGAGE_KICKER_HEIGHT};//Added
	Level_button level_button;//Added
	enum class Operation_buttons{KILL,DROP_CURRENT,MOVE_COLLECT,MOVE_DROP};
	Operation_buttons operation_buttons;
	float slide_pos;
	float override_height;
    //Buttons:
	bool move_arm_to_pos;
	bool kill;
	bool current_drop;
	bool move_drop;
	bool move_collect;
	bool chute_collect;
	bool stop;//Added
	bool piston_aligner;
	bool kicker_activate;
    //3 position Switches:
	int target_type;//Added
	int move_arm_one;
	int move_arm_cont;
	int collect_mode;
	int drop_mode;
	Panel();
};

std::ostream& operator<<(std::ostream&,Panel::Auto_mode);
std::ostream& operator<<(std::ostream&,Panel::Level_button);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret(Joystick_data);

#endif
