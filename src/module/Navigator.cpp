/**
 *  @file   Navigator.cpp
 *  @brief 直進と後進できるクラス
 *  @author Harada,Oiwane
 *  updated by T.Miyaji
 */

#include "Navigator.h"

Navigator::Navigator(Controller& controller_, int targetBrightness_)
  : distance(), controller(controller_), targetBrightness(targetBrightness_)
{
}

void Navigator::move(double specifiedDistance, int pwm, const double pGain)
{
  controller.resetMotorCount();

  // 左車輪の回転量 - 右車輪の回転量
  // 左車輪の方が多く回転していれば、alphaは正となり右車輪にPWM + alphaの操作量が加えられる
  Pid pid(0, pGain);

  if(specifiedDistance < 0) {
    while(hasArrived(specifiedDistance, false)) {
      double alpha = pid.control(controller.getLeftMotorCount() - controller.getRightMotorCount());
      setPwmValue(-std::abs(pwm), -alpha);
      controller.tslpTsk(4);
    }
  } else {
    while(hasArrived(specifiedDistance, true)) {
      double alpha = pid.control(controller.getLeftMotorCount() - controller.getRightMotorCount());
      setPwmValue(std::abs(pwm), -alpha);
      controller.tslpTsk(4);
    }
  }

  controller.stopMotor();
}

void Navigator::moveToSpecifiedColor(Color specifiedColor, int pwm)
{
  // 特定の色まで移動する
  while(controller.getColor() != specifiedColor) {
    setPwmValue(pwm);
    controller.tslpTsk(4);
  }
  controller.stopMotor();
}

void Navigator::spin(double angle, bool clockwise, int pwm, double weight)
{
  // angleの絶対値を取る
  angle = std::abs(angle);
  Rotation rotation;
  Filter<> filter;

  controller.resetMotorCount();
  controller.resetGyroSensor();

  double motorAngle
      = rotation.calculate(controller.getLeftMotorCount(), controller.getRightMotorCount());
  double gyroAngle = std::abs(static_cast<double>(controller.getAngleOfRotation()));

  while(filter.complementaryFilter(motorAngle, gyroAngle) < angle) {
    controller.setLeftMotorPwm(clockwise ? pwm : -pwm);
    controller.setRightMotorPwm(clockwise ? -pwm : pwm);
    controller.tslpTsk(4);

    motorAngle
        = rotation.calculate(controller.getLeftMotorCount(), controller.getRightMotorCount());
    gyroAngle = std::abs(static_cast<double>(controller.getAngleOfRotation()));
  }

  controller.stopMotor();
}

bool Navigator::hasArrived(double goalDistance, bool isForward)
{
  int leftAngle = controller.getLeftMotorCount();
  int rightAngle = controller.getRightMotorCount();

  // スタート地点からみた現在の距離
  int currentDistance = distance.getDistance(leftAngle, rightAngle);

  if(isForward) {
    return currentDistance <= goalDistance;
  } else {
    return currentDistance >= goalDistance;
  }
}

void Navigator::setPwmValue(int pwm, double alpha)
{
  controller.setRightMotorPwm(pwm + static_cast<int>(alpha));
  controller.setLeftMotorPwm(pwm - static_cast<int>(alpha));
}

void Navigator::traceBlackLine(double specifiedDistance, int pwm, double encoderPGain,
                               double lineTracePGain, bool isLeft)
{
  controller.resetMotorCount();

  // 左車輪の回転量 - 右車輪の回転量
  // 左車輪の方が多く回転していれば、alphaは正となり右車輪にPWM + alphaの操作量が加えられる
  Pid pidForEncoder(0, lineTracePGain);

  Pid pidForLineTrace(targetBrightness, lineTracePGain);

  if(specifiedDistance < 0) {
    // 指定した距離に到達するまでTrue
    while(hasArrived(specifiedDistance, false)) {
      // エンコーダーの値をp制御
      double encoderPidValue
          = pidForEncoder.control(controller.getLeftMotorCount() - controller.getRightMotorCount());
      // ライントレース用のp制御
      double lineTracePidValue = pidForLineTrace.control(controller.getBrightness());

      // 後進するため、pwmは負の値
      int rightPwm = -std::abs(pwm)
                     + static_cast<int>(isLeft ? lineTracePidValue : -lineTracePidValue)
                     - static_cast<int>(encoderPidValue);
      int leftPwm = -std::abs(pwm)
                    + static_cast<int>(isLeft ? -lineTracePidValue : lineTracePidValue)
                    + static_cast<int>(encoderPidValue);

      controller.setRightMotorPwm(rightPwm);
      controller.setLeftMotorPwm(leftPwm);

      controller.tslpTsk(4);
    }
  } else {
    // 上記のwhile文の中の処理とほぼ同じ
    while(hasArrived(specifiedDistance, true)) {
      double encoderPidValue
          = pidForEncoder.control(controller.getLeftMotorCount() - controller.getRightMotorCount());
      double lineTracePidValue = pidForLineTrace.control(controller.getBrightness());

      // 前進するため、pwmは正の値
      int rightPwm = std::abs(pwm)
                     + static_cast<int>(isLeft ? lineTracePidValue : -lineTracePidValue)
                     - static_cast<int>(encoderPidValue);
      int leftPwm = std::abs(pwm)
                    + static_cast<int>(isLeft ? -lineTracePidValue : lineTracePidValue)
                    + static_cast<int>(encoderPidValue);

      controller.setRightMotorPwm(rightPwm);
      controller.setLeftMotorPwm(leftPwm);

      controller.tslpTsk(4);
    }
  }

  controller.stopMotor();
}

void Navigator::traceBlackLineToSpecifiedColor(Color specifiedColor, int pwm, double lineTracePGain,
                                               bool isLeft)
{
  TurnControl turnControl(targetBrightness, 0.0, 0.0, 0.0);

  // 特定の色まで移動する
  while(controller.getColor() != specifiedColor) {
    double pidValue = turnControl.calculateTurn(pwm, controller.getBrightness(), targetBrightness,
                                                lineTracePGain, 0.0, 0.0);
    this->setPwmValue(pwm, (isLeft ? pidValue : -pidValue));
    controller.tslpTsk(4);
  }
  controller.stopMotor();
}