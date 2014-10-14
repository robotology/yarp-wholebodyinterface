/*
 * Copyright (C)2011  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Andrea Del Prete
 * email: andrea.delprete@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include <iCub/skinDynLib/common.h>
#include <yarp/os/Time.h>
#include <string>
#include <yarp/os/Log.h>
#include <yarp/math/api.h>

#include "yarpWholeBodyInterface/yarpWholeBodyStates.h"
#include "yarpWholeBodyInterface/yarpWbiUtil.h"

using namespace std;
using namespace wbi;
using namespace yarpWbi;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace iCub::skinDynLib;
using namespace iCub::ctrl;
using namespace yarp::math;

#define ESTIMATOR_PERIOD 10

// iterate over all body parts
#define FOR_ALL_BODY_PARTS(itBp)            FOR_ALL_BODY_PARTS_OF(itBp, jointIdList)
// iterate over all joints of all body parts
#define FOR_ALL(itBp, itJ)                  FOR_ALL_OF(itBp, itJ, jointIdList)


// *********************************************************************************************************************
// *********************************************************************************************************************
//                                          ICUB WHOLE BODY STATES
// *********************************************************************************************************************
// *********************************************************************************************************************
yarpWholeBodyStates::yarpWholeBodyStates(const char* _name, const yarp::os::Property & opt):
initDone(false), name(_name), wbi_yarp_properties(opt)
{
}

bool yarpWholeBodyStates::setYarpWbiProperties(const yarp::os::Property & yarp_wbi_properties)
{
    wbi_yarp_properties = yarp_wbi_properties;
    return true;
}

bool yarpWholeBodyStates::getYarpWbiProperties(yarp::os::Property & yarp_wbi_properties)
{
    yarp_wbi_properties = wbi_yarp_properties;
    return true;
}

yarpWholeBodyStates::~yarpWholeBodyStates()
{
    close();
}

bool yarpWholeBodyStates::init()
{
    if( !wbi_yarp_properties.check("robotName") )
    {
        std::cerr << "yarpWholeBodySensors error: robotName not found in configuration files" << std::endl;
        return false;
    }

    std::string robot = wbi_yarp_properties.find("robotName").asString().c_str();


    sensors = new yarpWholeBodySensors(name.c_str(), wbi_yarp_properties);              // sensor interface
    estimator = new yarpWholeBodyEstimator(ESTIMATOR_PERIOD, sensors);  // estimation thread
    bool ok = sensors->init();              // initialize sensor interface
    return ok ? estimator->start() : false; // start estimation thread
}

bool yarpWholeBodyStates::close()
{
    if(estimator) estimator->stop();  // stop estimator BEFORE closing sensor interface
    bool ok = (sensors ? sensors->close() : true);
    if(sensors) { delete sensors; sensors = 0; }
    if(estimator) { delete estimator; estimator = 0; }
    return ok;
}

bool yarpWholeBodyStates::setWorldBasePosition(const wbi::Frame & xB)
{
    return false;
    //return estimator->setWorldBasePosition(xB);
}

bool yarpWholeBodyStates::addEstimate(const EstimateType et, const wbiId &sid)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_VEL:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_ACC:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_TORQUE:             return lockAndAddSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return lockAndAddSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_POS:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_VEL:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_ACC:                return lockAndAddSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_TORQUE:             return lockAndAddSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return lockAndAddSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_PWM:                return lockAndAddSensor(SENSOR_PWM, sid);
    //case ESTIMATE_IMU:                      return lockAndAddSensor(SENSOR_IMU, sid);
    case ESTIMATE_FORCE_TORQUE_SENSOR:      return lockAndAddSensor(SENSOR_FORCE_TORQUE, sid);
    ///< \todo TODO properly account for external forque torque
    case ESTIMATE_EXTERNAL_FORCE_TORQUE:    return false; //estimator->openEEWrenchPorts();
    default: break;
    }
    return false;
}

int yarpWholeBodyStates::addEstimates(const EstimateType et, const wbiIdList &sids)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_JOINT_VEL:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_JOINT_ACC:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_JOINT_TORQUE:             return lockAndAddSensors(SENSOR_TORQUE, sids);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return lockAndAddSensors(SENSOR_TORQUE, sids);
    case ESTIMATE_MOTOR_POS:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_MOTOR_VEL:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_MOTOR_ACC:                return lockAndAddSensors(SENSOR_ENCODER, sids);
    case ESTIMATE_MOTOR_TORQUE:             return lockAndAddSensors(SENSOR_TORQUE, sids);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return lockAndAddSensors(SENSOR_TORQUE, sids);
    case ESTIMATE_MOTOR_PWM:                return lockAndAddSensors(SENSOR_PWM, sids);
    //case ESTIMATE_IMU:                    return lockAndAddSensors(SENSOR_IMU, sids);
    case ESTIMATE_FORCE_TORQUE_SENSOR:      return lockAndAddSensors(SENSOR_FORCE_TORQUE, sids);
    ///< \todo TODO properly account for external forque torque
    case ESTIMATE_EXTERNAL_FORCE_TORQUE:    return true;
    default: break;
    }
    return false;
}

bool yarpWholeBodyStates::removeEstimate(const EstimateType et, const wbiId &sid)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_VEL:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_ACC:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_JOINT_TORQUE:             return lockAndRemoveSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return lockAndRemoveSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_POS:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_VEL:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_ACC:                return lockAndRemoveSensor(SENSOR_ENCODER, sid);
    case ESTIMATE_MOTOR_TORQUE:             return lockAndRemoveSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return lockAndRemoveSensor(SENSOR_TORQUE, sid);
    case ESTIMATE_MOTOR_PWM:                return lockAndRemoveSensor(SENSOR_PWM, sid);
    //case ESTIMATE_IMU:                      return lockAndRemoveSensor(SENSOR_IMU, sid);
    case ESTIMATE_FORCE_TORQUE_SENSOR:             return lockAndRemoveSensor(SENSOR_FORCE_TORQUE, sid);
    default: break;
    }
    return false;
}

const wbiIdList& yarpWholeBodyStates::getEstimateList(const EstimateType et)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_JOINT_VEL:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_JOINT_ACC:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_JOINT_TORQUE:             return sensors->getSensorList(SENSOR_TORQUE);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return sensors->getSensorList(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_POS:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_VEL:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_ACC:                return sensors->getSensorList(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_TORQUE:             return sensors->getSensorList(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return sensors->getSensorList(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_PWM:                return sensors->getSensorList(SENSOR_PWM);
    //case ESTIMATE_IMU:                    return sensors->getSensorList(SENSOR_IMU);
    case ESTIMATE_FORCE_TORQUE_SENSOR:      return sensors->getSensorList(SENSOR_FORCE_TORQUE);
    /* ESTIMATE_EXTERNAL_FORCE_TORQUE: return list of links where is possible to get contact */
    default: break;
    }
    return emptyList;
}

int yarpWholeBodyStates::getEstimateNumber(const EstimateType et)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_JOINT_VEL:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_JOINT_ACC:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_JOINT_TORQUE:             return sensors->getSensorNumber(SENSOR_TORQUE);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return sensors->getSensorNumber(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_POS:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_VEL:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_ACC:                return sensors->getSensorNumber(SENSOR_ENCODER);
    case ESTIMATE_MOTOR_TORQUE:             return sensors->getSensorNumber(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return sensors->getSensorNumber(SENSOR_TORQUE);
    case ESTIMATE_MOTOR_PWM:                return sensors->getSensorNumber(SENSOR_PWM);
    //case ESTIMATE_IMU:                      return sensors->getSensorNumber(SENSOR_IMU);
    case ESTIMATE_FORCE_TORQUE_SENSOR:      return sensors->getSensorNumber(SENSOR_FORCE_TORQUE);
    /* ESTIMATE_EXTERNAL_FORCE_TORQUE: return number of links where is possible to get contact */
    default: break;
    }
    return 0;
}

bool yarpWholeBodyStates::getEstimate(const EstimateType et, const int numeric_id, double *data, double time, bool blocking)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastQ, data);
    case ESTIMATE_JOINT_VEL:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastDq, data);
    case ESTIMATE_JOINT_ACC:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastD2q, data);
    case ESTIMATE_JOINT_TORQUE:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastTauJ, data);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastDtauJ, data);
    case ESTIMATE_MOTOR_POS:
        return false;
    case ESTIMATE_MOTOR_VEL:
        return getMotorVel(numeric_id, data, time, blocking);
    case ESTIMATE_MOTOR_ACC:
        return false;
    case ESTIMATE_MOTOR_TORQUE:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastTauM, data);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:
        return estimator->lockAndCopyVectorElement(numeric_id, estimator->estimates.lastDtauM, data);
    case ESTIMATE_MOTOR_PWM:
        return lockAndReadSensor(SENSOR_PWM, numeric_id, data, time, blocking);
    //case ESTIMATE_IMU:
    //    return lockAndReadSensor(SENSOR_IMU, sid, data, time, blocking);
    case ESTIMATE_FORCE_TORQUE_SENSOR:
        return lockAndReadSensor(SENSOR_FORCE_TORQUE, numeric_id, data, time, blocking);
    case ESTIMATE_EXTERNAL_FORCE_TORQUE:
        return false; //lockAndGetExternalWrench(sid,data);
    default: break;
    }
    return false;
}

bool yarpWholeBodyStates::getEstimates(const EstimateType et, double *data, double time, bool blocking)
{
    switch(et)
    {
    case ESTIMATE_JOINT_POS:                return estimator->lockAndCopyVector(estimator->estimates.lastQ, data);
    case ESTIMATE_JOINT_VEL:                return estimator->lockAndCopyVector(estimator->estimates.lastDq, data);
    case ESTIMATE_JOINT_ACC:                return estimator->lockAndCopyVector(estimator->estimates.lastD2q, data);
    case ESTIMATE_JOINT_TORQUE:             return estimator->lockAndCopyVector(estimator->estimates.lastTauJ, data);
    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:  return estimator->lockAndCopyVector(estimator->estimates.lastDtauJ, data);
    case ESTIMATE_MOTOR_POS:                return false;
    case ESTIMATE_MOTOR_VEL:                return getMotorVel(data, time, blocking);
    case ESTIMATE_MOTOR_ACC:                return false;
    case ESTIMATE_MOTOR_TORQUE:             return estimator->lockAndCopyVector(estimator->estimates.lastTauM, data);
    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:  return estimator->lockAndCopyVector(estimator->estimates.lastDtauM, data);
    case ESTIMATE_MOTOR_PWM:                return lockAndReadSensors(SENSOR_PWM, data, time, blocking);
    //case ESTIMATE_IMU:                    return lockAndReadSensors(SENSOR_IMU, data, time, blocking);
    case ESTIMATE_FORCE_TORQUE_SENSOR:      return lockAndReadSensors(SENSOR_FORCE_TORQUE, data, time, blocking);
    default: break;
    }
    return false;
}

bool yarpWholeBodyStates::setEstimationParameter(const EstimateType et, const EstimationParameter ep, const void *value)
{
    return estimator->lockAndSetEstimationParameter(et, ep, value);
}

// *********************************************************************************************************************
// *********************************************************************************************************************
//                                          PRIVATE METHODS
// *********************************************************************************************************************
// *********************************************************************************************************************

bool yarpWholeBodyStates::getMotorVel(double *data, double time, bool /*blocking*/)
{
    bool res = estimator->lockAndCopyVector(estimator->estimates.lastDq, data);    ///< read joint vel
    if(!res) return false;
    /*
    wbiIdList idList = lockAndGetSensorList(SENSOR_ENCODER);
    int i=0;
    FOR_ALL_OF(itBp, itJ, idList)   ///< manage coupled joints
    {
        if(itBp->first == LEFT_ARM && *itJ==0)          // left arm shoulder
            data[i] = data[i];
        else if(itBp->first == LEFT_ARM && *itJ==1)     // left arm shoulder
            data[i] = data[i];
        else if(itBp->first == LEFT_ARM && *itJ==2)     // left arm shoulder
            data[i] = data[i];
        else if(itBp->first == RIGHT_ARM && *itJ==0)    // right arm shoulder
            data[i] = data[i];
        else  if(itBp->first == RIGHT_ARM && *itJ==1)   // right arm shoulder
            data[i] = data[i];
        else if(itBp->first == RIGHT_ARM && *itJ==2)    // right arm shoulder
            data[i] = data[i];
        else if(itBp->first == TORSO && *itJ==1)        // torso
            data[i] = data[i];
        else if(itBp->first == TORSO && *itJ==2)        // torso
            data[i] = data[i];
        i++;
    }
    */
    return false;
}

bool yarpWholeBodyStates::getMotorVel(const int numeric_id, double *data, double /*time*/, bool /*blocking*/)
{
    ///< read joint vel
    //return estimator->lockAndCopyVectorElement(sensors->getSensorList(SENSOR_ENCODER).localToGlobalId(lid), estimator->estimates.lastDq, data);
    return false;
}

bool yarpWholeBodyStates::lockAndReadSensors(const SensorType st, double *data, double /*time*/, bool blocking)
{
    estimator->mutex.wait();
    bool res = sensors->readSensors(st, data, 0, blocking);
    estimator->mutex.post();
    return res;
}

bool yarpWholeBodyStates::lockAndReadSensor(const SensorType st, const int numeric_id, double *data, double time, bool blocking)
{
    estimator->mutex.wait();
    bool res = sensors->readSensor(st, numeric_id, data, 0, blocking);
    estimator->mutex.post();
    return res;
}

/*
bool yarpWholeBodyStates::lockAndGetExternalWrench(const int numeric_id, double * data)
{
    return false;

    estimator->mutex.wait();
    if( !estimator->ee_wrenches_enabled )
    {
        estimator->mutex.post();
        return false;
    }

    if( sid == estimator->right_gripper_local_id )
    {
        memcpy(data,estimator->RAExtWrench.data(),6*sizeof(double));
    }
    else if( sid == estimator->left_gripper_local_id )
    {
        memcpy(data,estimator->LAExtWrench.data(),6*sizeof(double));
    }
    else if( sid == estimator->right_sole_local_id )
    {
        memcpy(data,estimator->RLExtWrench.data(),6*sizeof(double));
    }
    else if( sid == estimator->left_sole_local_id )
    {
        memcpy(data,estimator->LLExtWrench.data(),6*sizeof(double));
    }
    estimator->mutex.post();
    return true;
}
*/

bool yarpWholeBodyStates::lockAndAddSensor(const SensorType st, const wbiId &sid)
{
    estimator->mutex.wait();
    bool res = sensors->addSensor(st, sid);
    estimator->mutex.post();
    return res;
}

int yarpWholeBodyStates::lockAndAddSensors(const SensorType st, const wbiIdList &sids)
{
    estimator->mutex.wait();
    int res = sensors->addSensors(st, sids);
    estimator->mutex.post();
    return res;
}

bool yarpWholeBodyStates::lockAndRemoveSensor(const SensorType st, const wbiId &sid)
{
    estimator->mutex.wait();
    bool res = sensors->removeSensor(st, sid);
    estimator->mutex.post();
    return res;
}

wbiIdList yarpWholeBodyStates::lockAndGetSensorList(const SensorType st)
{
    estimator->mutex.wait();
    wbiIdList res = sensors->getSensorList(st);
    estimator->mutex.post();
    return res;
}

int yarpWholeBodyStates::lockAndGetSensorNumber(const SensorType st)
{
    estimator->mutex.wait();
    int res = sensors->getSensorNumber(st);
    estimator->mutex.post();
    return res;
}

// *********************************************************************************************************************
// *********************************************************************************************************************
//                                          ICUB WHOLE BODY ESTIMATOR
// *********************************************************************************************************************
// *********************************************************************************************************************
yarpWholeBodyEstimator::yarpWholeBodyEstimator(int _period, yarpWholeBodySensors *_sensors)
: RateThread(_period),
  sensors(_sensors),
  dqFilt(0),
  d2qFilt(0),
  dTauJFilt(0),
  dTauMFilt(0),
  tauJFilt(0),
  tauMFilt(0)//,
  //ee_wrenches_enabled(false)
{
    resizeAll(sensors->getSensorNumber(SENSOR_ENCODER));

    ///< Window lengths of adaptive window filters
    dqFiltWL            = 16;
    d2qFiltWL           = 25;
    dTauJFiltWL         = 30;
    dTauMFiltWL         = 30;

    ///< Threshold of adaptive window filters
    dqFiltTh            = 1.0;
    d2qFiltTh           = 1.0;
    dTauJFiltTh         = 0.2;
    dTauMFiltTh         = 0.2;

    ///< Cut frequencies
    tauJCutFrequency    =   3.0;
    tauMCutFrequency    =   3.0;
    pwmCutFrequency     =   3.0;
}

bool yarpWholeBodyEstimator::threadInit()
{
    resizeAll(sensors->getSensorNumber(SENSOR_ENCODER));
    ///< create derivative filters
    dqFilt = new AWLinEstimator(dqFiltWL, dqFiltTh);
    d2qFilt = new AWQuadEstimator(d2qFiltWL, d2qFiltTh);
    dTauJFilt = new AWLinEstimator(dTauJFiltWL, dTauJFiltTh);
    dTauMFilt = new AWLinEstimator(dTauMFiltWL, dTauMFiltTh);
    ///< read sensors
    assert((int)estimates.lastQ.size() == sensors->getSensorNumber(SENSOR_ENCODER));
    bool ok = sensors->readSensors(SENSOR_ENCODER, estimates.lastQ.data(), qStamps.data(), true);
    ok = ok && sensors->readSensors(SENSOR_TORQUE, estimates.lastTauJ.data(), tauJStamps.data(), true);
    ok = ok && sensors->readSensors(SENSOR_PWM, estimates.lastPwm.data(), 0, true);
    ///< create low pass filters
    tauJFilt    = new FirstOrderLowPassFilter(tauJCutFrequency, getRate()*1e-3, estimates.lastTauJ);
    tauMFilt    = new FirstOrderLowPassFilter(tauMCutFrequency, getRate()*1e-3, estimates.lastTauJ);
    pwmFilt     = new FirstOrderLowPassFilter(pwmCutFrequency, getRate()*1e-3, estimates.lastPwm);

    //H_world_base.resize(4,4);
    //H_world_base.eye();

    /*
    right_gripper_local_id = wbi::wbiId(RIGHT_ARM,8);
    left_gripper_local_id = wbi::wbiId(LEFT_ARM,8);
    left_sole_local_id = wbi::wbiId(LEFT_LEG,8);
    right_sole_local_id = wbi::wbiId(RIGHT_LEG,8);
    */

    return ok;
}

/*
bool yarpWholeBodyEstimator::openEEWrenchPorts()
{
    return false;

    bool okEE = true;
    if( ee_wrenches_enabled ) return true;
    mutex.wait();
    okEE = okEE && openEEWrenchPorts(right_gripper_local_id);
    okEE = okEE && openEEWrenchPorts(left_gripper_local_id);
    okEE = okEE && openEEWrenchPorts(right_sole_local_id);
    okEE = okEE && openEEWrenchPorts(left_sole_local_id);
    ee_wrenches_enabled = okEE;
    mutex.post();

    return okEE;

}*/

bool yarpWholeBodyEstimator::setWorldBasePosition(const wbi::Frame & xB)
{
    return false;
    /*
    mutex.wait();
    H_world_base.resize(4,4);
    xB.get4x4Matrix(H_world_base.data());
    mutex.post();
    return true;
    */
}

void yarpWholeBodyEstimator::run()
{
    mutex.wait();
    {
        resizeAll(sensors->getSensorNumber(SENSOR_ENCODER));

        ///< Read encoders
        if(sensors->readSensors(SENSOR_ENCODER, q.data(), qStamps.data(), false))
        {
            estimates.lastQ = q;
            AWPolyElement el;
            el.data = q;
            el.time = qStamps[0];
            estimates.lastDq = dqFilt->estimate(el);
            estimates.lastD2q = d2qFilt->estimate(el);
        }

        ///< Read joint torque sensors
        if(sensors->readSensors(SENSOR_TORQUE, tauJ.data(), tauJStamps.data(), false))
        {
            // @todo Convert joint torques into motor torques
            AWPolyElement el;
            el.time = yarp::os::Time::now();

            estimates.lastTauJ = tauJFilt->filt(tauJ);  ///< low pass filter
            estimates.lastTauM = tauMFilt->filt(tauJ);  ///< low pass filter

            el.data = tauJ;
            estimates.lastDtauJ = dTauJFilt->estimate(el);  ///< derivative filter

            //el.data = tauM;
            estimates.lastDtauM = dTauMFilt->estimate(el);  ///< derivative filter
        }

        ///< Read motor pwm
        sensors->readSensors(SENSOR_PWM, pwm.data(), 0, false);
        estimates.lastPwm = pwmFilt->filt(pwm);     ///< low pass filter

        ///< Read end effector wrenches
        /*
        if( ee_wrenches_enabled )
        {
            readEEWrenches(right_gripper_local_id,RAExtWrench);
            readEEWrenches(left_gripper_local_id,LAExtWrench);
            readEEWrenches(right_sole_local_id,RLExtWrench);
            readEEWrenches(left_sole_local_id,LLExtWrench);
        }*/
    }
    mutex.post();

    return;
}

/*
bool yarpWholeBodyEstimator::openEEWrenchPorts(const wbi::wbiId & local_id)
{
    std::string wbd_module_name = "wholeBodyDynamicsTree";
    std::string part, remotePort;
    part = getPartName(local_id);
    if( part == "")
    {
        std::cerr << "yarpWholeBodyEstimator::openEEWrenchPorts: local_id not found " << std::endl;
    }
    remotePort = "/" + wbd_module_name + "/" + part + "/" + "cartesianEndEffectorWrench:o";
    stringstream localPort;
    localPort << "/" << "yarpWholeBodyEstimator" << "/eeWrench" << local_id.bodyPart << "_" << local_id.index << ":i";
    portsEEWrenches[local_id] = new BufferedPort<Vector>();
    if(!portsEEWrenches[local_id]->open(localPort.str().c_str())) {
        // open local input port
        std::cerr << " yarpWholeBodyEstimator::openEEWrenchPorts: Open of localPort " << localPort << " failed " << std::endl;
        //YARP_ASSERT(false);
        ee_wrenches_enabled = false;
        return false;
    }
    if(!Network::exists(remotePort.c_str())) {            // check remote output port exists
        std::cerr << "yarpWholeBodyEstimator::openEEWrenchPorts:  " << remotePort << " does not exist " << std::endl;
        //YARP_ASSERT(false);
        ee_wrenches_enabled = false;
        return false;
    }
    if(!Network::connect(remotePort.c_str(), localPort.str().c_str(), "udp")) {  // connect remote to local port
        std::cerr << "yarpWholeBodyEstimator::openEEWrenchPorts:  could not connect " << remotePort << " to " << localPort << std::endl;
        //YARP_ASSERT(false);
        ee_wrenches_enabled = false;
        return false;
    }

    //allocate lastRead variables
    lastEEWrenches[local_id].resize(6,0.0);
    ee_wrenches_enabled = true;

    return true;
}

void yarpWholeBodyEstimator::readEEWrenches(const wbi::wbiId & local_id, yarp::sig::Vector & vec)
{
    vec.resize(6);
    vec.zero();
    if( ee_wrenches_enabled )
    {
        yarp::sig::Vector*res = portsEEWrenches[local_id]->read();
        if( res )
        {
            lastEEWrenches[local_id].setSubvector(0,H_world_base.submatrix(0,2,0,2)*(*res).subVector(0,2));
            lastEEWrenches[local_id].setSubvector(3,H_world_base.submatrix(0,2,0,2)*(*res).subVector(3,5));
        }
        vec = lastEEWrenches[local_id];
    }
}

void yarpWholeBodyEstimator::closeEEWrenchPorts(const wbi::wbiId & local_id)
{
    if( ee_wrenches_enabled )
    {
        portsEEWrenches[local_id]->close();
        delete portsEEWrenches[local_id];
    }
}*/

void yarpWholeBodyEstimator::threadRelease()
{
    //this causes a memory access violation (to investigate)
    if(dqFilt!=0)    { delete dqFilt;  dqFilt=0; }
    if(d2qFilt!=0)   { delete d2qFilt; d2qFilt=0; }
    if(dTauJFilt!=0) { delete dTauJFilt; dTauJFilt=0; }
    if(dTauMFilt!=0) { delete dTauMFilt; dTauMFilt=0; }     // motor torque derivative filter
    if(tauJFilt!=0)  { delete tauJFilt; tauJFilt=0; }  ///< low pass filter for joint torque
    if(tauMFilt!=0)  { delete tauMFilt; tauMFilt=0; }  ///< low pass filter for motor torque
    if(pwmFilt!=0)   { delete pwmFilt; pwmFilt=0;   }

    /*
    if( ee_wrenches_enabled )
    {
        closeExternalWrenchPorts(right_gripper_local_id);
    }*/

    return;
}

void yarpWholeBodyEstimator::lockAndResizeAll(int n)
{
    mutex.wait();
    resizeAll(n);
    mutex.post();
}

void yarpWholeBodyEstimator::resizeAll(int n)
{
    q.resize(n);
    qStamps.resize(n);
    tauJ.resize(n);
    tauJStamps.resize(n);
    pwm.resize(n);
    pwmStamps.resize(n);
    estimates.lastQ.resize(n);
    estimates.lastDq.resize(n);
    estimates.lastD2q.resize(n);
    estimates.lastTauJ.resize(n);
    estimates.lastTauM.resize(n);
    estimates.lastDtauJ.resize(n);
    estimates.lastDtauM.resize(n);
    estimates.lastPwm.resize(n);
}

bool yarpWholeBodyEstimator::lockAndCopyVector(const Vector &src, double *dest)
{
    if(dest==0)
        return false;
    mutex.wait();
    memcpy(dest, src.data(), sizeof(double)*src.size());
    mutex.post();
    return true;
}

bool yarpWholeBodyEstimator::lockAndCopyVectorElement(int index, const Vector &src, double *dest)
{
    mutex.wait();
    dest[0] = src[index];
    mutex.post();
    return true;
}

bool yarpWholeBodyEstimator::lockAndSetEstimationParameter(const EstimateType et, const EstimationParameter ep, const void *value)
{
    bool res = false;
    mutex.wait();
    switch(et)
    {
    case ESTIMATE_JOINT_VEL:
    case ESTIMATE_MOTOR_VEL:
        if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_MAX_SIZE)
            res = setVelFiltParams(((int*)value)[0], dqFiltTh);
        else if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_THRESHOLD)
            res = setVelFiltParams(dqFiltWL, ((double*)value)[0]);
        break;

    case ESTIMATE_JOINT_ACC:
    case ESTIMATE_MOTOR_ACC:
        if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_MAX_SIZE)
            res = setAccFiltParams(((int*)value)[0], d2qFiltTh);
        else if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_THRESHOLD)
            res = setAccFiltParams(d2qFiltWL, ((double*)value)[0]);
        break;

    case ESTIMATE_JOINT_TORQUE:
        if(ep==ESTIMATION_PARAM_LOW_PASS_FILTER_CUT_FREQ)
            res = setTauJCutFrequency(((double*)value)[0]);
        break;

    case ESTIMATE_JOINT_TORQUE_DERIVATIVE:
        if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_MAX_SIZE)
            res = setDtauJFiltParams(((int*)value)[0], dTauJFiltTh);
        else if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_THRESHOLD)
            res = setDtauJFiltParams(dTauJFiltWL, ((double*)value)[0]);
        break;

    case ESTIMATE_MOTOR_TORQUE:
        if(ep==ESTIMATION_PARAM_LOW_PASS_FILTER_CUT_FREQ)
            res = setTauMCutFrequency(((double*)value)[0]);
        break;

    case ESTIMATE_MOTOR_TORQUE_DERIVATIVE:
        if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_MAX_SIZE)
            res = setDtauMFiltParams(((int*)value)[0], dTauMFiltTh);
        else if(ep==ESTIMATION_PARAM_ADAPTIVE_WINDOW_THRESHOLD)
            res = setDtauMFiltParams(dTauMFiltWL, ((double*)value)[0]);
        break;

    case ESTIMATE_MOTOR_PWM:
        if(ep==ESTIMATION_PARAM_LOW_PASS_FILTER_CUT_FREQ)
            res = setPwmCutFrequency(((double*)value)[0]);
        break;

    //case ESTIMATE_IMU:
    case ESTIMATE_FORCE_TORQUE_SENSOR:
    case ESTIMATE_JOINT_POS:
    case ESTIMATE_MOTOR_POS:
    default: break;
    }
    mutex.post();
    return res;
}

bool yarpWholeBodyEstimator::setVelFiltParams(int windowLength, double threshold)
{
    if(windowLength<1 || threshold<=0.0)
        return false;
    dqFiltWL = windowLength;
    dqFiltTh = threshold;
    if(dqFilt!=NULL)
    {
        AWPolyList list = dqFilt->getList();
        dqFilt = new AWLinEstimator(dqFiltWL, dqFiltTh);
        for(AWPolyList::iterator it=list.begin(); it!=list.end(); it++)
            dqFilt->feedData(*it);
    }
    return true;
}

bool yarpWholeBodyEstimator::setAccFiltParams(int windowLength, double threshold)
{
    if(windowLength<1 || threshold<=0.0)
        return false;
    d2qFiltWL = windowLength;
    d2qFiltTh = threshold;
    if(d2qFilt!=NULL)
    {
        AWPolyList list = d2qFilt->getList();
        d2qFilt = new AWQuadEstimator(d2qFiltWL, d2qFiltTh);
        for(AWPolyList::iterator it=list.begin(); it!=list.end(); it++)
            d2qFilt->feedData(*it);
    }
    return true;
}

bool yarpWholeBodyEstimator::setDtauJFiltParams(int windowLength, double threshold)
{
    if(windowLength<1 || threshold<=0.0)
        return false;
    dTauJFiltWL = windowLength;
    dTauJFiltTh = threshold;
    if(dTauJFilt!=NULL)
    {
        AWPolyList list = dTauJFilt->getList();
        dTauJFilt = new AWLinEstimator(windowLength, threshold);
        for(AWPolyList::iterator it=list.begin(); it!=list.end(); it++)
            dTauJFilt->feedData(*it);
    }
    return true;
}

bool yarpWholeBodyEstimator::setDtauMFiltParams(int windowLength, double threshold)
{
    if(windowLength<1 || threshold<=0.0)
        return false;
    dTauMFiltWL = windowLength;
    dTauMFiltTh = threshold;
    if(dTauMFilt!=NULL)
    {
        AWPolyList list = dTauMFilt->getList();
        dTauMFilt = new AWLinEstimator(windowLength, threshold);
        for(AWPolyList::iterator it=list.begin(); it!=list.end(); it++)
            dTauMFilt->feedData(*it);
    }
    return true;
}

bool yarpWholeBodyEstimator::setTauJCutFrequency(double fc)
{
    return tauJFilt->setCutFrequency(fc);
}

bool yarpWholeBodyEstimator::setTauMCutFrequency(double fc)
{
    return tauMFilt->setCutFrequency(fc);
}

bool yarpWholeBodyEstimator::setPwmCutFrequency(double fc)
{
    return pwmFilt->setCutFrequency(fc);
}