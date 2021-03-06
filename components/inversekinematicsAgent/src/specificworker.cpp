/*
 *    Copyright (C) 2006-2010 by RoboLab - University of Extremadura
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "specificworker.h"
#include <agm.h>
#include <boost/concept_check.hpp>
#include <qt4/QtCore/qdatetime.h>


/**
* \brief Default constructor
*/

SpecificWorker::SpecificWorker(MapPrx& mprx) : GenericWorker(mprx)
{
	setlocale (LC_ALL, "C");

	worldModel = AGMModel::SPtr(new AGMModel());
	worldModel->name = "worldModel";
	active = false;

	innerModel = new InnerModel("/home/robocomp/robocomp/components/robocomp-ursus-touchgame/etc/ursusMM.xml");
}

void SpecificWorker::ballFound()
{
	static float a = 0;

	float tx =    0. + 500.*cos(a);
	float ty = 1000. + 200.*sin(a);
	float tz =  700.;
	QVec poseTr = innerModel->transform("world", QVec::vec3(tx,ty,tz), "robot");
	printf("gooooooo T saccadic3D =(%.2f, %.2f, %.2f)\n", poseTr(0), poseTr(1), poseTr(2));

	//sendRightHandPose(QVec::vec3(400, 400, 400), QVec::vec3(0,0,0), QVec::vec3(1,1,1), QVec::vec3(0,0,0));		
	try
	{
		bodyinversekinematics_proxy->goHome("RIGHTARM");
	}
	catch (Ice::Exception e)
	{
		qDebug()<<"error talking to bik"<<e.what();
	}
	saccadic3D(poseTr,QVec::vec3(0,    -1,   0));
	
	a += 0.65;
}

void SpecificWorker::ballCentered()
{
	int32_t ball = atoi(params["b"].value.c_str());
	int32_t robot = atoi(params["r"].value.c_str());

	try
	{
		const float tx = str2float(worldModel->getSymbol(ball)->getAttribute("tx"));
		const float ty = str2float(worldModel->getSymbol(ball)->getAttribute("ty"));
		const float tz = str2float(worldModel->getSymbol(ball)->getAttribute("tz"));
		QVec poseTr = innerModel->transform("world", QVec::vec3(tx, ty, tz), "robot");
		printf("gooooooo saccadic3D T=(%.2f, %.2f, %.2f)\n", poseTr(0), poseTr(1), poseTr(2));				
		saccadic3D(poseTr, QVec::vec3(0,-1,0));
		
		/// Include "fixates" edge
		AGMModel::SPtr newModel(new AGMModel(worldModel));
		newModel->addEdgeByIdentifiers(robot, ball, "fixates");
		sendModificationProposal(worldModel, newModel);
	}
	catch(AGMModelException &e)
	{
		try
		{
			speech_proxy->say("acho, pasó algo raro con centered", true);
		}
		catch(AGMModelException &e)
		{
			printf("acho, pasó algo raro con speech en algo raro con centered\n");
		}
		printf("I don't know object %d\n", ball);
	}
}


void SpecificWorker::ballTouched(bool first)
{
	QTime fff;
	if (first) fff = QTime::currentTime();

// 	if (tocaButton->isChecked())
	{
		tocaButton->setText("tocando");
		const int32_t ball = atoi(params["b"].value.c_str());
		const int32_t robot = atoi(params["r"].value.c_str());

		printf("\n\n----------\n");
		try
		{
			// Get 
			const float tx = str2float(worldModel->getSymbol(ball)->getAttribute("tx"));
			const float ty = str2float(worldModel->getSymbol(ball)->getAttribute("ty"));
			const float tz = str2float(worldModel->getSymbol(ball)->getAttribute("tz"));
			const QVec offset = QVec::vec3(0., 0., -100.);
			const QVec targetRobot = QVec::vec3(tx, ty, tz) + offset;
			
			const QVec actualWristPosition = QVec::vec3(
			str2float(worldModel->getSymbol(robot)->attributes["rightwrist_tx"]),
			str2float(worldModel->getSymbol(robot)->attributes["rightwrist_ty"]),
			str2float(worldModel->getSymbol(robot)->attributes["rightwrist_tz"]));
			
			actualWristPosition.print("actualWristPosition");
			

			const QVec error = targetRobot-actualWristPosition;

			// If the hand is far from the target, move the hand
			int32_t extra = 0.015 * (fff.elapsed() - 2000);
			if (extra<0) extra = 0;
			if (extra>200) extra = 200;
			int32_t umbral = 100. + extra;
			if (error.norm2() > umbral)
			{
				targetRobot.print("targetRobot");
				actualWristPosition.print("actualWristPosition");
				error.print("error");
				printf("ERROR: %f\n", error.norm2());
				printf("UMBRAL: %d\n", umbral);
				const QVec poseTr = innerModel->transform("world", targetRobot, "robot");
				printf("move hand to T=(%.2f, %.2f, %.2f)  \n", poseTr(0), poseTr(1), poseTr(2));
				sendRightHandPose(poseTr, QVec::vec3(0,0,0), QVec::vec3(1,1,1), QVec::vec3(0,0,0));		
			}
			// If the hand is close to the target, acknowledge the new state
			else
			{
				printf("lo conseguimos!!!\n");
				printf("lo conseguimos!!!\n");
				printf("lo conseguimos!!!\n");
				printf("lo conseguimos!!!\n");
				printf("lo conseguimos!!!\n");
				printf("lo conseguimos!!!\n");
				resetGame();
			}
		}
		catch(AGMModelException &e)
		{
			printf("I don't know ball %d\n", ball);
		}
	}
}

void SpecificWorker::resetGame()
{
	try{
		speech_proxy->say("He llegado", true);		
	}
	catch (Ice::Exception e)
	{
		qDebug()<<"error talking to face tablet";
	}
	
	
	
	usleep(1000000);
	bodyinversekinematics_proxy->goHome("RIGHTARM");
	usleep(3000000);
	AGMModel::SPtr newModel(new AGMModel());
	AGMModelConverter::fromXMLToInternal("/home/robocomp/robocomp/components/robocomp-ursus-touchgame/etc/initialModel.xml", newModel);

	tocaButton->setChecked(false);
	tocaButton->setText("toca?");
	
	sendModificationProposal(worldModel, newModel);
}

///slot
void SpecificWorker::compute( )
{
// printf("%s: %d\n", __FILE__, __LINE__);
	static std::string lastAction = "---";
	bool change = (lastAction != action);
	lastAction = action;

	printf("action: %s\n", action.c_str());
	if (action == "ballfound")
	{
		if (change)
		{
			try
			{
				speech_proxy->say("busco", true);
			}
			catch (Ice::Exception e)
			{
				qDebug()<<"error talking to face tablet";
			}
			
			RoboCompFaceTabletUrsus::valuesList vList;
			vList.clear();
			RoboCompFaceTabletUrsus::values v;
			v.idES = 5;//neutral
			vList.push_back(v);
			try
			{
				facetabletursus_proxy->newFaceTabletUrsus(vList);
			}
			catch (Ice::Exception e)
			{
				qDebug()<<"error talking to face tablet";
			}
			
		}
		ballFound();
		usleep(1000000);
	}
	else if (action == "ballcentered")
	{
		ballCentered();
		usleep(500000);
	}
	else if (action == "balltouched" )
	{
		RoboCompFaceTabletUrsus::valuesList vList;
		vList.clear();
		RoboCompFaceTabletUrsus::values v;
		v.idES = 4;//happy
		vList.push_back(v);
		
		try 
		{
			facetabletursus_proxy->newFaceTabletUrsus(vList);
		}
		catch (Ice::Exception e)
		{
			qDebug()<<"error talking to face tablet";
		}
		
		ballTouched(lastAction!=action);
		usleep(500000);
	}
	else if (action == "none" )
	{
		resetGame();
		usleep(3000000);

	}
	else
	{
		printf("ignoring this action (%s)...\n", action.c_str());
	}
}

void SpecificWorker::slotStop()
{
	currenState=STOP;
}

bool SpecificWorker::setParams(RoboCompCommonBehavior::ParameterList params)
{
	timer.start(Period);
	return true;
}


bool SpecificWorker::activateAgent(const ParameterMap& prs)
{
	bool activated = false;
	if (setParametersAndPossibleActivation(prs, activated))
	{
		if (not activated)
		{
			mutex->lock();
			active = true;
			mutex->unlock();
			return true;
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool SpecificWorker::deactivateAgent()
{
	mutex->lock();
	active = false;
	mutex->unlock();
	return true;
}

StateStruct SpecificWorker::getAgentState()
{
	StateStruct s;
	if (active)
	{
		s.state = Running;
	}
	else
	{
		s.state = Stopped;
	}
	s.info = action;
	return s;
}


ParameterMap SpecificWorker::getAgentParameters()
{
	return params;
}

bool SpecificWorker::setAgentParameters(const ParameterMap& prs)
{
	bool activated = false;
	return setParametersAndPossibleActivation(prs, activated);
}

void SpecificWorker::killAgent()
{
	exit(-1);
}

int SpecificWorker::uptimeAgent()
{
	return 0;
}

bool SpecificWorker::reloadConfigAgent()
{
	return true;
}

void SpecificWorker::modelModified(const RoboCompAGMWorldModel::Event& modification)
{
	mutex->lock();
	AGMModelConverter::fromIceToInternal(modification.newModel, worldModel);
	mutex->unlock();
}

void SpecificWorker::modelUpdated(const RoboCompAGMWorldModel::Node& modification)
{
	mutex->lock();
	AGMModelConverter::includeIceModificationInInternalModel(modification, worldModel);
	mutex->unlock();
}


bool SpecificWorker::setParametersAndPossibleActivation(const ParameterMap &prs, bool &reactivated)
{
	// We didn't reactivate the component
	reactivated = false;

	// Update parameters
	params.clear();
	for (ParameterMap::const_iterator it=prs.begin(); it!=prs.end(); it++)
	{
		params[it->first] = it->second;
	}

	try
	{
		action = params["action"].value;
		std::transform(action.begin(), action.end(), action.begin(), ::tolower);


		if (action == "findobjectvisually")
		{
			active = true;
		}
		else
		{
			active = true;
		}
	}
	catch (...)
	{
		return false;
	}

	// Check if we should reactivate the component
	if (active)
	{
		active = true;
		reactivated = true;
	}

	return true;
}

/**
* \brief Default destructor
*/
SpecificWorker::~SpecificWorker()
{

}


void SpecificWorker::sendRightHandPose(QVec t, QVec r, QVec wt, QVec wr)
{
	sendRightHandPose(t, r, wt(0), wt(1), wt(2), wr(0), wr(1), wr(2));
}

void SpecificWorker::sendRightHandPose(QVec t, QVec r, float wtx, float wty, float wtz, float wrx, float wry, float wrz)
{
	sendRightHandPose(t(0), t(1), t(2), r(0), r(1), r(2), wtx, wty, wtz, wrx, wry, wrz);
}

void SpecificWorker::sendRightHandPose(float tx, float ty, float tz, float rx, float ry, float rz, float wtx, float wty, float wtz, float wrx, float wry, float wrz)
{
	RoboCompBodyInverseKinematics::Pose6D target;
	target.x = tx;
	target.y = ty;
	target.z = tz;
	target.rx = rx;
	target.ry = ry;
	target.rz = rz;
	RoboCompBodyInverseKinematics::WeightVector weights;
	weights.x = wtx;	
	weights.y = wty;
	weights.z = wtz;
	weights.rx = wrx;
	weights.ry = wry;
	weights.rz = wrz;
	try
	{
		bodyinversekinematics_proxy->stop("RIGHTARM");
		usleep(500000);
		bodyinversekinematics_proxy->setTargetPose6D("RIGHTARM", target, weights, 100);
		usleep(1500000);
	}
	catch(...)
	{
		printf("IK connection error\n");
	}
}


void SpecificWorker::saccadic3D(QVec point, QVec axis)
{
	saccadic3D(point(0), point(1), point(2)+200, axis(0), axis(1), axis(2));
}

void SpecificWorker::saccadic3D(float tx, float ty, float tz, float axx, float axy, float axz)
{
	RoboCompBodyInverseKinematics::Pose6D targetSight;
	targetSight.x = tx;
	targetSight.y = ty;
	targetSight.z = tz;
	RoboCompBodyInverseKinematics::Axis axSight;
	axSight.x = axx;
	axSight.y = axy;
	axSight.z = axz;
	bool axisConstraint = false;
	float axisAngleConstraint = 0;
	try
	{
		bodyinversekinematics_proxy->stop("HEAD");
		usleep(500000);
		bodyinversekinematics_proxy->pointAxisTowardsTarget("HEAD", targetSight, axSight, axisConstraint, axisAngleConstraint);
	}
	catch(...)
	{
		printf("IK connection error\n");
	}
}

void SpecificWorker::sendModificationProposal(AGMModel::SPtr &worldModel, AGMModel::SPtr &newModel)
{
	try
	{	qDebug()<<"UUUU↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓UUUUUUUUU";	
		AGMModelPrinter::printWorld(newModel);
		AGMMisc::publishModification(newModel, agmagenttopic, worldModel, "ik");
	}
	catch(...)
	{
		exit(1);
	}
}



