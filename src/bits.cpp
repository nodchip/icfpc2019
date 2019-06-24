#include "bits.h"

namespace bits
{
int count8bit(uint8_t v) {
  uint32_t count = (v & 0x55) + ((v >> 1) & 0x55);
  count = (count & 0x33) + ((count >> 2) & 0x33);
  return (count & 0x0f) + ((count >> 4) & 0x0f);
}

int count16bit(uint16_t v) {
  uint16_t count = (v & 0x5555) + ((v >> 1) & 0x5555);
  count = (count & 0x3333) + ((count >> 2) & 0x3333);
  count = (count & 0x0f0f) + ((count >> 4) & 0x0f0f);
  return (count & 0x00ff) + ((count >> 8) & 0x00ff);
}

int count32bit(uint32_t v) {
  uint32_t count = (v & 0x55555555) + ((v >> 1) & 0x55555555);
  count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
  count = (count & 0x0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f);
  count = (count & 0x00ff00ff) + ((count >> 8) & 0x00ff00ff);
  return (count & 0x0000ffff) + ((count >> 16) & 0x0000ffff);
}

int count64bit(uint64_t v) {
  uint64_t count = (v & 0x5555555555555555) + ((v >> 1) & 0x5555555555555555);
  count = (count & 0x3333333333333333) + ((count >> 2) & 0x3333333333333333);
  count = (count & 0x0f0f0f0f0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f0f0f0f0f);
  count = (count & 0x00ff00ff00ff00ff) + ((count >> 8) & 0x00ff00ff00ff00ff);
  count = (count & 0x0000ffff0000ffff) + ((count >> 16) & 0x0000ffff0000ffff);
  return (int)((count & 0x00000000ffffffff) + ((count >> 32) & 0x00000000ffffffff));
}

int MSB8bit(uint8_t v) {
  v |= (v >> 1);
  v |= (v >> 2);
  v |= (v >> 4);
  return count8bit(v) - 1;
}

int MSB16bit(uint16_t v) {
  v |= (v >> 1);
  v |= (v >> 2);
  v |= (v >> 4);
  v |= (v >> 8);
  return count16bit(v) - 1;
}

int MSB32bit(uint32_t v) {
  v |= (v >> 1);
  v |= (v >> 2);
  v |= (v >> 4);
  v |= (v >> 8);
  v |= (v >> 16);
  return count32bit(v) - 1;
}

int MSB64bit(uint64_t v) {
  v |= (v >> 1);
  v |= (v >> 2);
  v |= (v >> 4);
  v |= (v >> 8);
  v |= (v >> 16);
  v |= (v >> 32);
  return count64bit(v) - 1;
}

int LSB8bit(uint8_t v) {
  v |= (v << 1);
  v |= (v << 2);
  v |= (v << 4);
  return 8 - count8bit(v);
}

int LSB16bit(uint16_t v) {
  v |= (v << 1);
  v |= (v << 2);
  v |= (v << 4);
  v |= (v << 8);
  return 16 - count16bit(v);
}

int LSB32bit(uint32_t v) {
  v |= (v << 1);
  v |= (v << 2);
  v |= (v << 4);
  v |= (v << 8);
  v |= (v << 16);
  return 32 - count32bit(v);
}

int LSB64bit(uint64_t v) {
  v |= (v << 1);
  v |= (v << 2);
  v |= (v << 4);
  v |= (v << 8);
  v |= (v << 16);
  v |= (v << 32);
  return 64 - count64bit(v);
}

int calcSquare8bit(uint8_t v) {
  return 1 << (MSB8bit(v - 1) + 1);
}

int calcSquare16bit(uint16_t v) {
  return 1 << (MSB16bit(v - 1) + 1);
}

int calcSquare32bit(uint32_t v) {
  return 1 << (MSB32bit(v - 1) + 1);
}

int calcSquare64bit(uint64_t v) {
  return 1 << (MSB64bit(v - 1) + 1);
}
}
