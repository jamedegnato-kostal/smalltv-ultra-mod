#include "BacklightPlan.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
static long litTimes256(const smalltv::BacklightPlan& p, bool inv) {
  if (!p.sigmaDelta) return (inv ? !p.level : p.level) ? 256 : 0;
  return inv ? 256 - p.duty : p.duty;
}
int main() {
  using namespace smalltv;
  for (int inv=0; inv<2; ++inv) {
    long previous=-1;
    for (int step=0; step<=255; ++step) {
      const auto p=backlightPlan(static_cast<uint8_t>(step),inv!=0);
      if (step==0 || step==255) assert(!p.sigmaDelta);
      else assert(p.sigmaDelta && p.duty>=1 && p.duty<=255);
      const long lit=litTimes256(p,inv!=0);
      assert(lit>previous); previous=lit;
      const long error=std::labs(lit*255-static_cast<long>(step)*256);
      assert(2*error<=255);
    }
  }
  for(int step=1;step<255;++step) {
    const auto a=backlightPlan(static_cast<uint8_t>(step),false);
    const auto b=backlightPlan(static_cast<uint8_t>(step),true);
    assert(a.duty+b.duty==256);
  }
  assert(backlightClockHz()==1000000u);
  assert(backlightPulseHz(1)>=3900u && backlightPulseHz(255)>=3900u);
  assert(backlightPlanSame(backlightPlan(50,true),backlightPlan(50,true)));
  assert(!backlightPlanSame(backlightPlan(0,true),backlightPlan(1,true)));
  puts("backlight plan tests passed");
  return 0;
}
