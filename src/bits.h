#pragma once

#include <cstdint>

// ‚»‚Ì17 ƒrƒbƒg‰‰ŽZ‚ ‚ê‚±‚ê http://marupeke296.com/TIPS_No17_Bit.html
namespace bits
{
int count8bit(uint8_t v);
int count16bit(uint16_t v);
int count32bit(uint32_t v);
int count64bit(uint64_t v);
int MSB8bit(uint8_t v);
int MSB16bit(uint16_t v);
int MSB32bit(uint32_t v);
int MSB64bit(uint64_t v);
int LSB8bit(uint8_t v);
int LSB16bit(uint16_t v);
int LSB32bit(uint32_t v);
int LSB64bit(uint64_t v);
int calcSquare8bit(uint8_t v);
int calcSquare16bit(uint16_t v);
int calcSquare32bit(uint32_t v);
int calcSquare64bit(uint64_t v);
}
