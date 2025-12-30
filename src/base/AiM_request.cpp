#include "base/AiM_request.h"

namespace Ramulator {

Request::Request(Addr_t addr, int type_id): addr(addr), type_id(type_id) {};
Request::Request(AddrHierarchy_t addr_h, int type_id): addr_h(addr_h), type_id(type_id) {};
Request::Request(Addr_t addr, int type_id, int source_id, std::function<void(Request&)> callback):
addr(addr), type_id(type_id), source_id(source_id), callback(callback) {};

Request::Request(bool is_aim_req, int type_id, int aim_num_banks): is_aim_req(is_aim_req), type_id(type_id), aim_num_banks(aim_num_banks) {};
}        // namespace Ramulator