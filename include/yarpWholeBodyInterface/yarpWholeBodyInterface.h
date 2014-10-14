/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Andrea Del Prete
 * email: andrea.delprete@iit.it
 *
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

#ifndef WBI_YARP_H
#define WBI_YARP_H

#include <yarpWholeBodyInterface/yarpWbiUtil.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/IVelocityControl2.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/BufferedPort.h>
#include <iCub/ctrl/adaptWinPolyEstimator.h>
#include <iCub/ctrl/filters.h>
#include <iCub/iDynTree/iCubTree.h>
#include <iCub/skinDynLib/skinContactList.h>
#include <map>

#define INITIAL_TIMESTAMP -1000.0

#include "yarpWholeBodyInterface/yarpWholeBodyActuators.h"
#include "yarpWholeBodyInterface/yarpWholeBodyModel.h"
#include "yarpWholeBodyInterface/yarpWholeBodyStates.h"

/* CODE UNDER DEVELOPMENT */

namespace yarpWbi
{

    /*
    const int JOINT_ESTIMATE_TYPES_SIZE = 3;
    ///< estimate types that are automatically added when calling addJoint(s) and automatically removed when calling removeJoint
    const wbi::EstimateType jointEstimateTypes[JOINT_ESTIMATE_TYPES_SIZE] =
    {
        wbi::ESTIMATE_JOINT_POS,         // joint position
        //wbi::ESTIMATE_JOINT_VEL,         // joint velocity
        //wbi::ESTIMATE_JOINT_ACC,         // joint acceleration
        wbi::ESTIMATE_JOINT_TORQUE,      // joint torque
        //wbi::ESTIMATE_MOTOR_VEL,         // motor velocity
        //wbi::ESTIMATE_MOTOR_TORQUE,      // motor torque
        wbi::ESTIMATE_MOTOR_PWM,         // motor PWM (proportional to motor voltage)
    };*/

    /**
     * Class to communicate with iCub.
     */
    class yarpWholeBodyInterface : public wbi::wholeBodyInterface
    {
    protected:
        yarpWholeBodyActuators  *actuatorInt;
        yarpWholeBodyModel      *modelInt;
        yarpWholeBodyStates     *stateInt;

        wbi::wbiIdList empty_id_list;

    public:
        // *** CONSTRUCTORS ***
        yarpWholeBodyInterface(const char* _interfaceName,
                               yarp::os::Property & _yarp_wbi_properties);


        inline virtual ~yarpWholeBodyInterface(){ close(); }
        virtual bool init();
        virtual bool close();
        virtual bool removeJoint(const wbi::wbiId &j);
        virtual bool addJoint(const wbi::wbiId &j);
        virtual int addJoints(const wbi::wbiIdList &j);

        // ACTUATORS
        //virtual int getActuatorNumber(){                        return actuatorInt->getActuatorNumber(); }
        virtual bool removeActuator(const wbi::wbiId &j){     return actuatorInt->removeActuator(j); }
        virtual bool addActuator(const wbi::wbiId &j){        return actuatorInt->addActuator(j); }
        virtual int addActuators(const wbi::wbiIdList &j){    return actuatorInt->addActuators(j); }
        virtual const wbi::wbiIdList& getActuatorList(){      return actuatorInt->getActuatorList(); }
        virtual bool setControlMode(wbi::ControlMode cm, double *ref=0, int jnt=-1)
        { return actuatorInt->setControlMode(cm, ref, jnt); }
        virtual bool setControlReference(double *ref, int jnt=-1)
        { return actuatorInt->setControlReference(ref, jnt); }
        virtual bool setControlParam(wbi::ControlParam parId, const void *val, int jnt=-1)
        { return actuatorInt->setControlParam(parId, val, jnt); }

        // STATES
        virtual bool addEstimate(const wbi::EstimateType st, const wbi::wbiId &sid){      return false; }
        virtual int addEstimates(const wbi::EstimateType st, const wbi::wbiIdList &sids){ return 0; }
        virtual bool removeEstimate(const wbi::EstimateType st, const wbi::wbiId &sid){   return false; }
        virtual const wbi::wbiIdList& getEstimateList(const wbi::EstimateType st){        return empty_id_list; }
        virtual int getEstimateNumber(const wbi::EstimateType st){                          return 0; }
        virtual bool getEstimate(const wbi::EstimateType et, const wbi::wbiId &sid, double *data, double time=-1.0, bool blocking=true)
        { return false; }
        virtual bool getEstimates(const wbi::EstimateType et, double *data, double time=-1.0, bool blocking=true)
        { return false; }
        virtual bool setEstimationParameter(const wbi::EstimateType et, const wbi::EstimationParameter ep, const void *value)
        { return false; }

        // MODEL
        virtual int getDoFs(){ return modelInt->getDoFs(); }
        virtual const wbi::wbiIdList& getJointList(){ return modelInt->getJointList(); }
        virtual bool getLinkId(const char *linkName, int &linkId)
        { return modelInt->getLinkId(linkName, linkId); }
        virtual bool getJointLimits(double *qMin, double *qMax, int joint=-1)
        { return modelInt->getJointLimits(qMin, qMax, joint); }
        virtual bool computeH(double *q, const wbi::Frame &xB, int linkId, wbi::Frame &H)
        { return modelInt->computeH(q, xB, linkId, H); }
        virtual bool computeJacobian(double *q, const wbi::Frame &xB, int linkId, double *J, double *pos=0)
        { return modelInt->computeJacobian(q, xB, linkId, J, pos); }
        virtual bool computeDJdq(double *q, const wbi::Frame &xB, double *dq, double *dxB, int linkId, double *dJdq, double *pos=0)
        { return modelInt->computeDJdq(q, xB, dq, dxB, linkId, dJdq, pos); }
        virtual bool forwardKinematics(double *q, const wbi::Frame &xB, int linkId, double *x)
        { return modelInt->forwardKinematics(q, xB, linkId, x); }
        virtual bool inverseDynamics(double *q, const wbi::Frame &xB, double *dq, double *dxB, double *ddq, double *ddxB, double *g, double *tau)
        { return modelInt->inverseDynamics(q, xB, dq, dxB, ddq, ddxB, g,tau); }
        virtual bool computeMassMatrix(double *q, const wbi::Frame &xB, double *M)
        { return modelInt->computeMassMatrix(q, xB, M); }
        virtual bool computeGeneralizedBiasForces(double *q, const wbi::Frame &xB, double *dq, double *dxB, double *g, double *h)
        { return modelInt->computeGeneralizedBiasForces(q, xB, dq, dxB, g, h); }
         virtual bool computeCentroidalMomentum(double *q, const wbi::Frame &xB, double *dq, double *dxB, double *h)
        { return modelInt->computeCentroidalMomentum(q, xB, dq, dxB, h); }

    };

} // end namespace yarpWbi

#endif