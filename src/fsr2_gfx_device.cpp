#include "fsr2_gfx_device.h"

bool FSR2GFXDevice::Initialize(IUnityInterfaces* const interfaces) {
  if (!initialized_) {
    unity_interfaces_ = interfaces;
    initialized_ = InternalInitialize();
  }
  return initialized_;
}

void FSR2GFXDevice::Destroy() {
  if (initialized_) {
    unity_interfaces_ = nullptr;
    initialized_ = false;
    InternalDestroy();
  }
}