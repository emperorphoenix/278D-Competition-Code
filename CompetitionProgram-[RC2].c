#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    exp1,           sensorAnalog)
#pragma config(Sensor, dgtl1,  SonarIN,        sensorSONAR_raw)
#pragma config(Sensor, dgtl3,  RSE,            sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  LSE,            sensorQuadEncoder)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           BL,            tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           RS,            tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port3,           RS2,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           FR,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           Load,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           Gather,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           BR,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           LS2,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           LS,            tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port10,          FL,            tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!\


//Variables for RPMM task
float RPML; //RPM Left
float RPMR;	//RPM Right
float RPMTempL, RPMTempR;
float RPMAOTL, RPMAOTR;
bool hasBeenRead;


//Calculates RPM of left and right shooters based off motor encoders
task RPMM()
{
	float RPTL;
	float RPTR;
	int loopz=2;
	while(true){
		for(int j=0; j<loopz; j++){

			//Reset sensor vals and counter variables
			nMotorEncoder[LS]=0;
			nMotorEncoder[RS]=0;
			RPTL=0;
			RPTR=0;

			//Average counter variables over 40ms
			for(int i=0; i<10; i++){ //Note: Doesn't actually count real RPM, not that it matters
				RPTL+=abs(nMotorEncoder[LS]);
				RPTR+=abs(nMotorEncoder[RS]);
				wait1Msec(2);
			}

			//I have no idea why this is like this, but it's been working...
			RPMR=RPTR/10;
			RPML=RPTL/10;
			RPMTempL+=RPML;
			RPMTempR+=RPMR;
			if (j==0){
				RPMAOTL=RPMTempL/loopz;
				RPMAOTR=RPMTempR/loopz;
				RPMTempL=0;
				RPMTempR=0;
				hasBeenRead=false;
			}
			wait1Msec(100);
		}
	}//Loop
}//Don't expect accurate RPM, but do expect consistancy

float window;
float Left, Right;
float RPMT; 		//RPM target
task Sped()
{
	int StartSpeed=40; //40
	Left=StartSpeed;
	Right=StartSpeed;
	window=1;
	RPMT=0;
	motor[LS]=abs(Left);
	motor[LS2]=abs(Left);
	motor[RS]=abs(Right);
	motor[RS2]=abs(Right);
	while(true)
	{
		if(RPML>(RPMT+(window/2))){
			Left-=.02;
		}
		if(RPML<(RPMT-(window/2))){
			Left+=.02;
		}
		if(RPMR>(RPMT+(window/2))){
			Right-=.02;
		}
		if(RPMR<(RPMT-(window/2))){
			Right+=.02;
		}

		if(Left<0){Left=0;}
		if(Left>127){Left=127;}
		if(Right<0){Right=0;}
		if(Right>127){Right=127;}

		motor[LS]=abs(Left);
		motor[LS2]=abs(Left);
		motor[RS]=abs(Right);
		motor[RS2]=abs(Right);

		wait1Msec(10);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

void pre_auton()
{
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
  // Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Autonomous Task
//
// This task is used to control your robot during the autonomous phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

//Variables are declared globally for reading in the debugger
float sustain;		//Power to keep shooter idling at
float power;			//Power for firing
int delaaa;			//Startup delay
float BattV; //Constantly set to value of the battery voltage
bool isFiring;	//Debugging variable, shows if in the firing stage of code
int ballcount;
int loadSpeed;

task autonomous()
{
  //bool qweda=false;
	ballcount=0;
	//Set global Target, Sustain & Shooting power, and Startup Delay
	sustain=5.5; // was 40
	power=5.6; // was 5.6
	delaaa=250; //was 400
	loadSpeed=40;
	bLCDBacklight = true;

	if(nAvgBatteryLevel<7200){ //Adjust target & power based on voltage
		RPMT+=0;
		//power=70;
		if(nAvgBatteryLevel<7100){
			//power=80;
		}
	}

	isFiring=false;

	motor[Load]=0;

	clearTimer(T1);

	startTask(Sped);

	//Set shooter motors to Sustain speed

	RPMT=sustain;

	wait1Msec(1000); //Give time for spin up

	startTask(RPMM); //Start calculating RPM

	while(true){ //Infinite loop for shooting

		BattV=nImmediateBatteryLevel;

		//Display Shooter Speed value on LCD
		clearLCDLine(0);
		displayLCDPos(0,0);
		displayNextLCDString("LS: ");
		displayNextLCDNumber(RPML);
		displayLCDPos(0,8);
		displayNextLCDString("RS: ");
		displayNextLCDNumber(RPMR);

		//Display battery voltage on LCD
		clearLCDLine(1);
		displayLCDPos(1,0);
		displayNextLCDString("Battery V: ");
		displayNextLCDNumber(nAvgBatteryLevel);

		motor[Load]=loadSpeed; //Activate loader track

		if (SensorValue[SonarIN]<300){ //Run loader track until ball is detected
			ballcount++;
			//Stop loading track and wait for shooters to reach target speed after a power increase
			motor[Load]=0;
			RPMT=power;
			wait1Msec(200);
			//waitUntil(!((RPML<(RPMT-window/2))&&(RPMR<(RPMT-window/2))&&(RPML>(RPMT+window/2))&&(RPMR>(RPMT+window/2))));
			//waitUntil((((RPMAOT<(RPMT-window/2))&&(RPMAOT>(RPMT+window/2)))&&(hasBeenRead==false)));
			while(!((RPMAOTL>RPMT-window/2)&&(RPMAOTL<RPMT+window/2)&&(RPMAOTR>RPMT-window/2)&&(RPMAOTR<RPMT+window/2))){
				wait1Msec(1);
			}
			hasBeenRead=true;
			isFiring=true;

			//Reactivate loading track, wait till ball has fired
			motor[Load]=loadSpeed+10; //When running shooter off the same battery as the loader, the loader needs higher power for consistant speed
			wait1Msec(delaaa);//? Replace with read from sonar beneath the flywheels?
			/*
			while((SensorValue[Sonar2IN]>600)&&(SensorValue[Sonar2IN]>0)){ //Note: when sonar sensor has something too close to it, it reads 65536
			wait1Msec(1);
			}*/

			//Reset launcher back to sustain power
			RPMT=sustain;
			isFiring=false;

		}//Loop
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

//Variables are declared globally for reading in the debugger
//float RPMT; 		//RPM targer
//int sustain;		//Power to keep shooter idling at
//int power;			//Power for firing
//int delaaa;			//Startup delay
//float BattV; //Constantly set to value of the battery voltage
//bool isFiring;	//Debugging variable, shows if in the firing stage of code
int targRPML, targRPMR;
int xyz;
//int ballcount;
int FrRi, FrLe, BaRi, BaLe;
//int wsd = 365;

task usercontrol()
{
	bool Tri1, Tri2, Tri3, Tri4;
	Tri1=false;
	Tri2=false;
	Tri3=false;
	Tri4=false;
	ballcount=0;
	//Set global Target, Sustain & Shooting power, and Startup Delay
	RPMT=9; // was 8.5
	sustain=50; // was 40
	power=55; // was 60
	delaaa=250; //was 400
	bLCDBacklight = true;

	/*if(nAvgBatteryLevel<7200){ //Adjust target & power based on voltage
		RPMT=8.7;
		power=70;
		if(nAvgBatteryLevel<7100){
			power=80;
		}
	}*/

	isFiring=false;

	motor[Load]=0;

	clearTimer(T1);



	//Set shooter motors to Sustain speed
	motor[RS]=sustain;
	motor[RS2]=sustain;
	motor[LS]=sustain;
	motor[LS2]=sustain;
	targRPMR=sustain;
	targRPML=sustain;
	wait1Msec(1000); //Give time for spin up

	startTask(RPMM); //Start calculating RPM

	while(true){//Infinite loop for driving

		BattV=nImmediateBatteryLevel;

		//Display sonar value on LCD
		clearLCDLine(0);
		displayLCDPos(0,0);
		displayNextLCDString("LS: ");
		displayNextLCDNumber(targRPML);
		displayLCDPos(0,8);
		displayNextLCDString("RS: ");
		displayNextLCDNumber(targRPMR);

		//Display battery voltage on LCD
		clearLCDLine(1);
		displayLCDPos(1,0);
		displayNextLCDString("Battery V: ");
		displayNextLCDNumber(nAvgBatteryLevel);

		/*CONTROLS:

		---BUTTONS---

		5:
		Up:			Gather in
		Down:		Gather out
		6:
		Up:			Shooter speed up
		Down:		Shooter speed down
		7:
		Up:			Sync Shooters to Avg
		Down:		Sync Shooters to 0
		Left:		Only Left Adjust
		Right:	Only Right Adjust
		8:
		Up:			Loader Forward
		Down: 	Loader Backward
		Left:		Less granular controls
		Right:	More granular controls

		---STICKS---

		Left Stick:
		Up:			Drive Forward
		Down:		Drive Backward
		Left:		Strafe Left
		Right:	Strafe Right

		Right Stick:
		Up:
		Down:
		Left:		Turn Left
		Right:	Turn Right
		*/




		if(vexRT(Btn7L)==true){ //Left Only
			if(vexRT(Btn6U)==true&&Tri1==false){ //Shooter speed up
				if(vexRT(Btn8R)==true){
					if(targRPML<128){
						targRPML+=1;

					}else{
						targRPML=127;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPML<108){
						targRPML+=20;
						}else{
						targRPML=127;
					}
					}else{
					if(targRPML<110){
						targRPML+=10;
						}else{
						targRPML=127;
					}
				}
			}

			if(vexRT(Btn6D)==true){ //Shooter speed down
				if(vexRT(Btn8R)==true){
					if(targRPML>-1){
						targRPML-=1;
					}else{
						targRPML=0;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPML>19){
						targRPML-=20;
						}else{
						targRPML=0;
					}
					}else{
					if(targRPML>9){
						targRPML-=10;
						}else{
						targRPML=0;
					}
				}
			}
			}else	if(vexRT(Btn7R)==true){ //Right Only
			if(vexRT(Btn6U)==true){ //Shooter speed up
				if(vexRT(Btn8R)==true){
					if(targRPMR<128){
						targRPMR+=1;
					}else{
						targRPMR=127;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPMR<108){
						targRPMR+=20;
						}else{
						targRPMR=127;
					}
					}else{
					if(targRPMR<110){
						targRPMR+=10;
						}else{
						targRPMR=127;
					}
				}
			}

			if(vexRT(Btn6D)==true){ //Shooter speed down
				if(vexRT(Btn8R)==true){
					if(targRPMR>-1){
						targRPMR-=1;
					}else{
						targRPMR=0;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPMR>19){
						targRPMR-=20;
						}else{
						targRPMR=0;
					}
					}else{
					if(targRPML>9){
						targRPMR-=10;
						}else{
						targRPMR=0;
					}
				}
			}
			}else{
			if(vexRT(Btn6U)==true){ //Shooter speed up
				if(vexRT(Btn8R)==true){
					if(targRPML<128){
						targRPML+=1;
					}
					if(targRPMR<128){
						targRPMR+=1;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPML<108){
						targRPML+=20;
					}
					if(targRPMR<108){
						targRPMR+=20;
					}
					}else{
					if(targRPML<110){
						targRPML+=10;
					}
					if(targRPMR<110){
						targRPMR+=10;
					}
				}
			}

			if(vexRT(Btn6D)==true){ //Shooter speed down
				if(vexRT(Btn8R)==true){
					if(targRPML>-1){
						targRPML-=1;
					}
					if(targRPMR>-1){
						targRPMR-=1;
					}
					}else if(vexRT(Btn8L)==true){
					if(targRPML>19){
						targRPML-=20;
					}
					if(targRPMR>19){
						targRPMR-=20;
					}
					}else{
					if(targRPML>9){
						targRPML-=10;
					}
					if(targRPMR>9){
						targRPMR-=10;
					}
				}
			}
		}

		if(vexRT(Btn7U)==true){
			xyz=((targRPMR+targRPML)/2);
			targRPML=xyz;
			targRPMR=xyz;
		}

		if(vexRT(Btn7D)==true){
			targRPML=0;
			targRPMR=0;
		}

		//Enforce boundaries
		if(targRPML>127){targRPML=127;}
		if(targRPML<0){targRPML=0;}
		if(targRPMR>127){targRPMR=127;}
		if(targRPMR<0){targRPMR=0;}

		if(vexRT(Btn5U)==true){ //Gather In
			if(vexRT(Btn8R)==true){
				motor[Gather]=10;
				}else if(vexRT(Btn8L)==true){
				motor[Gather]=100;
				}else{
				motor[Gather]=60;
			}
			}else if(vexRT(Btn5D)==true){ //Gather Out
			if(vexRT(Btn8R)==true){
				motor[Gather]=-10;
				}else if(vexRT(Btn8L)==true){
				motor[Gather]=-100;
				}else{
				motor[Gather]=-60;
			}
			}else{
			motor[Gather]=0;
		}

		if(vexRT(Btn8U)==true){ //Loader forwards
			if(vexRT(Btn8R)==true){
				motor[Load]=60;
				}else if(vexRT(Btn8L)==true){
				motor[Load]=100;
				}else{
				motor[Load]=30;
			}
			}else if(vexRT(Btn8D)==true){ //Loader backwards
			if(vexRT(Btn8R)==true){
				motor[Load]=-60;
				}else if(vexRT(Btn8L)==true){
				motor[Load]=-100;
				}else{
				motor[Load]=-30;
			}
			}else{
			motor[Load]=0;
		}


		FrRi = vexRT[Ch3] - vexRT[Ch1] - vexRT[Ch4];
		FrLe = vexRT[Ch3] + vexRT[Ch1] + vexRT[Ch4];
		BaRi =  vexRT[Ch3] - vexRT[Ch1] + vexRT[Ch4];
		BaLe =  vexRT[Ch3] + vexRT[Ch1] - vexRT[Ch4];

		if(abs(FrRi)>10){
			motor[FR]=FrRi;
			}else{
			motor[FR]=0;
		}
		if(abs(FrLe)>10){
			motor[FL]=FrLe;
			}else{
			motor[FL]=0;
		}
		if(abs(BaRi)>10){
			motor[BR]=BaRi;
			}else{
			motor[BR]=0;
		}
		if(abs(BaLe)>10){
			motor[BL]=BaLe;
			}else{
			motor[BL]=0;
		}


		motor[LS]=abs(targRPML);
		motor[LS2]=abs(targRPML);
		motor[RS]=abs(targRPMR);
		motor[RS2]=abs(targRPMR);

		if(vexRT(Btn8UXmtr2)==true){
		}

		wait1Msec(10);
	}//Loop
}
