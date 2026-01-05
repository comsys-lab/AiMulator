#include <filesystem>
#include <iostream>
#include <fstream>

#include "frontend/frontend.h"
#include "base/AiM_request.h"
#include "base/exception.h"
#include "addr_mapper/addr_mapper.h"

namespace Ramulator {

namespace fs = std::filesystem;

class AiMPacketTrace : public IFrontEnd, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IFrontEnd, AiMPacketTrace, "AiMPacketTrace", "AiM memory address trace.")

  public:
    IDRAM* m_dram = nullptr;
    std::vector<int> m_addr_bits;
    Addr_t m_tx_offset = -1;

  public:
    void init() override {
      std::string trace_path_str = param<std::string>("path").desc("Path to the read write trace file.").required();
      m_clock_ratio = param<uint>("clock_ratio").required();
      m_addr_mapper = create_child_ifce<IAddrMapper>();
      auto existing_logger = Logging::get("AiMPacketTrace");
      if (existing_logger) { m_logger = existing_logger; }
      else {
        m_logger = Logging::create_logger("AiMPacketTrace");
        m_logger->info("Loading trace file {} ...", trace_path_str);
        init_trace(trace_path_str);
        m_logger->info("Loaded {} lines.", m_trace.size());
      }
    };

    // AiMulator trace-based
    void tick() override {
      if (m_curr_trace_idx >= m_trace.size()) return;

      const Trace& t = m_trace[m_curr_trace_idx];
      bool request_sent = false;

      if (!t.is_aim) {
        Request rw_req = Request(t.addr, t.type_id);
        request_sent = m_memory_system->send(rw_req);
      } else {
        std::vector<Request> aim_reqs = convert_aim_pkt_trace_to_aim_reqs(t);
        request_sent = m_memory_system->send(aim_reqs);
      }

      if (request_sent) {
        m_curr_trace_idx = (m_curr_trace_idx + 1) % m_trace_length;
        m_trace_count++;
      }

      // TODO: Handle rejected requests assuming Instruction set register (ISR) FSM
      // Do we have to handle for each channel?
      // else {}
    };

    // TODO: FIXME
    bool is_finished() override {
      return m_trace_count >= m_trace_length;
    };

  private:
    IAddrMapper* m_addr_mapper = nullptr;
    std::vector<Trace> m_trace;
    
    size_t m_trace_length = 0;
    size_t m_curr_trace_idx = 0;
    size_t m_trace_count = 0;
    
    Logger_t m_logger;
  
  private:
    void init_trace(const std::string& file_path_str) {
      fs::path trace_path(file_path_str);
      if (!fs::exists(trace_path)) {
        throw ConfigurationError("Trace {} does not exist!", file_path_str);
      }
      std::ifstream trace_file(trace_path);
      if (!trace_file.is_open()) {
        throw ConfigurationError("Trace {} cannot be opened!", file_path_str);
      }

      std::string line;
      while (std::getline(trace_file, line)) {
        if (line[0] == '#' || line.empty()){
          continue; // comment or empty line
        }

        std::vector<std::string> tokens;
        tokenize(tokens, line, " ");
        Trace trace_entry {};

        if (tokens.empty()) continue;

        if (tokens[0] == "R") {
          trace_entry.type_id = Request::Type::Read;
          trace_entry.aim_num_banks = -1;
        } else if (tokens[0] == "W") {
          trace_entry.type_id = Request::Type::Write;
          trace_entry.aim_num_banks = -1;
        } else if (tokens[0] == "MAC_SBK") {
          trace_entry.type_id = Request::Type::MAC_SBK;
          trace_entry.aim_num_banks = 1;
        } else if (tokens[0] == "AF_SBK") {
          trace_entry.type_id = Request::Type::AF_SBK;
          trace_entry.aim_num_banks = 1;
        } else if (tokens[0] == "COPY_BKGB") {
          trace_entry.type_id = Request::Type::COPY_BKGB;
          trace_entry.aim_num_banks = 1;
        } else if (tokens[0] == "COPY_GBBK") {
          trace_entry.type_id = Request::Type::COPY_GBBK;
          trace_entry.aim_num_banks = 1;
        } else if (tokens[0] == "MAC_4BK_INTRA_BG") {
          trace_entry.type_id = Request::Type::MAC_4BK_INTRA_BG;
          trace_entry.aim_num_banks = 4;
        } else if (tokens[0] == "AF_4BK_INTRA_BG") {
          trace_entry.type_id = Request::Type::AF_4BK_INTRA_BG;
          trace_entry.aim_num_banks = 4;
        } else if (tokens[0] == "EWMUL") {
          trace_entry.type_id = Request::Type::EWMUL;
          trace_entry.aim_num_banks = 4;
        } else if (tokens[0] == "EWADD") {
          trace_entry.type_id = Request::Type::EWADD;
          trace_entry.aim_num_banks = 4;
        } else if (tokens[0] == "MAC_ABK") {
          trace_entry.type_id = Request::Type::MAC_ABK;
          trace_entry.aim_num_banks = 16;
        } else if (tokens[0] == "AF_ABK") {
          trace_entry.type_id = Request::Type::AF_ABK;
          trace_entry.aim_num_banks = 16;
        } else if (tokens[0] == "WR_AFLUT") {
          trace_entry.type_id = Request::Type::WR_AFLUT;
          trace_entry.aim_num_banks = 16;
        } else if (tokens[0] == "WR_BK") {
          trace_entry.type_id = Request::Type::WR_BK;
          trace_entry.aim_num_banks = 16;
        } else if (tokens[0] == "WR_GB") {
          trace_entry.type_id = Request::Type::WR_GB;
          trace_entry.aim_num_banks = 0;
        } else if (tokens[0] == "WR_MAC") {
          trace_entry.type_id = Request::Type::WR_MAC;
          trace_entry.aim_num_banks = 0;
        } else if (tokens[0] == "WR_BIAS") {
          trace_entry.type_id = Request::Type::WR_BIAS;
          trace_entry.aim_num_banks = 0;
        } else if (tokens[0] == "RD_MAC") {
          trace_entry.type_id = Request::Type::RD_MAC;
          trace_entry.aim_num_banks = 0;
        } else if (tokens[0] == "RD_AF") {
          trace_entry.type_id = Request::Type::RD_AF;
          trace_entry.aim_num_banks = 0;
        } else {
          throw ConfigurationError("Trace {} format invalid!", file_path_str);
        }
        // else if (tokens[0] == "MAC_4BK_INTER_BG") {
        //   trace_entry.type_id = Request::Type::MAC_4BK_INTER_BG;
        //   trace_entry.aim_num_banks = 4;
        // } else if (tokens[0] == "AF_4BK_INTER_BG") {
        //   trace_entry.type_id = Request::Type::AF_4BK_INTER_BG;
        //   trace_entry.aim_num_banks = 4;
        // } 
        
        switch (trace_entry.aim_num_banks) {
          case -1:
            trace_entry.is_aim = false;
            trace_entry.addr = std::stoll(tokens[1]);
            break;
          case 0:
            trace_entry.is_aim = true;
            trace_entry.ch_mask = static_cast<uint16_t>(std::stoi(tokens[1]));
            // trace_entry.reg_id = static_cast<int16_t>(std::stoi(tokens[2]));
            break;
          case 1:
            trace_entry.is_aim = true;
            trace_entry.ch_mask = static_cast<uint16_t>(std::stoi(tokens[1]));
            trace_entry.addr = std::stoll(tokens[7]);
            break;
          case 4:
            trace_entry.is_aim = true;
            trace_entry.ch_mask = static_cast<uint16_t>(std::stoi(tokens[1]));
            trace_entry.rank_addr = static_cast<uint16_t>(std::stoi(tokens[2]));
            trace_entry.pch_addr = static_cast<uint16_t>(std::stoi(tokens[3]));
            trace_entry.bank_addr_or_mask = static_cast<uint16_t>(std::stoi(tokens[4]));
            trace_entry.row_addr = static_cast<uint32_t>(std::stoi(tokens[5]));
            trace_entry.col_addr = static_cast<uint16_t>(std::stoi(tokens[6]));
            break;
          case 16:
            trace_entry.is_aim = true;
            trace_entry.ch_mask = static_cast<uint16_t>(std::stoi(tokens[1]));
            trace_entry.rank_addr = static_cast<uint16_t>(std::stoi(tokens[2]));
            trace_entry.pch_addr = static_cast<uint16_t>(std::stoi(tokens[3]));
            trace_entry.bank_addr_or_mask = static_cast<uint16_t>(std::stoi(tokens[4]));
            trace_entry.row_addr = static_cast<uint32_t>(std::stoi(tokens[5]));
            trace_entry.col_addr = static_cast<uint16_t>(std::stoi(tokens[6]));
            break;
          default: break;
        }

        m_trace.push_back(trace_entry);
      }

      trace_file.close();
      m_trace_length = m_trace.size();
    };

    std::vector<Request> convert_aim_pkt_trace_to_aim_reqs(const Trace& t) {
      // Convert addresses of the trace to that of the request.
      std::vector<Addr_t> aim_req_addrs = m_addr_mapper->convert_pkt_addr(t);
      // Initialize the request
      std::vector<Request> aim_reqs(aim_req_addrs.size(), Request(t.is_aim, t.type_id, t.aim_num_banks));
      // Copy aim_req and assign address for each
      for (int i = 0; i < aim_req_addrs.size(); i++) {
        aim_reqs[i].addr = aim_req_addrs[i];
      }
      return aim_reqs;
    };

};

}        // namespace Ramulator