// ben10 - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <rex/rex_app.h>
#include <rex/runtime.h>
#include <rex/system/interfaces/graphics.h>
#include <rex/system/gpu_plugin.h>
#include <rex/system/function_dispatcher.h>
#include <rex/ui/presenter.h>
#include <fstream>
#include <thread>
#include <Windows.h>
#include "frame_metrics.h"
#include "gpu_frame_capture.h"
#include "replay_input.h"
#include "performance_overlay.h"
#include "pc_menu_bridge.h"
#include "game_fault_probe.h"
#include "diagnostic_paths.h"
#include <rex/input/input_system.h>
#include <rex/system/achievement_manager.h>

class Ben10App : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  void OnConfigureFonts(ImFontAtlas* atlas) override {
    if(!ben10_pc_menu::enabled())return;
    char windows[MAX_PATH]{};
    if(GetWindowsDirectoryA(windows,MAX_PATH)) {
      const auto font=std::filesystem::path(windows)/"Fonts/segoeui.ttf";
      if(std::filesystem::exists(font))pc_video_font_=atlas->AddFontFromFileTTF(font.string().c_str(),19);
    }
  }

  void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {
    // Profiling/replay tools keep their own measurements; ordinary play gets
    // a passive overlay driven by the original game's swap counter.
    if(!std::getenv("BEN10_REPLAY_INPUT_PATH") && !std::getenv("BEN10_HIDE_PERFORMANCE_OVERLAY"))
      performance_overlay_=std::make_unique<Ben10PerformanceOverlay>(drawer);
  }

  void OnConfigurePaths(rex::PathConfig& paths) override {
    wchar_t portable_root[32768] = {};
    const DWORD portable_length = GetEnvironmentVariableW(
        L"BEN10_PORTABLE_ROOT", portable_root, DWORD(std::size(portable_root)));
    if (portable_length) {
      if (portable_length >= std::size(portable_root))
        throw std::runtime_error("Caminho da instalacao muito longo");
      const std::filesystem::path portable(portable_root);
      if (!portable.is_absolute())
        throw std::runtime_error("A instalacao exige um caminho absoluto");
      paths.game_data_root = portable / "game";
      paths.user_data_root = portable / "user";
      paths.cache_root = portable / "cache";
      paths.metadata_root = portable / "metadata";
      paths.config_path = portable / "runtime.toml";
      paths.update_data_root.clear();
      return;
    }
    const auto root=std::filesystem::current_path();
    paths.game_data_root=root/"originals";
    paths.user_data_root=root/(std::getenv("BEN10_REPLAY_INPUT_PATH")?"data/rex-benchmark-user":"data/rex-user");
    paths.cache_root=root/(std::getenv("BEN10_FORMATS_CACHE")?
        "data/rex-formats-cache":(std::getenv("BEN10_OPTIMIZED_CACHE")?
        "data/rex-optimized-cache":(std::getenv("BEN10_DIAGNOSTIC_CACHE")?
        "data/rex-profile-cache":"data/rex-cache")));
    paths.update_data_root.clear();
    paths.metadata_root=root/"native-rex/metadata";
    paths.config_path=root/"config/rex-runtime.toml";
  }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    Ben10InitializeFrameCapture();
    if (const char* mode=std::getenv("BEN10_WINDOW_MODE")) {
      const std::string_view value(mode);
      if (value!="windowed" && value!="borderless")
        throw std::runtime_error("Unknown presentation diagnostic mode");
      const char* fullscreen=value=="borderless" ? "true" : "false";
      if (!rex::cvar::SetFlagByName("fullscreen",fullscreen) ||
          rex::cvar::GetFlagByName("fullscreen")!=fullscreen)
        throw std::runtime_error("Presentation diagnostic unavailable");
      REXLOG_INFO("Ben10 presentation diagnostic requested: {}",mode);
    }
    config.gpu_plugin="xenos";
    REXLOG_INFO("Ben10 effective cache root: {}",cache_root().string());
    if (const char* resolution=std::getenv("BEN10_RESOLUTION")) {
      if (!rex::cvar::SetFlagByName("resolution",resolution) ||
          rex::cvar::GetFlagByName("resolution")!=resolution)
        throw std::runtime_error("Nao foi possivel aplicar o modo de video solicitado");
      REXLOG_INFO("Ben10 video mode requested: {}",resolution);
    }
    if (const char* backend=std::getenv("BEN10_GRAPHICS_BACKEND")) {
      config.graphics=rex::system::LoadGpuPlugin("xenos",backend);
      if (!config.graphics) throw std::runtime_error(std::string("Backend grafico indisponivel: ")+backend);
      if (std::getenv("BEN10_ESTIMATE_DRAW_EXTENT")) {
        if(std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("execute_unclipped_draw_vs_on_cpu","true") ||
            !rex::cvar::SetFlagByName("execute_unclipped_draw_vs_on_cpu_with_scissor","false") ||
            rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu")!="true" ||
            rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu_with_scissor")!="false")
          throw std::runtime_error("Existing SDK draw extent estimator unavailable");
      }
      REXLOG_INFO("Ben10 SDK draw extent: CPU={}, with-scissor={}",
          rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu"),
          rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu_with_scissor"));
      if (std::getenv("BEN10_HOST_NO_TEARING")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_allow_variable_refresh_rate_and_tearing","false") ||
            rex::cvar::GetFlagByName("d3d12_allow_variable_refresh_rate_and_tearing")!="false")
          throw std::runtime_error("Host no-tearing setting unavailable");
        REXLOG_INFO("Ben10 host presentation: allow VRR/tearing={}",
            rex::cvar::GetFlagByName("d3d12_allow_variable_refresh_rate_and_tearing"));
      }
      if (std::getenv("BEN10_DIRECT_RESOLVE")) {
        if(std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_exact_direct_resolve","true") ||
            rex::cvar::GetFlagByName("d3d12_exact_direct_resolve")!="true")
          throw std::runtime_error("Exact direct resolve diagnostic unavailable");
        REXLOG_INFO("Ben10 exact RGBA8 1x direct resolve enabled; generic fallback retained");
      }
      if (std::getenv("BEN10_RTV_ZERO_LOD")) {
        if (std::string_view(backend)!="d3d12" || !std::getenv("BEN10_RTV_UNSIGNED_TEXTURES") ||
            !rex::cvar::SetFlagByName("d3d12_rtv_zero_lod_bias","true") ||
            rex::cvar::GetFlagByName("d3d12_rtv_zero_lod_bias")!="true")
          throw std::runtime_error("RTV zero LOD diagnostic unavailable");
        REXLOG_INFO("Ben10 RTV zero fetch LOD specialization enabled; instruction and register LOD preserved");
      }
      if (std::getenv("BEN10_RTV_UNSIGNED_TEXTURES")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rtv_unsigned_textures","true") ||
            rex::cvar::GetFlagByName("d3d12_rtv_unsigned_textures")!="true")
          throw std::runtime_error("RTV unsigned textures diagnostic unavailable");
        REXLOG_INFO("Ben10 RTV unsigned texture specialization enabled; bindings verified per draw");
      }
      if (std::getenv("BEN10_RTV_INITIAL_DEPTH_ZERO")) {
        if (!rex::cvar::SetFlagByName("d3d12_rtv_initial_depth_zero","true") ||
            rex::cvar::GetFlagByName("d3d12_rtv_initial_depth_zero")!="true")
          throw std::runtime_error("Initial depth zero diagnostic unavailable");
        REXLOG_INFO("Ben10 initial unowned D24S8 zero clear enabled");
      }
      if (std::getenv("BEN10_ROV_RAW_EDRAM")) {
        if (std::string_view(backend)!="d3d12" ||
            !std::getenv("BEN10_DIAGNOSTIC_CACHE") ||
            !rex::cvar::SetFlagByName("d3d12_rov_raw_edram","true") ||
            rex::cvar::GetFlagByName("d3d12_rov_raw_edram")!="true")
          throw std::runtime_error("ROV raw EDRAM diagnostic unavailable");
        REXLOG_INFO("Ben10 ROV raw EDRAM: enabled; byte addresses, scalar words and rasterizer order preserved");
      }
      if (std::getenv("BEN10_DEPTH_TRANSFER_ALWAYS")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("depth_transfer_not_equal_test","false") ||
            rex::cvar::GetFlagByName("depth_transfer_not_equal_test")!="false")
          throw std::runtime_error("Depth transfer diagnostic unavailable");
        REXLOG_INFO("Ben10 depth transfer diagnostic: ALWAYS instead of NOTEQUAL");
      }
      if (const char* value=std::getenv("BEN10_INTEL_NATIVE_STENCIL")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("native_stencil_value_output_d3d12_intel",value) ||
            rex::cvar::GetFlagByName("native_stencil_value_output_d3d12_intel")!=value)
          throw std::runtime_error("Native Intel stencil diagnostic unavailable");
        REXLOG_INFO("Ben10 native Intel stencil output: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_ALPHA_ONE")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_alpha_one_fastpath",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_alpha_one_fastpath")!=value)
          throw std::runtime_error("Caminho alpha-one ROV indisponivel");
        REXLOG_INFO("Ben10 exact alpha-one ROV fast path: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_DEPTH_CONTROL")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_depth_control",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_depth_control")!=value)
          throw std::runtime_error("Controle de profundidade ROV indisponivel");
        REXLOG_INFO("Ben10 exact ROV depth control specialization: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_DEPTH")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_depth",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_depth")!=value)
          throw std::runtime_error("Especializacao de profundidade ROV indisponivel");
        REXLOG_INFO("Ben10 exact ROV depth specialization: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_FORMATS")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_formats",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_formats")!=value)
          throw std::runtime_error("Especializacao de formatos ROV indisponivel");
        REXLOG_INFO("Ben10 exact ROV color format specialization: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_SAMPLES")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_samples",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_samples")!=value)
          throw std::runtime_error("Especializacao de amostras ROV indisponivel");
        REXLOG_INFO("Ben10 exact guest MSAA specialization: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_BLEND")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_blend",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_blend")!=value)
          throw std::runtime_error("Especializacao ROV indisponivel neste plugin");
        REXLOG_INFO("Ben10 ROV identity blend specialization: {}",value);
      }
      if (const char* value=std::getenv("BEN10_ROV_STATIC_COMMON_BLEND")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_rov_static_common_blends",value) ||
            rex::cvar::GetFlagByName("d3d12_rov_static_common_blends")!=value)
          throw std::runtime_error("Especializacao comum ROV indisponivel");
        REXLOG_INFO("Ben10 ROV exact common blend specialization: {}",value);
      }
      if (const char* dump=std::getenv("BEN10_SHADER_DUMP")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("dump_shaders",dump) ||
            rex::cvar::GetFlagByName("dump_shaders")!=dump ||
            !rex::cvar::SetFlagByName("d3d12_dxbc_disasm","true"))
          throw std::runtime_error("Dump de shaders indisponivel");
        REXLOG_INFO("Ben10 diagnostic shader dump: {}",dump);
      }
      if (const char* bounds=std::getenv("BEN10_EDRAM_BOUNDS")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("execute_unclipped_draw_vs_on_cpu",bounds) ||
            rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu")!=bounds)
          throw std::runtime_error("Estimativa de limites EDRAM indisponivel");
        REXLOG_INFO("Ben10 EDRAM vertex bounds experiment: {}",
                    rex::cvar::GetFlagByName("execute_unclipped_draw_vs_on_cpu"));
      }
      if (const char* experiment=std::getenv("BEN10_INTEL_VIEWPORT")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_intel_viewport_workaround",experiment) ||
            rex::cvar::GetFlagByName("d3d12_intel_viewport_workaround")!=experiment)
          throw std::runtime_error("Experimento de viewport indisponivel neste plugin");
        REXLOG_INFO("Ben10 Intel viewport bounds experiment: {}",experiment);
      }
      if (const char* experiment=std::getenv("BEN10_INTEL_STENCIL")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("d3d12_intel_stencil_always_workaround",experiment) ||
            rex::cvar::GetFlagByName("d3d12_intel_stencil_always_workaround")!=experiment)
          throw std::runtime_error("Experimento de stencil indisponivel neste plugin");
        REXLOG_INFO("Ben10 Intel stencil ALWAYS experiment: {}",experiment);
      }
      if (const char* experiment=std::getenv("BEN10_INTEL_SHADER_SWITCH")) {
        if (std::string_view(backend)!="d3d12" ||
            !rex::cvar::SetFlagByName("dxbc_allow_switch_on_intel",experiment) ||
            rex::cvar::GetFlagByName("dxbc_allow_switch_on_intel")!=experiment)
          throw std::runtime_error("Experimento de shaders Intel indisponivel neste plugin");
        REXLOG_INFO("Ben10 Intel shader switch experiment: {}",
                    rex::cvar::GetFlagByName("dxbc_allow_switch_on_intel"));
      }
      if (const char* path=std::getenv("BEN10_RENDER_TARGET_PATH")) {
        if (!rex::cvar::SetFlagByName("render_target_path_d3d12",path) ||
            rex::cvar::GetFlagByName("render_target_path_d3d12")!=path)
          throw std::runtime_error("Nao foi possivel aplicar o render target solicitado");
      }
      REXLOG_INFO("Ben10 GPU selection: backend={}, render_target_path_d3d12={}, invalid_fetch={}",
                  backend,rex::cvar::GetFlagByName("render_target_path_d3d12"),
                  rex::cvar::GetFlagByName("gpu_allow_invalid_fetch_constants"));
      if (std::string_view(backend)=="vulkan")
        REXLOG_INFO("Ben10 Vulkan: primary_submit={}, dynamic_rendering={}, gamma_unorm16={}",
                    rex::cvar::GetFlagByName("vulkan_submit_on_primary_buffer_end"),
                    rex::cvar::GetFlagByName("vulkan_dynamic_rendering"),
                    rex::cvar::GetFlagByName("gamma_render_target_as_unorm16"));
    }
    if (const char* replay=std::getenv("BEN10_REPLAY_INPUT_PATH")) {
      const std::string path=replay;
      config.input_factory=[path](bool tool_mode) {
        auto input=rex::input::CreateDefaultInputSystem(tool_mode);
        auto driver=std::make_unique<rex::input::Ben10ReplayInput>(path.c_str());
        driver->Setup();input->AddDriver(std::move(driver));
        return input;
      };
    }
  }

  void OnPostSetup() override {
    ben10_fault_probe::Install();
    for(uint32_t address : {0x8210BB88u,0x8229A9F8u,0x8229AA20u,0x8229AA48u,
                            0x8229AA60u,0x8229AA78u,0x8229AA90u,0x8229AAA8u}) {
      if(!runtime()->function_dispatcher()->GetFunction(address))
        throw std::runtime_error("Required gameplay callback is not registered");
    }
    REXLOG_INFO("Reviewed gameplay callbacks registered, including 8210BB88 and 8229AA60");
    if(std::getenv("BEN10_CALLBACK_SELF_TEST")) {
      auto* memory=runtime()->memory();
      const uint32_t object=memory->SystemHeapAlloc(1408);
      if(!object)throw std::runtime_error("Callback self-test allocation failed");
      auto* base=memory->virtual_membase();
      std::memset(base+object,0x5A,1408);
      PPCContext context{};
      context.r3.u64=0xDEADBEEF;context.r4.u64=object;
      context.lr=0x82345678;context.ctr.u64=0x82100000;
      runtime()->function_dispatcher()->GetFunction(0x8210BB88)(context,base);
      bool valid=context.r3.u64==object && context.r4.u64==0 &&
                 context.lr==0x82345678 && context.ctr.u64==0x82100000;
      for(uint32_t i=0;i<1408;++i)
        valid &= base[object+i]==(i>=1320 && i<1324 ? 0 : 0x5A);
      memory->SystemHeapFree(object);
      if(!valid)throw std::runtime_error("Native gameplay callback self-test failed");
      REXLOG_INFO("Native callback 8210BB88 self-test passed: registered dispatch, real callee, isolated memory write, LR/CTR preserved");
    }
    // Presentation/dialogs are created before Runtime. Bind only after setup.
    if(ben10_pc_menu::enabled()) {
      ben10_pc_menu::runtime=runtime();
      ben10_pc_menu::window=window();
      auto* input=static_cast<rex::input::InputSystem*>(runtime()->input_system());
      input->SetActiveCallback([this] {
        return !imgui_drawer()->GetIO().WantCaptureMouse;
      });
    }
    if (const char* hidden=std::getenv("BEN10_TEST_HIDDEN")) {
      if (std::string_view(hidden)=="1" && window()) {
        if (auto* handle=window()->GetNativeWindowHandle())
          ShowWindow(static_cast<HWND>(handle),SW_HIDE);
      }
    }
    if (std::getenv("BEN10_CAPTURE_OUTPUT")) {
      capture_worker_=std::jthread([this](std::stop_token stop) {
        auto previous_time=std::chrono::steady_clock::now();
        const auto start_time=previous_time;
        std::string tuning_command;
        std::string notification_command;
        bool gpu_frame_captured=false;
        uint64_t previous_frames=Ben10SubmittedFrames();
        for (int tick=0; !stop.stop_requested(); ++tick) {
          std::this_thread::sleep_for(std::chrono::milliseconds(250));
          if (const char* stop_file=std::getenv("BEN10_STOP_FILE")) {
            if (std::filesystem::exists(stop_file)) {
              app_context().CallInUIThreadDeferred([this] { if (window()) window()->RequestClose(); });
              return;
            }
          }
          if (!gpu_frame_captured && Ben10FrameCaptureStorage()) {
            if (const char* request=std::getenv("BEN10_RENDERDOC_REQUEST")) {
              if (std::filesystem::exists(request)) {
                std::filesystem::remove(request);
                gpu_frame_captured=Ben10TriggerFrameCapture();
                REXLOG_INFO("Ben10 RenderDoc: explicit interval capture success={}",gpu_frame_captured);
              }
            }
          }
          if (tick%4==3) {
            // Cosmetic smoke test only: show an existing catalog entry through
            // the real notification callback, without unlocking achievements.
            if (const char* path=std::getenv("BEN10_UI_NOTIFICATION_SMOKE")) {
              std::ifstream request(path);
              std::string command;
              if (request>>command && command!=notification_command) {
                const auto catalog=achievements().ListAchievements();
                const uint32_t id=catalog.empty()?0:catalog.front().id;
                const bool before=!catalog.empty() && achievements().IsUnlocked(id);
                const bool shown=!catalog.empty() && achievements().ShowAchievementNotification(id);
                const bool after=!catalog.empty() && achievements().IsUnlocked(id);
                REXLOG_INFO("Ben10 notification smoke {}: catalog={}, shown={}, unlocked-before={}, unlocked-after={}",
                    command,catalog.size(),shown,before,after);
                notification_command=command;
              }
            }
            // This SDK switch is explicitly hot-reloadable. Keep experimental
            // changes inside opt-in benchmarks; normal play never reads this.
            if (std::getenv("BEN10_REPLAY_INPUT_PATH")) {
              const char* backend=std::getenv("BEN10_GRAPHICS_BACKEND");
              if (backend && (std::string_view(backend)=="vulkan" || std::string_view(backend)=="d3d12")) {
                const char* flag=std::string_view(backend)=="vulkan"?
                    "vulkan_submit_on_primary_buffer_end":"d3d12_submit_on_primary_buffer_end";
                std::ifstream command(Ben10DiagnosticDirectory()/"benchmark-gpu-command.txt");
                std::string id;int enabled=-1;
                if (command>>id>>enabled && id!=tuning_command && (enabled==0 || enabled==1)) {
                  const char* value=enabled?"true":"false";
                  const bool applied=rex::cvar::SetFlagByName(flag,value);
                  REXLOG_INFO("Ben10 benchmark GPU command {}: primary_submit={} applied={}",
                              id,rex::cvar::GetFlagByName(flag),applied);
                  tuning_command=id;
                }
              }
            }
            const auto now=std::chrono::steady_clock::now();
            const uint64_t frames=Ben10SubmittedFrames();
            const double elapsed=std::chrono::duration<double>(now-previous_time).count();
            const auto metrics_tmp=Ben10DiagnosticDirectory()/"rex-frame-metrics.tmp";
            const auto metrics_final=Ben10DiagnosticDirectory()/"rex-frame-metrics.json";
            std::ofstream metrics(metrics_tmp);
            metrics << "{\"submitted_frames\":" << frames << ",\"elapsed_us\":"
                    << std::chrono::duration_cast<std::chrono::microseconds>(now-start_time).count()
                    << ",\"fps\":" << (frames-previous_frames)/elapsed
                    << ",\"interval_seconds\":" << elapsed << ",\"source\":\"VdSwap submissions per second; not monitor presents\"}";
            metrics.close();
            MoveFileExW(metrics_tmp.c_str(),metrics_final.c_str(),MOVEFILE_REPLACE_EXISTING);
            previous_frames=frames;previous_time=now;
          }
          // GPU readbacks are for explicit visual tests, never ordinary play.
          // Lightweight frame counters above remain available to the dashboard.
          bool capture_requested=false;
          if (const char* request=std::getenv("BEN10_CAPTURE_REQUEST")) {
            if (std::filesystem::exists(request)) {
              std::filesystem::remove(request);capture_requested=true;
            }
          }
          if (!capture_requested && (tick%40!=39 || !std::getenv("BEN10_CAPTURE_FRAMES"))) continue;
          auto* graphics=runtime()->graphics_system();
          if (!graphics || !graphics->presenter()) continue;
          rex::ui::RawImage image;
          bool captured=graphics->presenter()->CaptureGuestOutput(image);
          std::ofstream status(Ben10DiagnosticDirectory()/"rex-output.json");
          status << "{\"captured\":" << (captured?"true":"false")
                 << ",\"width\":" << image.width << ",\"height\":" << image.height << "}";
          if (!captured || !image.width || !image.height) continue;
          std::ofstream output(Ben10DiagnosticDirectory()/"rex-output.ppm",std::ios::binary);
          output << "P6\n" << image.width << ' ' << image.height << "\n255\n";
          for (uint32_t y=0;y<image.height;++y)
            for (uint32_t x=0;x<image.width;++x)
              output.write(reinterpret_cast<const char*>(image.data.data()+y*image.stride+x*4),3);
        }
      });
    }
  }

  void OnShutdown() override {
    ben10_pc_video::visible=false;
    ben10_pc_video::release_guard=false;
    if(ben10_pc_video::closed_event)SetEvent(ben10_pc_video::closed_event);
    pc_video_menu_.reset();
    ben10_pc_menu::window=nullptr;
    ben10_pc_menu::runtime=nullptr;
    performance_overlay_.reset();
    if (capture_worker_.joinable()) { capture_worker_.request_stop();capture_worker_.join(); }
  }

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<Ben10App>(new Ben10App(ctx, "ben10",
        PPCImageConfig));
  }

  // Override virtual hooks for customization:
  // void OnPostInitLogging() override {}
  // void OnPreSetup(rex::RuntimeConfig& config) override {}
  // void OnLoadXexImage(std::string& xex_image) override {}
  // void OnPostLoadXexImage() override {}
  // void OnPostSetup() override {}
  // void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {}
  // std::unique_ptr<rex::ui::ImGuiDialog> CreateAchievementsOverlay() override;
  // std::unique_ptr<rex::ui::AchievementNotificationDialog>
  // CreateAchievementNotificationDialog() override;
  // void OnShutdown() override {}
  // void OnConfigurePaths(rex::PathConfig& paths) override {}
 private:
  ImFont* pc_video_font_=nullptr;
  std::unique_ptr<ben10_pc_video::Dialog> pc_video_menu_;
  std::unique_ptr<Ben10PerformanceOverlay> performance_overlay_;
  std::jthread capture_worker_;
};
