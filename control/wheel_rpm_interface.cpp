#ifdef WHEEL_RPMS

#include <vector>
#include <assert.h>
#include <stdlib.h>

#if 0 // usleep() is obsolete
#include <unistd.h>
#else // use nanosleep() instead
#include <time.h>
#endif

#include <iostream>
#include "../util/util.h"
#include "wheelrpms.h"
#include "../input/joystick_linux.h"
#include "../util/negedge_trigger.h"

using namespace std;

int usleep (const int us_delay){
    struct timespec tim, tim2;

    tim.tv_sec  = 0;
    tim.tv_nsec = us_delay / 1000;

    int retSts = nanosleep (&tim , &tim2);

    return retSts;
}

enum Button{
	START,BTN0,BTN1
};

void print_compact(ostream& o,Joystick_data_linux const& a){
	o<<"Data In("<<a.button<<")"<<endl<<endl;
	usleep(100000);
}

ostream& operator<<(ostream& o,Button a){
	if(a==START)o<<"START=ON"<<endl;
	if(a==BTN0)o<<"BTN0=ON"<<endl;
	if(a==BTN1)o<<"BTN1-ON"<<endl;
	return o;
}

struct Presscount{
	Negedge_trigger n;
	int btn1;
	int btn2;
	int btn3;
	Presscount():btn1(0),btn2(0),btn3(0){}
	void updatea(bool a){
		if(n(a))btn1++;
	}
	void updateb(bool b){
		if(n(b))btn2++;
	}
	void updatec(bool c){
		if(n(c))btn3++;
	}
};

void printpresses(){
	Joystick_linux input("/dev/input/js0");
	Joystick_data_linux output=input.read();
	bool b=output.button.find(0)->second;
	Negedge_trigger pressed;
	pressed(b);
}

void getinput(){
	Joystick_data_linux output;
	Joystick_linux input("/dev/input/js0");
	cout<<"Enter Stuff"<<endl;
	while(0<1){
	output=input.read();
	print_compact(cout,output);
	//stored=input.read();
	}
}

void printbutton1(){
	while(1<2){
		Joystick_linux input("/dev/input/js0");
		Joystick_data_linux output=input.read();
		cout<<*output.button.find(0)<<endl;
		usleep(100000);
	}
}

int changevalueconverter(vector<int> v){
	assert(v.size()==6);
	int p=0;
	if(v[0]==1)p=p+3200;
	if(v[1]==1)p=p+1600;
	if(v[2]==1)p=p+800;
	if(v[3]==1)p=p+400;
	if(v[4]==1)p=p+200;
	if(v[5]==1)p=p+100;
	return p;
}

vector<int> vthree(vector<int> v){
	assert(!v.size()<3);
	vector<int> newv;
	for(int i=0;i<3;i++){
		newv.push_back(v[i]);
	}
	return newv;
}

vector<int> vsix(vector<int> v){
	assert(!v.size()<9);
	vector<int> newv;
	for(int i=3;i<9;i++){
		newv.push_back(v[i]);
	}
	return newv;
}

wheelcalib whattochange (vector<int> v){
	assert(v.size()==3);
	wheelcalib r;
	vector<int> t=vthree(v);
	int s=changevalueconverter(vsix(v));
	if(t[0]==0&&t[1]==0&&t[2]==0)r.highgoal.top=s;
	if(t[0]==0&&t[1]==0&&t[2]==1)r.highgoal.bottom=s;
	if(t[0]==0&&t[1]==1&&t[2]==0)r.lowgoal.top=s;
	if(t[0]==0&&t[1]==1&&t[2]==1)r.lowgoal.bottom=s;
	if(t[0]==1&&t[1]==0&&t[2]==0)r.overtruss.top=s;
	if(t[0]==1&&t[1]==0&&t[2]==1)r.overtruss.bottom=s;
	if(t[0]==1&&t[1]==1&&t[2]==0)r.passing.top=s;
	if(t[0]==1&&t[1]==1&&t[2]==1)r.passing.bottom=s;
	writeconfig(r);
	return r;
}

#ifdef WHEEL_RPM_INTERFACE_TEST

int main(){
	try{
	vector<int> buttonorder;
	vector<int> motorandprep;
	vector<int> binarytoconvert;
	wheelcalib vpostchange;
	Presscount a;
	Presscount b;
	Presscount c;
	while(buttonorder.size()!=9){		
		Joystick_linux input("/dev/input/js0");
		Joystick_data_linux output=input.read();
		a.updatea(output.button.find(0)->second);
		if(a.btn1%2!=0){
			int btn0=b.btn2;
			int btn1=c.btn3;
			b.updateb(output.button.find(1)->second);
			c.updatec(output.button.find(2)->second);
			cout<<a.btn1<<"	";
			cout<<"btn2="<<b.btn2<<"	";
			cout<<"btn3="<<c.btn3<<endl;
			if(btn0<b.btn2)buttonorder.push_back(0);
			if(btn1<c.btn3)buttonorder.push_back(1);
			cout<<buttonorder.size()<<"	"<<buttonorder<<endl;
		}
	}
	motorandprep=vthree(buttonorder);
	binarytoconvert=vsix(buttonorder);
	cout<<motorandprep<<"	"<<changevalueconverter(binarytoconvert)<<"	"<<buttonorder<<endl;
	vpostchange=whattochange(buttonorder);
	cout<<vpostchange<<endl;
	}catch(string&s){
		cout<<"The dark fire will not avail you! Flame of Udûn!"<<endl;
	}
}

#endif // WHEEL_RPM_INTERFACE_TEST

#endif // _WRS_KERNEL
