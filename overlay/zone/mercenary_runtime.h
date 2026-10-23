#ifndef EQEMU_MERCENARY_RUNTIME_H
#define EQEMU_MERCENARY_RUNTIME_H

#include <algorithm>
#include <cstdint>

namespace Mercenary {
class PassiveUpkeepClock {
public:
	uint32_t Sample(uint32_t now, bool active, bool passive, uint32_t period) {
		elapsed_ = std::min(elapsed_, period);
		if (sampled_ && active && passive) elapsed_ += std::min(period - elapsed_, uint32_t(now - last_));
		last_ = now; sampled_ = true;
		return elapsed_;
	}
	void Restore(uint32_t value, uint32_t now, uint32_t period) {
		elapsed_ = std::min(value, period); last_ = now; sampled_ = true;
	}
	void Reset(uint32_t now) { elapsed_ = 0; last_ = now; sampled_ = true; }
	uint32_t Elapsed() const { return elapsed_; }
private:
	uint32_t elapsed_ = 0, last_ = 0;
	bool sampled_ = false;
};
class PullerHealOverride {
public:
	void Arm(uint32_t now) { until_ = now + 3000; armed_ = true; }
	bool Active(uint32_t now) const { return armed_ && int32_t(until_ - now) > 0; }
private:
	uint32_t until_ = 0;
	bool armed_ = false;
};
}

#endif
