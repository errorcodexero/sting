#ifndef PANEL2015_H
#define PANEL2015_H 

struct Level_buttons{
	bool  
}

struct Panel{
	enum Auto_mode{
		DO_NOTHING,MOVE,FULL_RUN
	};
	Auto_mode auto_mode;
	bool lifter_off;
    bool piston_aligner;
    //bool piston_switch_b;
    bool level_buttons [5];
	
	
	Panel();
};





Panel interpret(Driver_station_input);

#endif
