#include "../../src/PlaybackOrder.h"
#include <assert.h>
#include <stdint.h>
int main() {
  for(uint32_t seed=1;seed<256;++seed) {
    uint8_t order[32]={}; uint32_t state=seed;
    smalltv::playback::makeShuffleOrder(order,32,state,31);
    bool seen[32]={};
    for(uint8_t i=0;i<32;++i){assert(order[i]<32);assert(!seen[order[i]]);seen[order[i]]=true;}
    assert(order[0]!=31);
  }
  uint8_t one[1]={}; uint32_t state=1;
  smalltv::playback::makeShuffleOrder(one,1,state,0); assert(one[0]==0);
  return 0;
}
