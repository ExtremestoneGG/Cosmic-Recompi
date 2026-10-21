#pragma once
#include <rex/input/input_driver.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstring>
#include <mutex>

// Optional reproducible benchmark input. Disabled in normal interactive runs.
// Feeds the game's input system directly; never sends Windows key events.
namespace rex::input {
class Ben10ReplayInput final : public InputDriver {
 public:
  explicit Ben10ReplayInput(const char* file) : InputDriver(nullptr,0) {
    if (const char* command=std::getenv("BEN10_REPLAY_COMMAND_PATH")) command_file_=command;
    std::ifstream input(file);std::string line;
    while (std::getline(input,line)) {
      if (line.empty() || line[0]=='#') continue;
      for (char& c:line) if (c==',') c=' ';
      std::istringstream row(line);Step step{};
      if (row>>step.begin>>step.end>>step.buttons>>step.ly) {
        row>>step.rx>>step.ry>>step.lx;
        if (step.ly>=-32768 && step.ly<=32767 && step.rx>=-32768 && step.rx<=32767 && step.ry>=-32768 && step.ry<=32767 && step.lx>=-32768 && step.lx<=32767)
          steps_.push_back(step);
      }
    }
  }
  X_STATUS Setup() override { start_=std::chrono::steady_clock::now();return X_STATUS_SUCCESS; }
  void EnumerateDevices(std::vector<rex::input::DeviceInfo>& out) override {
    rex::input::DeviceInfo info;info.id=id_;info.name="Ben10 benchmark replay";
    info.synthetic=true;out.push_back(info);
  }
  X_RESULT GetDeviceState(rex::input::DeviceId id,rex::input::X_INPUT_STATE* state) override {
    if (id!=id_) return X_ERROR_DEVICE_NOT_CONNECTED;
    if (!state) return X_ERROR_SUCCESS;
    std::lock_guard guard(command_mutex_);
    std::memset(state,0,sizeof(*state));
    if(!is_active())return X_ERROR_SUCCESS;
    const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start_).count();
    uint16_t buttons=0;int16_t ly=0,rx=0,ry=0,lx=0;
    for (const auto& step:steps_) if (elapsed>=step.begin && elapsed<step.end) {
      buttons|=step.buttons;ly=static_cast<int16_t>(step.ly);rx=static_cast<int16_t>(step.rx);ry=static_cast<int16_t>(step.ry);lx=static_cast<int16_t>(step.lx);
    }
    const auto now=std::chrono::steady_clock::now();
    const int64_t wall_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (!command_file_.empty() && now>=next_command_read_) {
      next_command_read_=now+std::chrono::milliseconds(50);
      std::ifstream command(command_file_);unsigned bits=0;int value=0,rx_value=0,ry_value=0,lx_value=0;int64_t expiry=0;
      if (command>>expiry>>bits>>value && bits<=0xffff && value>=-32768 && value<=32767) {
        command>>rx_value>>ry_value>>lx_value;
        if (rx_value>=-32768 && rx_value<=32767 && ry_value>=-32768 && ry_value<=32767 && lx_value>=-32768 && lx_value<=32767) {
          command_expires_=expiry;command_buttons_=static_cast<uint16_t>(bits);command_ly_=static_cast<int16_t>(value);
          command_rx_=static_cast<int16_t>(rx_value);command_ry_=static_cast<int16_t>(ry_value);command_lx_=static_cast<int16_t>(lx_value);
        }
      }
    }
    if (wall_ms<command_expires_) { buttons|=command_buttons_;ly=command_ly_;rx=command_rx_;ry=command_ry_;lx=command_lx_; }
    const uint64_t packet=(uint64_t(buttons)<<48)|(uint64_t(uint16_t(ly))<<32)|(uint64_t(uint16_t(rx))<<16)|uint16_t(ry);
    if (packet!=previous_state_ || lx!=previous_lx_) { ++packet_number_;previous_state_=packet;previous_lx_=lx; }
    state->packet_number=packet_number_;state->gamepad.buttons=buttons;state->gamepad.thumb_ly=ly;state->gamepad.thumb_rx=rx;state->gamepad.thumb_ry=ry;state->gamepad.thumb_lx=lx;
    return X_ERROR_SUCCESS;
  }
  X_RESULT GetDeviceCapabilities(rex::input::DeviceId id,uint32_t,rex::input::X_INPUT_CAPABILITIES* caps) override {
    if (id!=id_) return X_ERROR_DEVICE_NOT_CONNECTED;
    if (caps) { std::memset(caps,0,sizeof(*caps));caps->type=1;caps->sub_type=1;caps->gamepad.buttons=0xffff; }
    return X_ERROR_SUCCESS;
  }
  X_RESULT SetDeviceVibration(rex::input::DeviceId id,rex::input::X_INPUT_VIBRATION*) override {
    return id==id_?X_ERROR_SUCCESS:X_ERROR_DEVICE_NOT_CONNECTED;
  }
  X_RESULT GetDeviceKeystroke(rex::input::DeviceId id,uint32_t,rex::input::X_INPUT_KEYSTROKE*) override {
    return id==id_?X_ERROR_EMPTY:X_ERROR_DEVICE_NOT_CONNECTED;
  }
 private:
  struct Step { int64_t begin,end;uint16_t buttons;int ly,rx,ry,lx; };
  std::vector<Step> steps_;
  std::chrono::steady_clock::time_point start_;
  std::chrono::steady_clock::time_point next_command_read_;
  std::mutex command_mutex_;
  std::string command_file_;
  int64_t command_expires_=0;
  uint16_t command_buttons_=0;
  int16_t command_ly_=0;
  int16_t command_rx_=0,command_ry_=0;
  int16_t command_lx_=0,previous_lx_=0;
  uint64_t previous_state_=0;
  uint32_t packet_number_=0x40000000;
  static constexpr auto id_=static_cast<rex::input::DeviceId>(0x42454e313052504c);
};
}  // namespace rex::input
