#ifndef X64_H
#define X64_H

#include <map>
#include <string>
#include <cstdint>

namespace kcc
{

enum Mnemonic
{
    MOV,
    PUSH,
    POP,
    RET
};

enum RegisterX64
{
    // General purpose registers
    kRAX,
    kRBX,
    kRCX,
    kRDX,
    kRDI,
    kRSI,
    kRBP,
    kRSP,
    kR8,
    kR9,
    kR10,
    kR11,
    kR12,
    kR13,
    kR14,
    kR15,

    kEAX,
    kEBX,
    kECX,
    kEDX,
    kESI,
    kEDI,
    kESP,
    kEBP,

    kAH, kAL,
    kBH, kBL,
    kCH, kCL,
    kDH, kDL,
    kBP,
    kSI,
    kDI,
    kSP,

    // segment registers
    kCS, kDS, kSS, kES, kFS, kGS
};

const static std::map<RegisterX64, std::string> reg_string = {
    {kRAX, "rax"},
    {kRBX, "rbx"},
    {kRCX, "rcx"},
    {kRDX, "rdx"},
    {kRDI, "rdi"},
    {kRSI, "rsi"},
    {kRBP, "rbp"},
    {kRSP, "rsp"},
    {kR8, "r8"},
    {kR9, "r9"},
    {kR10, "r10"},
    {kR11, "r11"},
    {kR12, "r12"},
    {kR13, "r13"},
    {kR14, "r14"},
    {kR15, "r15"}};

const static std::map<RegisterX64, bool> reg_use_map = {
    {kRAX, false},
    {kRBX, false},
    {kRCX, false},
    {kRDX, false},
    {kRDI, false},
    {kRSI, false},
    {kRBP, false},
    {kRSP, false},
    {kR8, false},
    {kR9, false},
    {kR10, false},
    {kR11, false},
    {kR12, false},
    {kR13, false},
    {kR14, false},
    {kR15, false}};

static std::map<RegisterX64, uint64_t> regs = {
    {kRAX, 0},
    {kRBX, 0},
    {kRCX, 0},
    {kRDX, 0},
    {kRDI, 0},
    {kRSI, 0},
    {kRBP, 0},
    {kRSP, 0},
    {kR8, 0},
    {kR9, 0},
    {kR10, 0},
    {kR11, 0},
    {kR12, 0},
    {kR13, 0},
    {kR14, 0},
    {kR15, 0}
};

static inline uint64_t RAX() { return static_cast<uint64_t>(regs[kRAX]); }
static inline uint32_t EAX() { return static_cast<uint32_t>(regs[kRAX] & 0xffffffff); }
static inline uint16_t AX() { return static_cast<uint16_t>(regs[kRAX] & 0xffff); }
static inline uint8_t AH() { return static_cast<uint8_t>((regs[kRAX] & 0xff00) >> 8); }
static inline uint8_t AL() { return static_cast<uint8_t>(regs[kRAX] & 0xff); }

static inline uint64_t RBX() { return static_cast<uint64_t>(regs[kRBX]); }
static inline uint32_t EBX() { return static_cast<uint32_t>(regs[kRBX] & 0xffffffff); }
static inline uint16_t BX() { return static_cast<uint16_t>(regs[kRBX] & 0xffff); }
static inline uint8_t BH() { return static_cast<uint8_t>((regs[kRBX] & 0xff00) >> 8); }
static inline uint8_t BL() { return static_cast<uint8_t>(regs[kRBX] & 0xff); }

} // namespace kcc

#endif
