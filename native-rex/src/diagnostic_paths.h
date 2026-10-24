#pragma once
#include <cstdlib>
#include <filesystem>

inline std::filesystem::path Ben10DiagnosticDirectory() {
  const char* value=std::getenv("BEN10_DIAGNOSTIC_ROOT");
  auto root=value?std::filesystem::path(value):std::filesystem::current_path()/"diagnostics";
  std::filesystem::create_directories(root);
  return root;
}
