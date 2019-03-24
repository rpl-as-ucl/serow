/*
 * humanoid_state_estimation - a complete state estimation scheme for humanoid robots
 *
 * Copyright 2017-2018 Stylianos Piperakis, Foundation for Research and Technology Hellas (FORTH)
 * License: BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Foundation for Research and Technology Hellas (FORTH) 
 *	 nor the names of its contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef  __IMUinEKF_H__
#define  __IMUinEKF_H__

#include <iostream>
#include <eigen3/Eigen/Dense>
#include <cmath>       /* isnan, sqrt */
/* @author MUMRA
 */

/* @brief  IMU-Kinematics/Encoders-VO-LO fusion
 * dx/dt=f(x,u,w)
 * y_k=Hx_k+v
 * State is  position in world frame : pwb
 * velocity in  world frame : vwb
 * orientation of Body frame wrt the World frame: Rwb
 * accelerometer bias in Body frame : bacc
 * gyro bias in Body frame : bgyr
 * Measurement is Body Position/Orinetation in Inertial frame by Odometry-VO
 * and the relative Support Foot Position/Orientation by the Kinematics/Encoders
 */

 
using namespace Eigen;

class IMUinEKF {

private:



	Matrix<double, 21, 21> P, Af, Adj, Phi, If, Qff, Qf;
	Matrix3d I;
	Vector3d  w_, a_, w, a;
	
	//State vector - with biases included
	Matrix<double, 7, 7> X;
	Matrix<double, 6, 1> theta;
	void updateStateSingleContact(Matrix<double,7,1> Y, Matrix<double,7,1> b, Matrix<double,3,21> H, Matrix3d N, Matrix<double,3,7> PI);
	void updateStateDoubleContact(Matrix<double,14,1>Y, Matrix<double,14,1> b, Matrix<double,6,21> H, Matrix<double,6,6> N, Matrix<double,6,14> PI);


public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
             


	bool firstrun;

	// Gravity vector
    Vector3d g, pwb, vwb, dR, dL, bgyr, bacc, gyro, acc,  angle;

	//Noise Stds

	double  acc_qx,acc_qy,acc_qz,gyr_qx,gyr_qy,gyr_qz,gyrb_qx,gyrb_qy,gyrb_qz,
	accb_qx,accb_qy,accb_qz, foot_contactx, foot_contacty, foot_contactz, foot_kinx, foot_kiny, foot_kinz;

	double gyroX, gyroY, gyroZ, angleX, angleY, angleZ, bias_gx, bias_gy, bias_gz,
			bias_ax, bias_ay, bias_az, ghat;

	double accX, accY, accZ, velX, velY, velZ, rX, rY, rZ;
	Matrix3d Rwb, Rib, Qc, R;

	Affine3d  Tib;

	Quaterniond qib;

	//Sampling time = 1.0/Sampling Frequency
	double dt;

	IMUinEKF();

	void constructState(Matrix<double,7,7>&X_, Matrix<double,6,1> &theta_, Matrix3d R_, Vector3d v_, Vector3d  p_, Vector3d dR_,  Vector3d dL_, Vector3d bg_, Vector3d ba_);
	void seperateState(Matrix<double,7,7>X_, Matrix<double,6,1> theta_, Matrix3d& R_, Vector3d& v_, Vector3d&  p_, Vector3d& dR_,  Vector3d& dL_, Vector3d& bg_, Vector3d& ba_);
	Matrix<double,7,7> exp(Matrix<double,15,1> v);
	Matrix3d exp_SO3(Vector3d v);
	Matrix<double,21,21> Adjoint(Matrix<double,7,7> X_);





	void updateVars();

	void setdt(double dtt) {
		dt = dtt;
	}

	void setGyroBias(Vector3d bgyr_)
	{
		bgyr = bgyr_;
		theta.segment<3>(0) = bgyr;
		bias_gx = bgyr(0);
		bias_gy = bgyr(1);
		bias_gz = bgyr(2);
	}
	void setAccBias(Vector3d bacc_)
	{
		bacc = bacc_;
		theta.segment<3>(3) = bacc;
		bias_ax = bacc(0);
		bias_ay = bacc(1);
		bias_az = bacc(2);
	}
	//Initialize the Position
	void setBodyPos(Vector3d bp)
	{
		pwb = bp;
	}

	//Initialize the Rotation Matrix and the Orientation Error
	void setBodyOrientation(Matrix3d bR){
		Rwb = bR;
	}

    void setBodyVel(Vector3d bv)
    {
        vwb = bv;
    }


	/** @fn void Filter(Matrix<double,3,1> f, Matrix<double,3,1> omega, Matrix<double,3,1>  y_r, Matrix<double,3,1>  y_q)
	 *  @brief filters the acceleration measurements from the IMU
	 */
	void predict(Vector3d angular_velocity, Vector3d linear_acceleration, Vector3d hR_R, Vector3d hR_L, int contactR, int contactL);
	void updateKinematics(Vector3d s_pR, Vector3d s_pL, Matrix3d JRQeJR, Matrix3d JLQeJL, int contactL, int contactR);


	// Initializing Variables
	void init();

	//Computes the skew symmetric matrix of a 3-D vector
	Matrix3d skew(Vector3d v) 
	{
		Matrix3d res;
		res = Matrix3d::Zero();
		res(0, 1) = -v(2);
		res(0, 2) = v(1);
		res(1, 2) = -v(0);
		res(1, 0) = v(2);
		res(2, 0) = -v(1);
		res(2, 1) = v(0);
		return res;
	}
	Vector3d vec(Matrix3d M)
	{
		Vector3d v = Vector3d::Zero();
		v(0) = M(2,1);
		v(1) = M(0,2);
		v(2) = M(1,0);
		return v;
	}

		
	//Get the Euler Angles from a Rotation Matrix
	inline Vector3d getEulerAngles(
			Matrix3d Rt) {
		Vector3d res;
		res = Vector3d::Zero();

		res(0) = atan2(Rt(2, 1), Rt(2, 2));
		res(1) = atan2(-Rt(2, 0), sqrt(pow(Rt(2, 1), 2) + pow(Rt(2, 2), 2)));
		res(2) = atan2(Rt(1, 0), Rt(0, 0));
		return res;
	}
};
#endif