#include "booster.h"
#include <cassert>

const std::vector<BoosterInfo> boosters = {
  {BoosterType::MANIPULATOR,    CellType::kBoosterManipulatorBit,   'B'},
  {BoosterType::FAST_WHEEL,     CellType::kBoosterFastWheelBit,     'F'},
  {BoosterType::DRILL,          CellType::kBoosterDrillBit,         'L'},
  {BoosterType::CLONING,        CellType::kBoosterCloningBit,       'C'},
  {BoosterType::TELEPORT,       CellType::kBoosterTeleportBit,      'R'},
};

BoosterInfo boosterFromChar(char c) {
  for (auto& b : boosters) {
    if (b.character == c) {
      return b;
    }
  }
  assert (false);
  return {};
}