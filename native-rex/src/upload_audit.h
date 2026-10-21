#pragma once
// Exact staged-page shadow. Diagnostic mode observes; opt-in reuse returns changed ranges.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace ben10_profile {
class UploadAudit {
  std::string output_;
  uint32_t page_log2_;
  size_t capacity_;
  std::mutex mutex_;
  std::map<uint32_t,std::vector<uint8_t>> shadow_;
  uint64_t pages_=0,identical_=0,invalidated_=0,capacity_clears_=0;
  std::chrono::steady_clock::time_point previous_=std::chrono::steady_clock::now();
 public:
  UploadAudit(const char* output,uint32_t page_log2)
      : output_(output ? output : ""),page_log2_(page_log2),
        capacity_(std::max(size_t(1),(size_t(32)<<20)>>page_log2)) {}
  void Invalidate(uint32_t first,uint32_t last) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it=shadow_.lower_bound(first>>page_log2_);
    while(it!=shadow_.end() && it->first<=(last>>page_log2_)) {
      it=shadow_.erase(it);++invalidated_;
    }
  }
  size_t Observe(uint32_t address,const uint8_t* staged,size_t bytes,
                 std::vector<std::pair<size_t,size_t>>* changed=nullptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    const size_t page_size=size_t(1)<<page_log2_;
    size_t same=0;
    if(changed) changed->clear();
    for(size_t offset=0;offset+page_size<=bytes;offset+=page_size) {
      const uint32_t page=(address+uint32_t(offset))>>page_log2_;
      auto it=shadow_.find(page);
      ++pages_;
      if(it!=shadow_.end() && !std::memcmp(it->second.data(),staged+offset,page_size)) {
        ++same;++identical_;
      } else {
        if(changed) {
          if(!changed->empty() && changed->back().first+changed->back().second==offset)
            changed->back().second+=page_size;
          else changed->emplace_back(offset,page_size);
        }
        if(it==shadow_.end() && shadow_.size()>=capacity_) {
          shadow_.clear();++capacity_clears_;
        }
        auto& copy=shadow_[page];
        copy.assign(staged+offset,staged+offset+page_size);
      }
    }
    auto clock=std::chrono::steady_clock::now();
    if(!output_.empty() && clock-previous_>=std::chrono::seconds(5)) {
      auto now=std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
      std::ofstream log(output_,std::ios::app);
      log<<"{\"time_ms\":"<<now<<",\"interval_seconds\":"
         <<std::chrono::duration<double>(clock-previous_).count()
         <<",\"pages\":"<<pages_<<",\"identical_pages\":"<<identical_
         <<",\"bytes\":"<<(pages_<<page_log2_)
         <<",\"identical_bytes\":"<<(identical_<<page_log2_)
         <<",\"gpu_invalidated_entries\":"<<invalidated_
         <<",\"capacity_clears\":"<<capacity_clears_
         <<",\"cached_pages\":"<<shadow_.size()<<"}\n";
      pages_=identical_=invalidated_=capacity_clears_=0;previous_=clock;
    }
    return same;
  }
};
} // namespace ben10_profile
