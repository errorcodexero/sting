#ifndef LCD_SCROLLER_H
#define LCD_SCROLLER_H

#include<iosfwd>
#include "../util/posedge_trigger.h"
#include "../util/driver_station_interface.h"

class Lcd_scroller{
	unsigned top_line;
	Posedge_trigger up,down;
	Driver_station_output::Lcd out;

	public:
	Lcd_scroller();

	void update(std::string const&,bool up,bool down);
	Driver_station_output::Lcd get()const;

	friend std::ostream& operator<<(std::ostream&,Lcd_scroller);
};
std::ostream& operator<<(std::ostream&,Lcd_scroller);

#endif
