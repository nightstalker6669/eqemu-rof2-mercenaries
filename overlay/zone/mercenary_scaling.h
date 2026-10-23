#ifndef EQEMU_MERCENARY_SCALING_H
#define EQEMU_MERCENARY_SCALING_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Mercenary {
// Emulator policy for gaps above the installed content ceiling, up to RoF2 level 100.
// Preserve the actual lower-level template; no fabricated database rows are required.
template <typename Stats>
bool ScaleAboveContentLevel(Stats& stats, uint32_t content_level, uint32_t owner_level)
{
	if (!content_level || owner_level <= content_level || owner_level > 100) {
		return false;
	}
	const long double ratio = static_cast<long double>(owner_level) / content_level;
	const auto scale = [](auto& field, long double factor) {
		using Value = std::remove_reference_t<decltype(field)>;
		if (field > 0) {
			field = static_cast<Value>(std::min(
				static_cast<long double>(std::numeric_limits<Value>::max()),
				static_cast<long double>(field) * factor));
		}
	};
	// Resources and damage grow quadratically; other numeric bonuses grow linearly.
	for (auto* field : {&stats.max_hp, &stats.current_hp}) {
		scale(*field, ratio * ratio);
	}
	scale(stats.Mana, ratio * ratio);
	scale(stats.min_dmg, ratio * ratio);
	scale(stats.max_dmg, ratio * ratio);
	scale(stats.AC, ratio); scale(stats.ATK, ratio);
	scale(stats.STR, ratio); scale(stats.STA, ratio); scale(stats.DEX, ratio);
	scale(stats.AGI, ratio); scale(stats.INT, ratio); scale(stats.WIS, ratio); scale(stats.CHA, ratio);
	scale(stats.MR, ratio); scale(stats.CR, ratio); scale(stats.DR, ratio);
	scale(stats.FR, ratio); scale(stats.PR, ratio); scale(stats.Corrup, ratio);
	scale(stats.hp_regen, ratio); scale(stats.mana_regen, ratio);
	stats.level = static_cast<decltype(stats.level)>(owner_level);
	return true;
}
}

#endif
