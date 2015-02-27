#ifndef    //bool piston_switch_b; PANEL2015_H
#define PANEL2015_H 

struct Level_buttons{
	bool  
}

struct Panel{
	enum Auto_mode{
		DO_NOTHING,MOVE,FULL_RUN
	};
	Auto_mode auto_mode;
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
    bool operation_buttons [4];
    bool level_buttons [5];
    //3 position Switches:
    int target_type;
    int move_arm_one;
    int move_arm_cont;
    int collect_mode;
    int drop_mode;
    
	
	Panel();
};



Panel interpret(Driver_station_input);

#endif