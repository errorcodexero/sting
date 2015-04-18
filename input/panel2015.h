#ifndef PANEL2015_H
#define PANEL2015_H 

#include "../util/maybe.h"
#include "../control/lift.h"
#include "../control/kicker.h"
#include "../util/interface.h"
#include "../control/combo_lift.h"

struct Panel{
	bool in_use;//Added
	enum class Auto_mode{DO_NOTHING,MOVE,CAN_GRAB};//Added
	Auto_mode auto_mode;//Added
	enum class Level_button{DEFAULT,LEVEL0,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5,LEVEL6};
	Level_button level_button;//Added
	enum class Operation_buttons{DEFAULT,CAN_NUDGE_SMALL,ENGAGE_KICKER_HEIGHT,TOTE_NUDGE,MOVE_COLLECT,CAN_NUDGE};//Added
	Operation_buttons operation_buttons;//Added
	float slide_pos;
	float override_height;
    //Buttons:
	bool move_arm_to_pos;
	bool can_nudge_small;//added
	bool engage_kicker_height;//added
	bool tote_nudge;//Added
	bool can_nudge;//Added
	bool chute_collect;
	bool stop;//Added
	bool piston_aligner;
    //3 position Switches:
	int target_type;//Added
	int move_arm_one;
	int move_arm_cont;
	int bottom_mode;//added
	Panel();
};

std::ostream& operator<<(std::ostream&,Panel::Auto_mode);
std::ostream& operator<<(std::ostream&,Panel::Level_button);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret(Joystick_data);

#endif
