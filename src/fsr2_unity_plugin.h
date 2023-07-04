#include <cstdint>
#include <vector>
#include <array>

#include "IUnityInterface.h"
#include "IUnityLog.h"
#include "IUnityGraphics.h"
#include "ffx_fsr2.h"

#include "fsr2_gfx_device.h"

class FSR2UnityPlugin {
 public:
  static IUnityInterfaces* unity_interfaces;
  static IUnityGraphics* unity_graphics;
  static IUnityLog* unity_log;

 public:
  static void UNITY_INTERFACE_API
  OnGraphicsDeviceEvent(UnityGfxDeviceEventType event_type);
};

class FFXFSR2 {
 public:
  struct InitializeParam {
    std::uint32_t flags;
    std::uint32_t displaySize[2];
  };

  struct GenReactiveParam {
    void* color_opaque_only;
    void* color_pre_upscale;
    void* out_reactive;
    std::uint32_t render_size[2];
    float scale;
    float cutoff_threshold;
    float binary_vaule;
    std::uint32_t flags;
  };

  struct ExecuteParam {
    void* color;
    void* depth;
    void* motion_vectors;
    void* reactive;
    void* transparencyAndComposition;
    void* output;
    float motion_vector_scale[2];
    std::uint32_t render_size[2];
    bool enable_sharpening;
    float sharpness;
    float frame_time_delta_in_sec;
    float camera_near;
    float camera_far;
    float camera_fov;
  };

 public:
  inline static FFXFSR2& Instance() {
    static FFXFSR2 instance;
    return instance;
  }

 public:
  FFXFSR2();

 public:
  void Initialize(IUnityInterfaces* interfaces);
  FfxErrorCode InitializeContext(const InitializeParam& init_param);
  void Destroy();
  void DestroyContext();
  std::array<float, 2> GetNewJitterOffset(const std::int32_t index,
                                          const std::uint32_t render_width,
                                          const std::uint32_t display_width);
  FfxErrorCode GenerateReactiveMask(
      const GenReactiveParam& gen_reactive_param);
  FfxErrorCode Execute(const ExecuteParam& exe_param);

  FSR2GFXDevice* GetFSR2GFXDevice() const;

 private:
  FfxFsr2Context context_{};
  bool context_created_{false};
  std::vector<char> scratch_buffer_;
  std::array<float, 2> jitter_offset_{};
  FSR2GFXDevice* fsr2_gfx_device_{nullptr};
};
