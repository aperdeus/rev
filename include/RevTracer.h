//
// _RevTracer_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTRACER_H_
#define _SST_REVCPU_REVTRACER_H_

// -- SST Headers
#include "SST.h"

// -- Standard Headers
#include <cstdint>
#include <ostream>
#include <string>
#include <iostream>
#include <map>
#include <vector>

// -- Rev Headers

// Integrated Disassembler (toolchain dependent)
#ifndef NO_REV_TRACER
#include "riscv/disasm.h"
#endif

// Tracing macros
#ifndef NO_REV_TRACER
#define TRACE_REG_READ(R,V)  { if (Tracer) Tracer->regRead(  (uint8_t) (R),(uint64_t) (V) ); }
#define TRACE_REG_WRITE(R,V) { if (Tracer) Tracer->regWrite( (uint8_t) (R),(uint64_t) (V) ); }
#define TRACE_PC_WRITE(PC)   { if (Tracer) Tracer->pcWrite( (uint64_t) (PC) ); }
#define TRACE_MEM_WRITE(ADR, LEN, DATA) { if (Tracer) Tracer->memWrite( (ADR), (LEN), (DATA) ); }
#define TRACE_MEM_READ(ADR, LEN, DATA)  { if (Tracer) Tracer->memRead(  (ADR), (LEN), (DATA) ); }
#else
#define TRACE_REG_READ(R,V)
#define TRACE_REG_WRITE(R,V)
#define TRACE_PC_WRITE(PC)
#define TRACE_MEM_WRITE(ADR, LEN, DATA)
#define TRACE_MEM_READ(ADR, LEN, DATA)
#endif

namespace SST{
  namespace RevCPU{

  enum TRC_CONTROL_NOP : uint32_t {
    TRACE_OFF      = 0x00004013, // xori x0,x0,0
    TRACE_ON       = 0x00104013, // xori x0,x0,1
    TRACE_PUSH_OFF = 0x00204013, // xori x0,x0,2
    TRACE_PUSH_ON  = 0x00304013, // xori x0,x0,3
    TRACE_POP      = 0x00404013  // xori x0,x0,4
  };
  
  enum class EVENT_SYMBOL : unsigned {
    OK = 0x0,
    STALL = 0x1,
    FLUSH = 0x2,
    TRACE_ON = 0x100,
    TRACE_OFF = 0x102,
  };

  const std::map<EVENT_SYMBOL,char> event2char {
    {EVENT_SYMBOL::OK,' '},
    {EVENT_SYMBOL::STALL,'#'},
    {EVENT_SYMBOL::FLUSH,'!'},
    {EVENT_SYMBOL::TRACE_ON,'+'},
    {EVENT_SYMBOL::TRACE_OFF,'-'}
  };
    
  union TraceEvents_t {
    uint64_t v = 0;
    struct {
      // define 1 bit per stall type
      uint64_t stall : 1;           // [0]
      uint64_t stall_sources : 15;  // [15:1]
      uint64_t flush : 1;           // [16]
      uint64_t flush_sources : 15;  // [31:17]
      uint64_t spare : 31;          // [62:32]
      uint64_t trc_ctl : 1;         // [63] indicate change in trace controls
    } f;
  };

  enum TraceKeyword_t { RegRead, RegWrite, MemLoad, MemStore, PcWrite };

  // Generic record so we can preserve code ordering of events in trace
  //     // register       memory
  // a;  // reg             adr
  // b;  // value           len
  // c;  // origin(TODO)    data (limit 8 bytes)
  struct TraceRec_t {
    TraceKeyword_t key;
                 // register        memory
    uint64_t a;  // reg             adr
    uint64_t b;  // value           len
    uint64_t c;  // origin(TODO)    data (limited to show 8 bytes)
    TraceRec_t() {};
    TraceRec_t(TraceKeyword_t Key, uint64_t A, uint64_t B, uint64_t C=0)
      : key(Key), a(A), b(B), c(C) {};
  };

  class RevTracer{
    public:
      RevTracer(std::string Name, SST::Output* output);
      ~RevTracer();

      int SetDisassembler(std::string machine);
      int SetTraceSymbols(std::map<uint64_t,std::string>* TraceSymbols);
      void SetStartCycle(uint64_t c);
      void SetCycleLimit(uint64_t c);
      void CheckUserControls(uint64_t cycle);
      void SetFetchedInsn(uint64_t _pc, uint32_t _insn);
      bool OutputEnabled();

      // 32 or 64 bit integer register file captures
      void regRead(uint8_t r, uint64_t v);
      void regWrite(uint8_t r, uint64_t v);

      // memory requests
      void memWrite(uint64_t adr, unsigned len, const void *data);
      void memRead(uint64_t adr, unsigned len, void *data);

      // program counter
      void pcWrite(uint64_t newpc);

      // render trace to output stream and reset capture buffer
      void InstTrace(size_t cycle, unsigned id, unsigned hart, unsigned tid);
  
      void SetOutputEnable(bool e) { outputEnabled=e; }
      void Reset();

    private:
      std::string name;
      // TODO Generic interface wrappers that allow using any disassembler
      #ifndef NO_REV_TRACER
      isa_parser_t* isaParser;
      disassembler_t* diasm;
      #endif
      SST::Output *pOutput;
      bool outputEnabled;     // disable output but continue capturing
      TraceEvents_t events;
      std::vector<TraceRec_t> traceRecs;
      uint64_t pc;
      uint32_t insn;
      std::map<uint64_t,std::string>* traceSymbols;
      uint64_t lastPC;    // avoid displaying sequential addresses
    
      // formatters
      void fmt_reg(uint8_t r, std::stringstream& s);
      void fmt_data(unsigned len, uint64_t data, std::stringstream& s);
      std::string RenderOneLiner();

      // user settings
      uint64_t startCycle;
      uint64_t cycleLimit;
      std::vector<bool> enableQ;
      // allow wrapping around for deep push/pop programming errors.
      unsigned enableQindex;
      const unsigned MAX_ENABLE_Q = 100;
      // trace counter
      uint64_t traceCycles;
      bool disabled;

    }; // class RevTracer

  } //namespace RevCPU
} //namespace SST

#endif //  _SST_REVCPU_REVTRACER_H_
