/**
 *  @file   Navigator.h
 *  @brief 直進と後進できるクラス
 *  @author Harada,Oiwane
 */

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "Controller.h"
#include "Distance.h"
#include <cmath>

class Navigator {
 public:
  explicit Navigator(Controller& controller_);
  void move(double specifiedValue, int pwm = 30);

 private:
  Distance distance;
  Controller& controller;
  void forward(double specifiedValue, double goalDistance, int pwm);
  void backward(double specifiedValue, double goalDistance, int pwm);
  bool hasArrived(double goalDistance, bool isForward);
};

#endif