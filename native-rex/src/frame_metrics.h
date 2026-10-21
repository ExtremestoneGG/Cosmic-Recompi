#pragma once
#include <cstdint>
#include <array>
#include <cstddef>
// Counts the original game's VdSwap calls without altering its PPC state.
extern "C" void Ben10FrameSubmitted();
uint64_t Ben10SubmittedFrames();
struct Ben10FrameStats {
  std::array<float,240> frame_times_ms{};
  size_t count=0;
  double fps=0,latest_ms=0,p95_ms=0;
  uint64_t submitted_frames=0;
};
Ben10FrameStats Ben10ReadFrameStats();
