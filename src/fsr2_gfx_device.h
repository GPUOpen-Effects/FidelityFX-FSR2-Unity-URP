#pragma once

#include <cstddef>

#include "IUnityInterface.h"

#include "ffx_fsr2.h"

class FSR2GFXDevice {
 public:
  bool Initialize(IUnityInterfaces* const interfaces);
  void Destroy();
  virtual FfxDevice GetFfxDevice() = 0;
  virtual std::size_t GetScratchMemorySize() = 0;
  virtual FfxErrorCode GetInterface(
      void* const scratch_buffer, const size_t scratch_buffer_size,
      FfxFsr2Interface* const out_fsr2_interface) = 0;
  virtual FfxResource GetResource(
      FfxFsr2Context* const context, void* const res_ptr,
      const wchar_t* const name = nullptr,
      const FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ) = 0;
  virtual FfxCommandList GetCommandList() = 0;

 protected:
  bool initialized_{false};
  IUnityInterfaces* unity_interfaces_;

 private:
  virtual bool InternalInitialize() = 0;
  virtual void InternalDestroy() = 0;
};