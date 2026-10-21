#pragma once
// Opt-in observation only: no copied resource, changed state or cache validity.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
namespace ben10_profile {
struct ResolvePrefillAudit {
  const char* output = std::getenv("BEN10_RESOLVE_PREFILL_AUDIT");
  using Key = std::array<uint32_t, 20>;
  std::map<Key, uint64_t> counts;
  std::chrono::steady_clock::time_point previous = std::chrono::steady_clock::now();
  void Record(const Key& key) {
    if (!output) return;
    ++counts[key];
    const auto now = std::chrono::steady_clock::now();
    if (now - previous < std::chrono::seconds(5)) return;
    static constexpr const char* names[] = {"base", "resolve_width", "resolve_height",
      "resolve_endian", "swap_rb", "dest_coordinates", "clear_flags", "existing",
      "texture_width", "texture_height", "texture_pitch_bytes", "texture_endian",
      "texture_format", "texture_dimension", "texture_layers", "texture_mip_max",
      "texture_tiled", "texture_scaled", "texture_signed_separate", "texture_outdated"};
    const auto unix_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream log(output, std::ios::app);
    log << "{\"time_ms\":" << unix_ms << ",\"interval_seconds\":"
        << std::chrono::duration<double>(now-previous).count() << ",\"entries\":[";
    bool first = true;
    for (const auto& pair : counts) {
      log << (first ? "" : ",") << "{\"count\":" << pair.second;
      for (size_t i = 0; i < pair.first.size(); ++i)
        log << ",\"" << names[i] << "\":" << pair.first[i];
      log << "}"; first = false;
    }
    log << "]}\n";
    counts.clear(); previous = now;
  }
};
inline thread_local ResolvePrefillAudit resolve_prefill_audit;
}
