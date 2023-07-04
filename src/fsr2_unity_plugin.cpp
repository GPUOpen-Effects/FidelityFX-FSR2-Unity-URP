#include <format>

#include "ffx_fsr2.h"
#ifdef FSR2_BACKEND_DX11
#include "fsr2_gfx_device_dx11.h"
#else
#error unsupported FSR2 backend
#endif

#include "fsr2_unity_plugin.h"

#include "fsr2_gfx_device.h"

#define FSR2_LOG(msg) UNITY_LOG(FSR2UnityPlugin::unity_log, msg);
#define FSR2_ERROR(msg) UNITY_LOG_ERROR(FSR2UnityPlugin::unity_log, msg);

IUnityInterfaces* FSR2UnityPlugin::unity_interfaces{nullptr};
IUnityGraphics* FSR2UnityPlugin::unity_graphics{nullptr};
IUnityLog* FSR2UnityPlugin::unity_log{nullptr};

void UNITY_INTERFACE_API
FSR2UnityPlugin::OnGraphicsDeviceEvent(UnityGfxDeviceEventType event_type) {
  switch (event_type) {
    case kUnityGfxDeviceEventInitialize:
      FFXFSR2::Instance().Initialize(unity_interfaces);
      break;
    case kUnityGfxDeviceEventShutdown:
      FFXFSR2::Instance().Destroy();
      break;
  }
}

FFXFSR2::FFXFSR2() {
  fsr2_gfx_device_ = new
#ifdef FSR2_BACKEND_DX11
      FSR2GFXDeviceDX11()
#else
#error unsupported FSR2 backend
#endif
      ;
}

void FFXFSR2::Initialize(IUnityInterfaces* interfaces) {
  fsr2_gfx_device_->Initialize(interfaces);
}

FfxErrorCode FFXFSR2::InitializeContext(const InitializeParam& init_param) {
  DestroyContext();
  FfxFsr2ContextDescription ctx_desc{
      .flags = init_param.flags,
      .maxRenderSize{.width = init_param.displaySize[0],
                     .height = init_param.displaySize[1]},
      .displaySize{.width = init_param.displaySize[0],
                   .height = init_param.displaySize[1]},
      .device = fsr2_gfx_device_->GetFfxDevice()};
  scratch_buffer_.resize(fsr2_gfx_device_->GetScratchMemorySize());
  fsr2_gfx_device_->GetInterface(
      scratch_buffer_.data(), scratch_buffer_.size(), &(ctx_desc.callbacks));

  const auto err_code = ffxFsr2ContextCreate(&context_, &ctx_desc);
  if (err_code == FFX_OK)
    context_created_ = true;
  else
    FSR2_ERROR("FFXFSR2 Initialize failed");
  return err_code;
}

void FFXFSR2::Destroy() {
  DestroyContext();
  fsr2_gfx_device_->Destroy();
}

void FFXFSR2::DestroyContext() {
  if (context_created_) {
    ffxFsr2ContextDestroy(&context_);
    context_created_ = false;
  }
}

FSR2GFXDevice* FFXFSR2::GetFSR2GFXDevice() const { return fsr2_gfx_device_; }

std::array<float, 2> FFXFSR2::GetNewJitterOffset(
    const std::int32_t index, const std::uint32_t render_width,
    const std::uint32_t display_width) {
  ffxFsr2GetJitterOffset(
      &(jitter_offset_[0]), &(jitter_offset_[1]), index,
      ffxFsr2GetJitterPhaseCount(render_width, display_width));
  return jitter_offset_;
}

FfxErrorCode FFXFSR2::GenerateReactiveMask(
    const GenReactiveParam& gen_reactive_param) {
  if (context_created_) {
    FfxFsr2GenerateReactiveDescription fsr2_gen_reactive_desc{
        .commandList = fsr2_gfx_device_->GetCommandList(),
        .colorOpaqueOnly = fsr2_gfx_device_->GetResource(
            &context_, gen_reactive_param.color_opaque_only),
        .colorPreUpscale = fsr2_gfx_device_->GetResource(
            &context_, gen_reactive_param.color_pre_upscale),
        .outReactive = fsr2_gfx_device_->GetResource(
            &context_, gen_reactive_param.out_reactive,
            L"FSR2_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS),
        .renderSize = {.width = gen_reactive_param.render_size[0],
                       .height = gen_reactive_param.render_size[1]},
        .scale = gen_reactive_param.scale,
        .cutoffThreshold = gen_reactive_param.cutoff_threshold,
        .binaryValue = gen_reactive_param.binary_vaule,
        .flags = gen_reactive_param.flags};
    const auto err =
        ffxFsr2ContextGenerateReactiveMask(&context_, &fsr2_gen_reactive_desc);
    if (err != FFX_OK) FSR2_ERROR("FFXFSR2 GenerateReactiveMask failed");
    return err;
  } else
    return !FFX_OK;
}

FfxErrorCode FFXFSR2::Execute(const ExecuteParam& exe_param) {
  if (context_created_) {
    FfxFsr2DispatchDescription dispatch_desc{
        .commandList = fsr2_gfx_device_->GetCommandList(),
        .color = fsr2_gfx_device_->GetResource(&context_, exe_param.color,
                                               L"FSR2_InputColor"),
        .depth = fsr2_gfx_device_->GetResource(&context_, exe_param.depth,
                                               L"FSR2_InputDepth"),
        .motionVectors = fsr2_gfx_device_->GetResource(
            &context_, exe_param.motion_vectors, L"FSR2_InputMotionVectors"),
        .reactive = fsr2_gfx_device_->GetResource(
            &context_, exe_param.reactive, L"FSR2_InputReactiveMap"),
        .transparencyAndComposition = fsr2_gfx_device_->GetResource(
            &context_, exe_param.transparencyAndComposition,
            L"FSR2_TransparencyAndCompositionMap"),
        .output = fsr2_gfx_device_->GetResource(
            &context_, exe_param.output, L"FSR2_OutputUpscaledColor",
            FFX_RESOURCE_STATE_UNORDERED_ACCESS),
        .jitterOffset{.x = jitter_offset_[0], .y = jitter_offset_[1]},
        .motionVectorScale{.x = exe_param.motion_vector_scale[0],
                           .y = exe_param.motion_vector_scale[1]},
        .renderSize{.width = exe_param.render_size[0],
                    .height = exe_param.render_size[1]},
        .enableSharpening = exe_param.enable_sharpening,
        .sharpness = exe_param.sharpness,
        .frameTimeDelta = exe_param.frame_time_delta_in_sec * 1000,
        .preExposure = 1.0f,
        .reset = false,
        .cameraNear = exe_param.camera_near,
        .cameraFar = exe_param.camera_far,
        .cameraFovAngleVertical = exe_param.camera_fov};

    const auto err = ffxFsr2ContextDispatch(&context_, &dispatch_desc);
    if (err != FFX_OK) FSR2_ERROR("FFXFSR2 Execute failed");
    return err;
  } else
    return !FFX_OK;
}

#ifdef __cplusplus
extern "C" {
#endif

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces) {
  FSR2UnityPlugin::unity_interfaces = unityInterfaces;
  FSR2UnityPlugin::unity_log = unityInterfaces->Get<IUnityLog>();
  const auto unity_graphics = unityInterfaces->Get<IUnityGraphics>();
  unity_graphics->RegisterDeviceEventCallback(
      &FSR2UnityPlugin::OnGraphicsDeviceEvent);
  FSR2UnityPlugin::OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload() {
  FSR2UnityPlugin::unity_graphics->UnregisterDeviceEventCallback(
      &FSR2UnityPlugin::OnGraphicsDeviceEvent);
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
FSR2Initialize(const FFXFSR2::InitializeParam* init_param) {
  FFXFSR2::Instance().InitializeContext(*init_param);
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
FSR2GetProjectionMatrixJitterOffset(const std::uint32_t render_width,
                                    const std::uint32_t render_height,
                                    const std::uint32_t display_width,
                                    float* out_jitter_offset) {
  static std::int32_t index = 0;
  const auto& jitter_offset = FFXFSR2::Instance().GetNewJitterOffset(
      index++, render_width, display_width);
  if (out_jitter_offset != nullptr) {
    out_jitter_offset[0] = 2.0f * jitter_offset[0] / render_width;
    out_jitter_offset[1] = -2.0f * jitter_offset[1] / render_height;
  }
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
FSR2GenReactiveMask(const FFXFSR2::GenReactiveParam* gen_reactive_param) {
  FFXFSR2::Instance().GenerateReactiveMask(*gen_reactive_param);
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
FSR2Execute(const FFXFSR2::ExecuteParam* exe_param) {
  FFXFSR2::Instance().Execute(*exe_param);
}

#ifdef __cplusplus
}
#endif