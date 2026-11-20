#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
static std::string source() {
  std::ifstream in("src/Gfx.cpp");
  return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}
int main() {
  const std::string s=source();
  const std::string signature="static void backlightSet(uint8_t steps, bool inverted)";
  const size_t first=s.find(signature);
  if(s.empty() || first==std::string::npos || s.find(signature,first+1)==std::string::npos) return 1;
  const size_t end=s.find("\n#endif",first);
  if(end==std::string::npos) return 2;
  const std::string body=s.substr(first,end-first);
  const size_t duty=body.find("sigmaDeltaWrite(0, plan.duty)");
  const size_t attach=body.find("sigmaDeltaAttachPin(TFT_BL)");
  const size_t detach=body.find("sigmaDeltaDetachPin(TFT_BL)");
  const size_t fixed=body.find("digitalWrite(TFT_BL, plan.level ? HIGH : LOW)");
  if(duty==std::string::npos || attach==std::string::npos || detach==std::string::npos ||
     fixed==std::string::npos || !(duty<attach && attach<detach && detach<fixed) ||
     body.find("analogWrite(TFT_BL")!=std::string::npos) return 3;
  std::cout << "backlight wiring tests passed\n";
  return 0;
}
