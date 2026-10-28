#pragma once
#include <cstdint>
namespace ben10_rgba {
// Compose guest/host component selection with a red/blue backing-byte swap.
// Each selector is 3 bits; constants and the D3D12 always-set marker survive.
constexpr uint32_t SwapInputsRB(uint32_t mapping){
  uint32_t result=mapping&~UINT32_C(0xFFF);
  for(uint32_t component=0;component<4;++component){
    uint32_t input=(mapping>>(component*3))&7;
    if(input==0)input=2;else if(input==2)input=0;
    result|=input<<(component*3);
  }
  return result;
}
// Existing SDK border colors 0,1,2 have equal red and blue; color 3 does not.
constexpr bool NeedsBorderRBCompensation(uint32_t border_color,bool backing_rb_swapped){
  return backing_rb_swapped && border_color==3;
}
}
