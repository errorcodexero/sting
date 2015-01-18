#include "lcd_scroller.h"
#include<cassert>
#include<iostream>
#include "../util/util.h"

using namespace std;

Lcd_scroller::Lcd_scroller():top_line(0){}

void Lcd_scroller::update(string const& s1,bool up_button,bool down_button){
	if(up(up_button)){
		if(top_line>0) top_line--;
	}
	if(down(down_button)){
		top_line++;
	}
	vector<string> lines=to_lines(s1,Driver_station_output::Lcd::WIDTH-3);
	unsigned i;
	for(i=0;i+top_line<lines.size() && i<Driver_station_output::Lcd::HEIGHT;i++){
		out.line[i]=as_string(i+top_line+1)+":"+lines[i+top_line];
	}
	for(;i<Driver_station_output::Lcd::HEIGHT;i++){
		out.line[i]="~";
	}
}

Driver_station_output::Lcd Lcd_scroller::get()const{ return out; }

ostream& operator<<(ostream& o,Lcd_scroller a){
	o<<"Lcd_scroller(";
	o<<"top_line:"<<a.top_line;
	o<<" up:"<<a.up;
	o<<" down:"<<a.down;
	o<<" "<<a.out;
	return o<<")";
}

#ifdef LCD_SCROLLER_TEST
int main(){
	Lcd_scroller a;
	cout<<a<<"\n";
	string s="hi\nhello\nwhat\there\nthere\n55\n77\n99";
	a.update(s,0,0);
	cout<<a<<"\n";
	for(unsigned i=0;i<3;i++){
		a.update(s,0,0);
		cout<<a<<"\n";
		a.update(s,0,1);
		cout<<a<<"\n";
	}
}
#endif
