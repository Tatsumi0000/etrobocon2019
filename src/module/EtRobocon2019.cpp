#include "EtRobocon2019.h"
#include "Controller.h"
#include "Calibrator.h"
#include "Display.h"
#include "NormalCourse.h"

void EtRobocon2019::start()
{
  Controller controller;
  Calibrator calibrator(controller);
  calibrator.calibration();

  // コースと、カラーセンサー目標値の設定
  bool isRightCourse = calibrator.isLeftCource();
  int targetBrightness = (calibrator.getWhiteBrightness() + calibrator.getBlackBrightness()) / 2;

  // 右ボタンが押されるまで待つ
  while(!controller.buttonIsPressedRight()) {
  }

  NormalCourse normalCourse(controller, targetBrightness);
  normalCourse.selectedEdgeLR(isRightCourse);
}
