#pragma once

#include <d3d11.h>

#include "fsr2_gfx_device.h"

class FSR2GFXDeviceDX11 : public FSR2GFXDevice {
 private:
  virtual bool InternalInitialize() override;
  virtual void InternalDestroy() override;
  virtual FfxDevice GetFfxDevice() override;
  virtual std::size_t GetScratchMemorySize() override;
  virtual FfxErrorCode GetInterface(
      void* const scratch_buffer, const size_t scratch_buffer_size,
      FfxFsr2Interface* const out_fsr2_interface);
  virtual FfxResource GetResource(FfxFsr2Context* const context,
                                  void* const res_ptr,
                                  const wchar_t* const name,
                                  const FfxResourceStates state) override;
  virtual FfxCommandList GetCommandList() override;

 private:
  ID3D11Device* d3d11_device_{nullptr};
  ID3D11DeviceContext* immediate_context_{nullptr};
};