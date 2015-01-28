#include <iostream>
using namestace std;

float getTorqueMotor(float weightOunce, float sprocRad, float gearRed){
	return (weightOunce*sprocRad)/gearRed);
}

float weightSpeed(float speed, float gearRed, float stallTorque, float motorTorque, float sprocRad)
	int slope = speed/stallTorque;
	return ((((slope*motorTorque)+speed)/60)*sprocRad)/(12*gearRed);
}

int main(){
	float gearRed = 250;
	float sprocRad = 1+(11/16);
	float stallTorque = 61;
	float weight;
	float speed;
	float motorTorque = getTorqueMotor(weight, sprocRad, gearRad);
}
