// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//Copyright 2022 Shigeoka Kodai. All Rights Reserved.
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//        limitations under the License.

#define __RAY_TRACING__
#define __ANDROID_MAIN__
#include <android/log.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "VulkanMain.hpp"
#include <unistd.h>

glm::vec2 touchPositions[2] = {glm::vec2(0.0f), glm::vec2(0.0f)};
bool isTouched = false;
bool isFocused = false;

struct Sensors {
    ASensorManager *gSensorManager;
    ASensorEventQueue *gSensorQueue;
    const ASensor *gSensorGravity;
    const ASensor *gSensorGyroscope;
    const ASensor *gSensorLinearAccelerometer;
    ASensorEvent *gSensorEvent;
} gSensors;
glm::vec3 gravityData(0.0f);
glm::vec3 lastGravityData(0.0f);
glm::vec3 laData(0.0f);
glm::vec3 lastLaData(0.0f);

ASensorEvent* tempSensorEvent = new ASensorEvent();

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      InitVulkan(app);
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      DeleteVulkan();
      break;
//    case APP_CMD_INPUT_CHANGED:
//        __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorials onAppCmd",
//                            "input change event received %d", cmd);
//          break;
    default:
      __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorials onAppCmd",
                          "event not handled: %d", cmd);
  }
}

int32_t handle_input(struct android_app* app, AInputEvent* inputEvent)
{
  int32_t eventType = AInputEvent_getType(inputEvent);
  switch (eventType)
  {
      case AINPUT_EVENT_TYPE_KEY :
        break;
      case AINPUT_EVENT_TYPE_MOTION :
        isTouched = true;
        touchPositions[0].x = AMotionEvent_getX(inputEvent, 0);
        touchPositions[0].y = AMotionEvent_getY(inputEvent, 0);
        touchPositions[1].x = AMotionEvent_getX(inputEvent, 1);
        touchPositions[1].y = AMotionEvent_getY(inputEvent, 1);
        break;
      case AINPUT_EVENT_TYPE_FOCUS :
          isFocused = true;
          break;
  }
  return 0;
}

int SensorEvents(int fd, int events, void* data)
{
    ASensorEvent event;

    while(ASensorEventQueue_getEvents(gSensors.gSensorQueue, &event, 1) > 0)
    {
        if(event.type == ASENSOR_TYPE_GRAVITY)
        {
            gravityData.x = event.vector.x;
            gravityData.y = event.vector.y;
            gravityData.z = event.vector.z;
            __android_log_print(ANDROID_LOG_DEBUG, "gravity :  ", (std::to_string(gravityData.x) + " " + std::to_string(gravityData.y) +
            " " + std::to_string(gravityData.z)).c_str(), 10);
        }
    }
    return 0;
}

void InitSensor(struct android_app* app)
{
    gSensors.gSensorManager = ASensorManager_getInstance();
//    gSensors.gSensorLinearAccelerometer = ASensorManager_getDefaultSensor(gSensors.gSensorManager, ASENSOR_TYPE_LINEAR_ACCELERATION);
//    gSensors.gSensorGravity = ASensorManager_getDefaultSensor(gSensors.gSensorManager, ASENSOR_TYPE_GRAVITY);
    gSensors.gSensorGyroscope = ASensorManager_getDefaultSensor(gSensors.gSensorManager, ASENSOR_TYPE_GYROSCOPE);
    gSensors.gSensorQueue = ASensorManager_createEventQueue(gSensors.gSensorManager, app->looper, LOOPER_ID_USER, NULL, NULL);
//    __android_log_print(ANDROID_LOG_DEBUG, "register sensor test LA",
//                        std::to_string(ASensorEventQueue_registerSensor(gSensors.gSensorQueue, gSensors.gSensorLinearAccelerometer, 10, 10)).c_str(), 10);
//    __android_log_print(ANDROID_LOG_DEBUG, "register sensor test gravity",
//                        std::to_string(ASensorEventQueue_registerSensor(gSensors.gSensorQueue, gSensors.gSensorGravity, 10, 10)).c_str(), 10);
    __android_log_print(ANDROID_LOG_DEBUG, "sensor test gyro",
                        std::to_string(ASensorEventQueue_registerSensor(gSensors.gSensorQueue, gSensors.gSensorGyroscope, 10, 10)).c_str(), 10);
//    __android_log_print(ANDROID_LOG_DEBUG, "register sensor test LA",
//                        std::to_string(ASensorEventQueue_registerSensor(gSensors.gSensorQueue, gSensors.gSensorLinearAccelerometer, 10, 10)).c_str(), 10);
//    __android_log_print(ANDROID_LOG_DEBUG, "sensor test gravity",
//                        std::to_string(ASensorEventQueue_enableSensor(gSensors.gSensorQueue, gSensors.gSensorGravity)).c_str(), 0);
    __android_log_print(ANDROID_LOG_DEBUG, "sensor test gyro",
                        std::to_string(ASensorEventQueue_enableSensor(gSensors.gSensorQueue, gSensors.gSensorGyroscope)).c_str(), 0);
//    auto delaytime = ASensor_getMinDelay(gSensors.gSensorGravity);
//    ASensorEventQueue_setEventRate(gSensors.gSensorQueue, gSensors.gSensorAccelerometer, delaytime);
//    ASensorEventQueue_setEventRate(gSensors.gSensorQueue, gSensors.gSensorGravity, delaytime);
}


void android_main(struct android_app* app) {

  // Set the callback to process system events
  app->userData = &gSensors;
  app->onAppCmd = handle_cmd;
  app->onInputEvent = handle_input;

  // Used to poll the events in the main loop
  int ident;
  int events;
  android_poll_source* source;
  uint32_t currentFrame = 0;
  //init sensor
  InitSensor(app);
  // Main loop
  do {
    if ((ident = ALooper_pollAll(IsVulkanReady() ? 60 : -1, nullptr,
                        &events, (void**)&source)) >= 0) {
      if (source != NULL) source->process(app, source);
      if(ident == LOOPER_ID_USER)
      {
          __android_log_print(ANDROID_LOG_DEBUG, "gyro event count in loop = ", std::to_string(ASensorEventQueue_hasEvents(gSensors.gSensorQueue)).c_str(), 10);
          while(ASensorEventQueue_getEvents(gSensors.gSensorQueue, tempSensorEvent, 1) > 0) {
              lastLaData = laData;
              laData.x = tempSensorEvent->vector.x;
              laData.y = tempSensorEvent->vector.y;
              laData.z = tempSensorEvent->vector.z;
              __android_log_print(ANDROID_LOG_DEBUG, "LA :  ", (std::to_string(laData.x) + " " + std::to_string(laData.y) +
                                                                     " " + std::to_string(laData.z)).c_str(), 10);
          }
      }
    }
    // render if vulkan is ready
    if (IsVulkanReady()) {
      VulkanDrawFrame(app, currentFrame, isTouched, isFocused, touchPositions, &laData, &lastLaData);
      currentFrame = (currentFrame + 1) % MAX_IN_FLIGHT;
    }
    //usleep(100);
  } while (app->destroyRequested == 0);
}
