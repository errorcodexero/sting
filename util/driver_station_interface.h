#ifndef DRIVER_STATION_INTERFACE_H
#define DRIVER_STATION_INTERFACE_H

#include<iosfwd>
#include<string>
#include<vector>
#include<bitset>
#include "maybe.h"

struct Driver_station_output{
	static const unsigned DIGITAL_OUTPUTS=8;
	std::bitset<DIGITAL_OUTPUTS> digital;
	
	struct Lcd{
		static const unsigned HEIGHT=6;
		static const unsigned WIDTH=21;
		std::string line[HEIGHT];
	};
	Lcd lcd;
};
bool operator==(Driver_station_output::Lcd,Driver_station_output::Lcd);
bool operator!=(Driver_station_output::Lcd,Driver_station_output::Lcd);
std::ostream& operator<<(std::ostream&,Driver_station_output::Lcd);
bool operator==(Driver_station_output,Driver_station_output);
bool operator!=(Driver_station_output,Driver_station_output);
std::ostream& operator<<(std::ostream&,Driver_station_output);

std::vector<std::string> to_lines(std::string const&,unsigned width);
Driver_station_output::Lcd format_for_lcd(std::string const&);

struct Driver_station_input{
	//In the traditional mode of the Cybress board there are only 4 available, but there are 8 in the 'enhanced' mode
	static const unsigned ANALOG_INPUTS=8;
	double analog[ANALOG_INPUTS];

	static const unsigned DIGITAL_INPUTS=8;
	std::bitset<DIGITAL_INPUTS> digital;

	Driver_station_input();

	static Maybe<Driver_station_input> parse(std::string const&);
	static Driver_station_input rand();
};
bool operator==(Driver_station_input,Driver_station_input);
bool operator!=(Driver_station_input,Driver_station_input);
std::ostream& operator<<(std::ostream&,Driver_station_input);

#endif
