#pragma once
#include <vector>

namespace BoosterType {
  static constexpr int MANIPULATOR = 0;
  static constexpr int FAST_WHEEL = 1;
  static constexpr int DRILL = 2;
  static constexpr int CLONING = 3;
  static constexpr int TELEPORT = 4;
  static constexpr int N = 5;
};

namespace CellType {
  static constexpr int kEmpty = 0;
  static constexpr int kWrappedBit = 1 << 0;
  static constexpr int kObstacleBit = 1 << 1;
  static constexpr int kBoosterManipulatorBit = 1 << 2; // B
  static constexpr int kBoosterFastWheelBit = 1 << 3; // F
  static constexpr int kBoosterDrillBit = 1 << 4; // L
  static constexpr int kSpawnPointBit = 1 << 5; // X
  static constexpr int kBoosterTeleportBit = 1 << 6; // R
  static constexpr int kBoosterCloningBit = 1 << 7; // C
  static constexpr int kTeleportTargetBit = 1 << 8; // installed teleport target
}

struct BoosterInfo {
  int booster_type;
  int map_bit;
  char character;
};

extern const std::vector<BoosterInfo> boosters;

BoosterInfo boosterFromChar(char c);
