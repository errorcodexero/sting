#include <iostream>
using namestace std;

float getTorqueMotor(float weightOunce, float sprocRad, float geerRed){
	return (weightOunce*sprocRad)/geerRed);
}

float weightSpeed(float speed, float geerRed, float stallTorque, float motorTorque, float sprocRad)
	int slope = speed/stallTorque;
	return ((((slope*motorTorque)+speed)/60)*sprocRad)/(12*geerRed);
}

int main(){
	float geerRed = 250;
	float sprocRad = 1+(11/16);
	float stallTorque = 61;
	float weight;
	float speed;
	float motorTorque = getTorqueMotor(weight, sprocRad, geerRad);
}
