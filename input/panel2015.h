#ifndef PANEL2015_H//bool piston_switch_b; PANEL2015_H
#define PANEL2015_H 

#include "../util/maybe.h"
#include "../control/lift.h"
#include "../control/kicker.h"
#include "../util/interface.h"
#include "../control/combo_lift.h"

struct Panel{
	enum class Auto_mode{
		DO_NOTHING,MOVE,FULL_RUN
	};
	Auto_mode auto_mode;
	enum class Level_buttons{
		LEVEL0,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5,LEVEL6
	};
	Level_buttons level_buttons;
	enum class Operation_buttons{
		COLLECT_CURRENT,DROP_CURRENT,MOVE_COLLECT,MOVE_DROP
	};
	Operation_buttons operation_buttons;
	float slide_pos;
    //Buttons:
        bool move_arm_to_pos;
        bool current_collect;
        bool current_drop;
        bool move_drop;
        bool move_collect;
        bool chute_collect;
        bool lifter_off;
        bool piston_aligner;
        bool kicker_activate;
    //3 position Switches:
        int target_type;
        int move_arm_one;
        int move_arm_cont;
        int collect_mode;
        int drop_mode;
    	
	Panel();
};
std::ostream& operator<<(std::ostream&,Panel::Auto_mode);
std::ostream& operator<<(std::ostream&,Panel::Level_buttons);
//std::ostream& operator<<(std::ostream&,Panel::Slide_pos);
std::ostream& operator<<(std::ostream&,Panel);

Panel interpret(Driver_station_input);

#endif
