# ez80zdi library

> This library was extracted from [agon-recovery](https://github.com/envenomator/agon-recovery) tool.

## Overview

This libary provides a way to use ez80 ZDI interface ESP32 microcontroller.

## Usage

```c++
#include "ez80zdi.h"
#include "ez80cpu.h"


ZDI* zdi;
CPU* cpu;

void init_ez80();

void setup() {
    zdi = new ZDI(ZDI_TCKPIN, ZDI_TDIPIN);
    cpu = new CPU(zdi);

    init_ez80();
}

void init_ez80() {
    cpu->setBreak();
    cpu->setADLmode(true);
    cpu->instruction_di();

    // configure cpu
    // set stack
    cpu->sp(0xBFFFF);
    // set PC to some program
    cpu->pc(0x40000);

    cpu->setContinue(); // start cpu after break
}
```