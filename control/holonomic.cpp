#include "holonomic.h"
#include<math.h>
#include<algorithm>
#include<iostream>

using namespace std;

double max3(double a,double b,double c){
	return max(max(a,b),c);
}

Drive_goal::Drive_goal(){
	field_relative = 0;
}

Drive_goal::Drive_goal(Pt p,bool b):direction(p),field_relative(b){}

std::ostream& operator<<(std::ostream& o, Drive_goal g){
	return o<<"dg("<<g.direction<<","<<g.field_relative<<")";
}

Drive_motors func_inner(double x, double y, double theta){	
	Drive_motors r;
	r.a=-double(1)/3* theta- double(1)/3* x -(double(1)/sqrt(3))*y;
	r.b=-double(1)/3* theta- double(1)/3* x +(double(1)/sqrt(3))*y;
	r.c=(-(double(1)/3)* theta) + ((double(2)/3)* x);
	return r;
}
Pt rotate_vector(double x, double y, double theta, double angle){
	double cosA = cos(angle * (3.14159 / 180.0));
	double sinA = sin(angle * (3.14159 / 180.0));
	double xOut = x * cosA - y * sinA;
	double yOut = x * sinA + y * cosA;
	
	return Pt(xOut,yOut,theta);
}
Drive_motors maximizeSpeed(Drive_motors r){
	const double s=sqrt(3);
	r.a*=s;
	r.b*=s;
	r.c*=s;
	const double m=max3(fabs(r.a),fabs(r.b),fabs(r.c));
	if(m>1){
		r.a/=m;
		r.b/=m;
		r.c/=m;
	}
	return r;
}
Drive_motors holonomic_mix(double x,double y,double theta,double orientation,bool fieldRelative){
	//This function exists in order to pull the full power out of the drivetrain.  
	//It makes some of the areas of the x/y/theta space have funny edges/non-smooth areas, but I think this is an acceptable tradeoff.
	//Also rotates the vectors of the drive if the robot is in field relative mode
	Pt p;
	if (fieldRelative){
		p=rotate_vector(x,y,theta,orientation);
	} 
	else {
		p=Pt(x,y,theta);
	}
	static int i = 0;
	if(i==0){
		cerr<<"Joy.x= "<<x<<" "<<"Joy.y= "<<y<<" "<<"Joystick.theta= "<<theta<<"\n";
		cerr<<"P.x= "<<p.x<<" "<<"P.y= "<<p.y<<" "<<"P.theta= "<<p.theta<<"\n";	
	}
	i=(i+1)%500;
	
	return maximizeSpeed(func_inner(p.x,p.y,p.theta));
}

Drive_motors holonomic_mix(Pt p){
	return holonomic_mix(p.x,p.y,p.theta,0,false);
}

ostream& operator<<(ostream& o, Drive_motors d){
	return o<<"dm("<<d.a<<","<<d.b<<","<<d.c<<")";
}

Drive_motors control(Drive_goal dg, float orientation){
/*	
	-orientation takes care of 1 variable: float
	-Drive_motors has 4 variables: x, y, theta, field_relative
	-Drive_motors holonomic_mix needs in order: x, y, theta, orientation, field_relative
*/
	Pt a = dg.direction;
	return holonomic_mix(a.x, a.y, a.theta, orientation, dg.field_relative);
}

#ifdef HOLONOMIC_TEST
int main(){
	Drive_goal dg;
	cout<<dg;
}
#endif
