#ifndef EQEMU_MERCENARY_ROF2_WIRE_H
#define EQEMU_MERCENARY_ROF2_WIRE_H
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace Mercenary { namespace RoF2 {
using Bytes=std::vector<uint8_t>;
constexpr size_t MaximumRows=100, MaximumStances=10, MaximumNameBytes=63;
constexpr size_t RowPrefixBytes=58;
struct Stance { uint32_t index{}, id{}; };
struct Row {
 std::array<uint32_t,9> values{}; // template/type/subtype, gold costs, status, alternate costs/type
 uint8_t unknown36{}; int32_t time_left{};
 uint32_t command{}, selected_stance{}, unknown53{}; uint8_t unknown57{};
 std::string name; std::vector<Stance> stances;
};
inline void Put32(Bytes& out,uint32_t value) {for(unsigned n=0;n<4;++n)out.push_back(static_cast<uint8_t>(value>>(8*n)));}
inline bool AppendRow(Bytes& out,const Row& row) {
 if(row.name.size()>MaximumNameBytes || row.name.find('\0')!=std::string::npos || row.stances.size()>MaximumStances)return false;
 for(auto value:row.values)Put32(out,value);
 out.push_back(row.unknown36);Put32(out,static_cast<uint32_t>(row.time_left));
 Put32(out,row.command);Put32(out,row.selected_stance);Put32(out,static_cast<uint32_t>(row.stances.size()));
 Put32(out,row.unknown53);out.push_back(row.unknown57);
 out.insert(out.end(),row.name.begin(),row.name.end());out.push_back(0);
 for(const auto& stance:row.stances){Put32(out,stance.index);Put32(out,stance.id);}
 return true;
}
inline std::optional<Bytes> Offers(const std::vector<uint32_t>& grades,const std::vector<Row>& rows) {
 if(grades.size()>10 || rows.size()>MaximumRows)return std::nullopt;
 Bytes out;Put32(out,static_cast<uint32_t>(grades.size()));for(auto grade:grades)Put32(out,grade);
 Put32(out,static_cast<uint32_t>(rows.size()));for(const auto& row:rows)if(!AppendRow(out,row))return std::nullopt;
 return out;
}
inline std::optional<Bytes> Roster(int32_t selected,const std::vector<Row>& rows,uint32_t capacity) {
 if(rows.size()>MaximumRows || selected < -1 || (selected>=0 && static_cast<size_t>(selected)>=rows.size()) || !capacity || capacity>MaximumRows)return std::nullopt;
 Bytes out;Put32(out,static_cast<uint32_t>(selected));Put32(out,static_cast<uint32_t>(rows.size()));
 for(const auto& row:rows)if(!AppendRow(out,row))return std::nullopt;
 // Native0x5684a0 consumes ONE final capacity after the entire counted row array.
 Put32(out,capacity);return out;
}
} }
#endif
