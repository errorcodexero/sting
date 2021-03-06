#ifndef HEIGHT_H
#define HEIGHT_H

#include<array>
#include<vector>

struct Lift_position{
	bool pickup,is_can,on_step,placed_on_scoring,engage_kicker;
	unsigned int stacked_bins;
	Lift_position();
};
std::ostream& operator<<(std::ostream&,Lift_position const&);
std::vector<Lift_position> lift_positions();

std::array<float,3> find_height(Lift_position const&);

#endif
