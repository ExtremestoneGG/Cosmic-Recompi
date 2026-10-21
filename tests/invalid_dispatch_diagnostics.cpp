#include "../native-rex/src/invalid_dispatch_diagnostics.h"
#include <cassert>
#include <string>
#include <vector>
#include <cstdio>
union Register { uint64_t u64;uint32_t u32; };
struct Context { uint32_t last_indirect_target=0;uint64_t lr=0;Register ctr{},r1{},r3{},r4{},r11{},r12{}; };
int main() {
  Context ctx;ctx.lr=0x82123458;ctx.r1.u32=128;
  uint8_t memory[4096]{};
  memory[128]=0x82;memory[129]=0x10;memory[130]=0xbb;memory[131]=0x88;
  std::vector<std::string> rows;
  auto report=[&](const char* text){rows.emplace_back(text);};
  ben10_diagnostics::InvalidDispatch(ctx,memory,report);
  assert(rows[0].find("target=00000000")!=std::string::npos);
  assert(rows[0].find("LR=0000000082123458")!=std::string::npos);
  bool guest=false,native=false;
  for(auto& row:rows){guest|=row.find("+000: 8210BB88")!=std::string::npos;native|=row.find("native frame")!=std::string::npos;}
  assert(guest && native);
  assert(ctx.lr==0x82123458 && ctx.r1.u32==128 && ctx.last_indirect_target==0);
  rows.clear();ctx.r1.u32=0xfffffffe;
  ben10_diagnostics::InvalidDispatch(ctx,memory,report);
  assert(rows[1].find("unavailable")!=std::string::npos);
  rows.clear();ctx.r1.u32=0;
  ben10_diagnostics::InvalidDispatch(ctx,reinterpret_cast<uint8_t*>(1),report);
  assert(rows[1].find("unavailable")!=std::string::npos);
  rows.clear();ben10_diagnostics::InvalidDispatch(ctx,nullptr,report);
  assert(rows[1].find("unavailable")!=std::string::npos);
  std::puts("{\"passed\":true,\"context_logged\":true,\"safe_guest_stack_read\":true,\"native_stack\":true,\"invalid_memory_safe\":true,\"context_preserved\":true}");
}
