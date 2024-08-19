/*
MIT License

Copyright (c) 2024 Jeroen Venema

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef __EZ80CPU_H_
#define __EZ80CPU_H_
#include <Arduino.h>
#include <stdint.h>
#include "ez80zdi.h"

class CPU {
public:
    CPU(ZDI* zdi);
    // (Break) control
    void reset(void);
    void setBreak(void);
    void SingleStep(void);
    void setContinue(void);
    void setBreakpoint(uint8_t index, uint32_t address);
uint32_t getBreakpoint(uint8_t index);
    void enableBreakpoint(uint8_t index);
    void disableBreakpoint(uint8_t index);
    bool isBreakpointEnabled(uint8_t index);

    // CPU status
    bool isRunning(void);
    int8_t onBreakpoint(void); // -1: no match, or 0-3 on match
    bool onHaltSleep(void);
    bool getADLmode(void);
    void setADLmode(bool adlmode);
    bool getMADLmode(void);
    bool getIEF1flag(void);

    // execute specific instructions on actual ez80 CPU
    void instruction_out(uint8_t port_address, uint8_t value);
    void instruction_di(void);
    void exx(void);
    
    // get-set CPU registers
     uint8_t a(void);
        void a(uint8_t);
     uint8_t f(void);
        void f(uint8_t);
     uint8_t b(void);
        void b(uint8_t);
     uint8_t c(void);
        void c(uint8_t);
     uint8_t bcu(void);
        void bcu(uint8_t);
     uint8_t d(void);
        void d(uint8_t);
     uint8_t e(void);
        void e(uint8_t);
     uint8_t deu(void);
        void deu(uint8_t);
     uint8_t h(void);
        void h(uint8_t);
     uint8_t l(void);
        void l(uint8_t);
     uint8_t hlu(void);
        void hlu(uint8_t);
    uint32_t bc(void);
        void bc(uint32_t);
    uint32_t de(void);
        void de(uint32_t);
    uint32_t hl(void);
        void hl(uint32_t);
    uint32_t ix(void);
        void ix(uint32_t);
     uint8_t ixu(void);
        void ixu(uint8_t);
     uint8_t ixh(void);
        void ixh(uint8_t);
     uint8_t ixl(void);
        void ixl(uint8_t);
    uint32_t iy(void);
        void iy(uint32_t);
     uint8_t iyu(void);
        void iyu(uint8_t);
     uint8_t iyh(void);
        void iyh(uint8_t);
     uint8_t iyl(void);
        void iyl(uint8_t);
    uint32_t sp(void);
        void sp(uint32_t);
    uint32_t pc(void);
        void pc(uint32_t);
     uint8_t mbase(void); // read-only, datasheet is ambivalent here. Writing mbase doesn't work
private:
    ZDI* zdi;
    uint8_t debug_flags;
    uint32_t breakpoints[MAXBREAKPOINTS];
    bool    enabled_breakpoints[MAXBREAKPOINTS];
};

#endif