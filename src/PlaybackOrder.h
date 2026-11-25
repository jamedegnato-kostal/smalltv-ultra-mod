#pragma once
#include <stdint.h>
namespace smalltv { namespace playback {
inline uint32_t nextShuffleValue(uint32_t& state) {
  if (!state) state = 0x6D2B79F5UL;
  state ^= state << 13; state ^= state >> 17; state ^= state << 5;
  return state;
}
inline void makeShuffleOrder(uint8_t* order, uint8_t count, uint32_t& state, uint8_t previousFirst = 0xFF) {
  for (uint8_t i=0;i<count;++i) order[i]=i;
  for (int16_t i=static_cast<int16_t>(count)-1;i>0;--i) {
    const uint8_t j=static_cast<uint8_t>(nextShuffleValue(state)%static_cast<uint16_t>(i+1));
    const uint8_t value=order[i]; order[i]=order[j]; order[j]=value;
  }
  if (count>1 && order[0]==previousFirst) {
    const uint8_t other=static_cast<uint8_t>(1+(nextShuffleValue(state)%static_cast<uint8_t>(count-1)));
    const uint8_t value=order[0]; order[0]=order[other]; order[other]=value;
  }
}
} }
