#ifndef EQEMU_MERCENARY_TRANSACTION_H
#define EQEMU_MERCENARY_TRANSACTION_H

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

class Database;
namespace Mercenary {
enum class SaveResult { Saved, Failed, CommitUncertain };
struct AlternatePayment { uint32_t currency_id = 19, before = 0, after = 0; };
struct Payment {
	// Personal inventory coins only: copper, silver, gold, platinum.
	std::array<uint32_t, 4> before{};
	std::array<uint32_t, 4> after{};
	std::optional<AlternatePayment> alternate;
	// Called inside the same transaction; verifies/locks the authenticated writer lease.
	std::function<bool(Database&, uint64_t connection)> guard;
};
}

#endif
