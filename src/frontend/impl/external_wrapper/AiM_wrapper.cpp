#include <filesystem>
#include <iostream>
#include <fstream>

#include "frontend/frontend.h"
#include "base/AiM_request.h"
#include "base/exception.h"
#include "addr_mapper/addr_mapper.h"

DECLARE_DEBUG_FLAG(AiMWrapper)

namespace Ramulator {

ENABLE_DEBUG_FLAG(AiMWrapper)

namespace fs = std::filesystem;

class AiMWrapper : public IFrontEnd, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IFrontEnd, AiMWrapper, "AiMWrapper", "AiM wrapper for another simulators.")

  public:
    void init() override {
      auto existing_logger = Logging::get("AiMWrapper");
      if (existing_logger) {
        m_logger = existing_logger;
      } else {
        m_logger = Logging::create_logger("AiMWrapper");
      }
      DEBUG_LOG(AiMWrapper, m_logger, "AiM Wrapper initialized!");
    };

    bool receive_external_requests(int req_type_id, Addr_t addr, int source_id, std::function<void(Request&)> callback) {
      Request req = Request(addr, req_type_id, source_id, callback);
      return m_memory_system->send(req);
    };

    bool receive_external_aim_requests(int req_type_id, Addr_t addr, std::function<void(Request&)> callback) {
      DEBUG_LOG(AiMWrapper, m_logger,
                "[AiMulator: Wrapper] Request type={} received!",
                req_type_id);

      int aim_num_banks = -1;
      bool is_aim = false;
      
      switch (req_type_id) {
        case Request::Type::MAC_SBK:
        case Request::Type::AF_SBK:
        case Request::Type::COPY_BKGB:
        case Request::Type::COPY_GBBK:
          aim_num_banks = 1;
          break;
        case Request::Type::MAC_4BK_INTRA_BG:
        case Request::Type::AF_4BK_INTRA_BG:
        case Request::Type::EWMUL:
        case Request::Type::EWADD:
          aim_num_banks = 4;
          break;
        case Request::Type::MAC_ABK:
        case Request::Type::AF_ABK:
        case Request::Type::WR_AFLUT:
        case Request::Type::WR_BK:
          aim_num_banks = 16;
          break;
        case Request::Type::WR_GB:
        case Request::Type::WR_MAC:
        case Request::Type::WR_BIAS:
        case Request::Type::RD_MAC:
        case Request::Type::RD_AF:
          aim_num_banks = 0;
          break;
        default:
          aim_num_banks = -1;
          break;
      }
      
      switch (aim_num_banks) {
        case -1:
          is_aim = false;
          break;
        case 0:
        case 1:
        case 4:
        case 16:
          is_aim = true;
          break;
        default:
          is_aim = false;
          break;
      }
      
      Request req = Request(is_aim, req_type_id, addr, aim_num_banks, callback);
      return m_memory_system->send(req);
    };

    void tick() override {};
  
  private:
    Logger_t m_logger;

  private:
    bool is_finished() override { return true; };
};

}        // namespace Ramulator