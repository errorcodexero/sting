/*#include <iostream>

using namespace std;

int main(){
	//independent variables
	float gearRed = 250;
	float sprocRad = 1+((float)11/16);//inches
	float stallTorque = 61;//oz in
	float maxSpeed = 7200;//rotations per minute
	float weight = 1346.1;//oz
	//dependent variables;
	float speTorSlope = -1*maxSpeed/stallTorque;
	float motTorque = weight*sprocRad/gearRed;
	float rotWitWeight = speTorSlope*motTorque+maxSpeed;
	float speWitWeight = rotWitWeight/(60*gearRed);//may be wrong but i think i got it right


	float maxWeight = gearRed*stallTorque/sprocRad/16;//Max Weight in pounds
	cout<<"Slope of a speed torque graph: "<<speTorSlope<<"\n";
	cout<<"Oz*in on motor sized: "<<motTorque<<"\n";
	cout<<"Rotational per second with weight: "<<rotWitWeight<<"\n";
	cout<<"Speed of chain moving inch per second: "<<speWitWeight<<"\n";
}*/
