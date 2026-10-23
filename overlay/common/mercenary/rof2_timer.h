#pragma once

#include <cstdint>

namespace Mercenary::RoF2 {
// May 2013's native duration formatter has a fixed local buffer. Mixed
// hours/minutes/seconds strings overrun it before its bounded output copy.
constexpr bool SafeNativeDuration(std::uint32_t milliseconds)
{
	const auto seconds = milliseconds / 1000;
	return (seconds / 3600 != 0) + ((seconds / 60) % 60 != 0) + (seconds % 60 != 0) <= 1;
}
}
