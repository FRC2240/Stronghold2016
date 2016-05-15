	#include "WPILib.h"

class Robot: public IterativeRobot
{
private:
	/*--------------------------VARIABLES--------------------------*/
	float gatherSpeed 		= 0.0;
	float move 				= 0.0;
	float rotate			= 0.0;
	/*--------------------------BOOLEANS--------------------------*/
	bool shooterPosition 	= true; //false = out, true = in
	bool shooterWheels 		= false;
	bool wheelsGathererIn 	= false;
	bool wheelsGathererOut 	= false;
	bool done 				= false;
	bool shootingHigh 		= false;
	bool shootingLow 		= false;
	bool clamped 			= false;
	bool clamping 			= false;
	/*--------------------------TIMERS----------------------------*/
	int gatherButtonTimer 	= 0;
	int shooterButtonTimer 	= 0;
	int clampButtonTimer 	= 0;
	int shooterTimer 		= 0;
	int clampTimer 			= 0;
	bool autonomousMode 	= false;
	bool autoClampHigh		= false;
	bool autoClampLow 		= false;
	bool autoClampHighDone 	= false;
	bool autoClampLowDone	= false;
	/*--------------------------OBJECTS---------------------------*/
	TalonSRX 	 *frontLeft;
	TalonSRX 	 *frontRight;
	TalonSRX 	 *backLeft;
	TalonSRX 	 *backRight;
	TalonSRX 	 *lShooter;
	TalonSRX 	 *rShooter;
	TalonSRX 	 *gatherer;
	TalonSRX 	 *shooterInOut;
	TalonSRX 	 *gathererWheels;
	Encoder 	 *lEnc;
	Encoder 	 *rEnc;
	DigitalInput *autoSwitch;
	RobotDrive 	 *drive;
	Joystick 	 *stick;

	void RobotInit()
	{
		stick 		 	= new Joystick(0);
		frontLeft 	 	= new TalonSRX(1);
		backLeft 	 	= new TalonSRX(0);
		frontRight 	 	= new TalonSRX(8);
		backRight 	 	= new TalonSRX(9);
		shooterInOut 	= new TalonSRX(6);
		lShooter 		= new TalonSRX(2); // Left shooter wheels
		rShooter 	   	= new TalonSRX(3); // Right shooter wheels
		gatherer 	   	= new TalonSRX(5); // Gatherer up/down
		gathererWheels 	= new TalonSRX(4); //Gatherer in and out wheels
		//lEnc = new Encoder(0, 1, false, Encoder::EncodingType::k4X);
		//rEnc = new Encoder(2, 3, false, Encoder::EncodingType::k4X);
		//lEnc->SetMaxPeriod(.1);
		//lEnc->SetMinRate(10);
		//lEnc->SetDistancePerPulse(5);
		//lEnc->SetReverseDirection(true);
		//lEnc->SetSamplesToAverage(7);
		//rEnc->SetMaxPeriod(.1);
		//rEnc->SetMinRate(10);
		//rEnc->SetDistancePerPulse(5);
		//rEnc->SetReverseDirection(true);
		//rEnc->SetSamplesToAverage(7);
		autoSwitch 		= new DigitalInput(0);
		try
		{
			CameraServer::GetInstance()->SetQuality(50);
			CameraServer::GetInstance()->StartAutomaticCapture("cam0");
		}catch(...){

		}
	}
	void AutonomousInit()
	{
		//autonomousMode = autoSwitch->Get();
		done = false;
	}
	void AutonomousPeriodic()
	{
		/*if(!done && !autonomousMode)
		{
			Auto("Move", -.2, 100, 1.0, .975);
			Auto("Move", -0.6, 600, 1.0, .975);
			Auto("Move", .2, 100, .85, 1.0);
			Auto("Move", 0.6, 600, .85, 1.0);
			Auto("Move", -.2, 100, 1.0, .975);
			Auto("Move", -0.6, 600, 1.0, .975);
			done = true;
		}*/
		if(!done/* && autonomousMode*/)
		{
			Auto("Move", -.2, 100, 1.0, 1.0);
			Auto("Move", -.6, 725, 1.0, 1.0);
			done = true;
		}
		else
		{
			frontLeft->Set(0.0);
			backLeft->Set(0.0);
			frontRight->Set(0.0);
			backRight->Set(0.0);
		}
	}
	void TeleopInit()
	{
		drive = new RobotDrive(backLeft, frontLeft, backRight, frontRight);
	}
	void TeleopPeriodic()
	{
		/*------------------------DRIVING------------------------*/
		move = stick->GetRawAxis(1) * -1.0;
		rotate = stick->GetRawAxis(4) * -1.0;
		drive->ArcadeDrive(move, rotate, false);
		/*------------------------CLAMPING-----------------------*/
		if(!clamping && clampButtonTimer >= 25 && stick->GetRawButton(5))
		{
			clamping = true;
			clampButtonTimer = 0;
		}
		if(clamping)
		{
			clampTimer++;
			if(clampTimer < 10 && !clamped)
			{
				shooterInOut->Set(-0.75);
			}
			else if(clampTimer < 10 && clamped)
			{
				shooterInOut->Set(0.75);
			}
			else
			{
				clampTimer = 0;
				clamped = !clamped;
				clamping = false;
				if(autoClampHigh)
				{
					autoClampHigh = false;
					autoClampHighDone = true;
				}
				else if(autoClampLow)
				{
					autoClampLow = false;
					autoClampLowDone = true;
				}
			}
		}
		/*------------------------SHOOTING------------------------*/
		if(((!shootingHigh || !shootingLow) && shooterButtonTimer >= 50 && stick->GetRawButton(4)) || autoClampHighDone)
		{
			shooterButtonTimer = 0;
			if(clamped)
			{
				clamping = true;
				autoClampHigh = true;
			}
			else
			{
				shootingHigh = true;
				autoClampHighDone = false;
			}
		}
		else if(((!shootingLow || !shootingHigh) && shooterButtonTimer >= 50 && stick->GetRawButton(1)) || autoClampLowDone)
		{
			shooterButtonTimer = 0;
			if(clamped)
			{
				clamping = true;
				autoClampLow = true;
			}
			else
			{
				shootingLow = true;
				autoClampLowDone = false;
			}
		}
		if(shootingHigh)
		{
			shooterTimer++;
			lShooter->Set(-1.0);
			rShooter->Set(1.0);
			//shooterInOut->Set(.25);
			//if(shooterTimer >= 25 && shooterTimer <= 30)
			//{
			//	shooterInOut->Set(0.75);
			//}
			if(shooterTimer > 30 && shooterTimer <= 40)
			{
				shooterInOut->Set(-1.0);
			}
			if(shooterTimer > 45 && shooterTimer <= 50)
			{
				shooterInOut->Set(0.0);
			}
			if(shooterTimer > 50 && shooterTimer <= 55)
			{
				shooterInOut->Set(0.6);
			}
			if(shooterTimer > 55)
			{
				shootingHigh = false;
				shooterTimer = 0;
			}
		}
		else if(shootingLow)
		{
			shooterTimer++;
			lShooter->Set(-0.44);
			rShooter->Set(0.44);
			//shooterInOut->Set(.25);
			//if(shooterTimer >= 15 && shooterTimer <= 20)
			//{
			//	shooterInOut->Set(0.75); //this is out
			//}
			if(shooterTimer > 20 && shooterTimer <= 30)
			{
				shooterInOut->Set(-1.0);
			}
			if(shooterTimer > 35 && shooterTimer <= 40)
			{
				shooterInOut->Set(0.0);
			}
			if(shooterTimer > 45 && shooterTimer <= 50)
			{
				shooterInOut->Set(0.6);
			}
			if(shooterTimer > 50)
			{
				shootingLow = false;
				shooterTimer = 0;
			}
		}
		else
		{
			lShooter->Set(0.0);
			rShooter->Set(0.0);
			if(!clamping)
			{
				shooterInOut->Set(0.0);
			}
		}
		/*---------------------GATHER WHEELS---------------------*/
		if(!wheelsGathererIn && gatherButtonTimer >= 15 && stick->GetRawButton(2))
		{
			gatherButtonTimer = 0;
			wheelsGathererOut = !wheelsGathererOut;
		}
		if(!wheelsGathererOut && gatherButtonTimer >= 15 && stick->GetRawButton(3))
		{
			gatherButtonTimer = 0;
			wheelsGathererIn = !wheelsGathererIn;
		}
		if(wheelsGathererIn)
		{
			gathererWheels->Set(1.0);
		}
		else if(wheelsGathererOut)
		{
			gathererWheels->Set(-1.0);
		}
		else
		{
			gathererWheels->Set(0.0);
		}
		/*------------------------GATHERER-----------------------*/
		if(stick->GetRawAxis(2) != 0)
		{
			gatherSpeed = stick->GetRawAxis(2);
		}
		else if(stick->GetRawAxis(3) != 0)
		{
			gatherSpeed = stick->GetRawAxis(3) * -1;
		}
		else
		{
			gatherSpeed = 0.0;
		}
		gatherer->Set(gatherSpeed);
		/*------------------------TIMERS-------------------------*/
		gatherButtonTimer++;
		shooterButtonTimer++;
		clampButtonTimer++;
	}
	void Auto(std::string command, float power, float time, float correctL, float correctR)
	{
		if(command == "Move")
		{
			float correct = 1.0;
			while(time >= 0)
			{
				if(power < 0)
				{
					correctR = 1.0;
				}
				//drive->ArcadeDrive(power, 0, false);
				frontLeft->Set(-power * correctL);
				backLeft->Set(-power * correctL);
				frontRight->Set(power * correctR);
				backRight->Set(power * correctR);
				Wait(0.005);
				time--;
			}
		}
		else if(command == "Turn")
		{
			while(time >= 0)
			{
				//drive->ArcadeDrive(0, power, false);
				Wait(0.005);
				time--;
			}
		}
	}
};

START_ROBOT_CLASS(Robot);
