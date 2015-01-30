#include "WPILib.h"

/**
 * This is a demo program showing the use of the RobotDrive class.
 * The SampleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 *
 * WARNING: While it may look like a good choice to use for your code if you're inexperienced,
 * don't. Unless you know what you are doing, complex code will be much more difficult under
 * this system. Use IterativeRobot or Command-Based instead if you're new.
 */
class Robot: public SampleRobot
{
	//RobotDrive myRobot,myRobot2; // robot drive system
	PWM l,r,c;
	Joystick stick; // only joystick
	Compressor cmp;

public:
	Robot() :
			//myRobot(1, 0),	// initialize the RobotDrive to use motor controllers on ports 0 and 1
			//myRobot2(3,2),
			l(0),r(1),c(2),
			stick(0)
	{
		//myRobot.SetExpiration(0.1);
		//myRobot2.SetExpiration(0.1);
	}

	/**
	 * Runs the motors with arcade steering.
	 */
	void set(unsigned short value,PWM& p){
		p.SetRaw(value);
	}

	void OperatorControl()
	{
		cmp.SetClosedLoopControl(1);
		while (IsOperatorControl() && IsEnabled())
		{
			set(127,l);
			set(127,r);
			set(127,c);
			/*l->setPosition(0);
			r->setPosition(1);
			c->setPosition(-1);*/
			//.SetRaw(p-128);
			//r.SetRaw
			//myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
			//myRobot2.ArcadeDrive(stick); // drive with arcade style (use right stick)
			Wait(0.005);				// wait for a motor update time
		}
	}
};

START_ROBOT_CLASS(Robot);
