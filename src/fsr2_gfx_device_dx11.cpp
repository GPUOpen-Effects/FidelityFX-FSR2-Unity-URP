
#include "fsr2_gfx_device_dx11.h"

#include "IUnityGraphicsD3D11.h"

#include "dx11/ffx_fsr2_dx11.h"

bool FSR2GFXDeviceDX11::InternalInitialize() {
  if (unity_interfaces_ != nullptr) {
    const auto unity_gfx_d3d11 = unity_interfaces_->Get<IUnityGraphicsD3D11>();
    if (unity_gfx_d3d11 != nullptr) {
      d3d11_device_ = unity_gfx_d3d11->GetDevice();
    }
    if (d3d11_device_ != nullptr) {
      d3d11_device_->GetImmediateContext(&immediate_context_);
    }
  }

  return d3d11_device_ != nullptr;
}

void FSR2GFXDeviceDX11::InternalDestroy() {
  immediate_context_->Release();
  d3d11_device_ = nullptr;
}

FfxDevice FSR2GFXDeviceDX11::GetFfxDevice() {
  return ffxGetDeviceDX11(d3d11_device_);
}

std::size_t FSR2GFXDeviceDX11::GetScratchMemorySize() {
  return ffxFsr2GetScratchMemorySizeDX11();
}

FfxErrorCode FSR2GFXDeviceDX11::GetInterface(
    void* const scratch_buffer, const size_t scratch_buffer_size,
    FfxFsr2Interface* const out_fsr2_interface) {
  return ffxFsr2GetInterfaceDX11(out_fsr2_interface, d3d11_device_,
                                 scratch_buffer, scratch_buffer_size);
}

FfxResource FSR2GFXDeviceDX11::GetResource(FfxFsr2Context* const context,
                                           void* const res_ptr,
                                           const wchar_t* const name,
                                           const FfxResourceStates state) {
  return ffxGetResourceDX11(
      context, reinterpret_cast<ID3D11Resource*>(res_ptr), name, state);
}

FfxCommandList FSR2GFXDeviceDX11::GetCommandList() {
  return reinterpret_cast<FfxCommandList>(immediate_context_);
}
