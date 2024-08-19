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


#include "ez80zdi.h"
#include <Arduino.h>
#include "esp32_io.h"

ZDI::ZDI(uint8_t tckpin, uint8_t tdipin) {
    this->tckpin = tckpin;
    this->tdipin = tdipin;

    directWriteHigh(tckpin);
    directModeOutput(tckpin);    

    set_debugflags(0);
}

void ZDI::signal_start(void) {
    directWriteHigh(tdipin);
    directModeOutput(tdipin);
    directWriteHigh(tckpin);
    directWriteLow(tdipin);
}

void ZDI::signal_done(void) {
    write_bit(true);
}

void ZDI::signal_continue(void) {
    write_bit(false);
}

void ZDI::write_bit(bool bit) {
    directWriteLow(tckpin);
    if (bit)
        directWriteHigh(tdipin);
    else
        directWriteLow(tdipin);
    directModeOutput(tdipin);
    directWriteHigh(tckpin);
}

bool ZDI::read_bit(void) {
    bool bit;
    directWriteLow(tckpin);
    directModeInput(tdipin);
    bit = directRead(tdipin);
    directWriteHigh(tckpin);
    return bit;
}

void ZDI::address_register(uint8_t zdi_regnr, ZDI_RWBIT rw) {
    // write 7-bits, msb to lsb order
    for(int i=6;i>=0;i--) write_bit(zdi_regnr & (0b1<<i));
    write_bit(rw);
}

void ZDI::write_byte(uint8_t value) {
    // write 8-bits, msb to lsb order
    for(int i=7;i>=0;i--) write_bit(value & (0b1<<i));
}

uint8_t ZDI::read_byte(void) {
    uint8_t value = 0;
    //pinMode(tdipin, INPUT);
    // read 8-bits, msb to lsb order
    for(int i=7;i>=0;i--)
    {
        value = value << 1;
        value = value | read_bit();
    }
    //pinMode(tdipin, OUTPUT);
    return value;
}

uint8_t ZDI::read_register(ZDI_REG zdi_regnr) {
    uint8_t value;
    noInterrupts();
    signal_start();
    address_register(zdi_regnr, READ);
    signal_continue();
    value  = read_byte();
    signal_done();
    delayMicroseconds(3);
    interrupts();
    return value;
}

void ZDI::write_register(ZDI_REG zdi_regnr, uint8_t value) {
    //noInterrupts();
    //signal_start();
    //zdi_register (regnr,ZDI_WRITE);
    //zdi_separator (1);
    //zdi_write(value);
    //zdi_separator (1);
    //delayMicroseconds (3);
    //interrupts();

    noInterrupts();
    signal_start();
    address_register(zdi_regnr, WRITE);
    signal_continue();
    write_byte(value);
    signal_done();
    delayMicroseconds(3);
    interrupts();
}

uint32_t ZDI::read_cpu_register(ZDI_RWControl cpureg) {
    write_register(ZDI_RW_CTL, cpureg);
    uint8_t values[3];
    uint8_t* ptr = values;

    read_register(ZDI_RD_L, 3, values);
    return(values[2]<<16)+(values[1]<<8)+values[0];
}

void ZDI::write_cpu_register(ZDI_RWControl cpureg, uint32_t value) {
    uint8_t buffer[3];
    buffer[0] = value & 0xff;       // L
    buffer[1] = (value>>8) & 0xff;  // H
    buffer[2] = (value>>16) & 0xff; // U
    write_register(ZDI_WR_DATA_L, 3, buffer);
    write_register(ZDI_RW_CTL, cpureg | 0b10000000); // set high bit to indicate write
}

void ZDI::read_register(ZDI_REG zdi_regnr, uint8_t count, uint8_t* values) {
    signal_start();
    address_register(zdi_regnr, READ);
    while(count--) {
        signal_continue();
        *(values++) = read_byte();
    }
    signal_done();
}

void ZDI::write_register(ZDI_REG zdi_regnr, uint8_t count, uint8_t* values) {
    signal_start();
    address_register(zdi_regnr, WRITE);
    while(count--) {
        signal_continue();
        write_byte(*(values++));
    }
    signal_done();
}

uint16_t ZDI::get_productid(void) {
    uint8_t id[2];
    read_register(ZDI_ID_L, 2, id);
    return (id[1]<<8)+id[0];
}

uint8_t ZDI::get_revision(void) {
    return read_register(ZDI_ID_REV);
}

void ZDI::read_memory (uint32_t address, uint32_t count, uint8_t* buffer) {
    uint32_t oldpc;
    uint32_t i;

    oldpc = read_cpu_register(ZDI_RWControl::pc);
    write_cpu_register(ZDI_RWControl::pc, address);

    signal_start();
    address_register(ZDI_RD_MEM, READ);
    for(i=0;i<count;i++) {
        signal_continue();
        delayMicroseconds (3);
        *(buffer++) = read_byte();
    }
    signal_done();

    write_cpu_register(ZDI_RWControl::pc, oldpc);
}

void ZDI::write_memory(uint32_t address, uint32_t count, uint8_t* buffer) {
    uint32_t oldpc;
    uint32_t i;

    write_cpu_register(ZDI_RWControl::pc, address);

    signal_start();
    address_register(ZDI_WR_MEM, WRITE);
    for (i=0;i<count;i++) {
        signal_continue();
        delayMicroseconds (3);
        write_byte(*(buffer++));
    }
    signal_done();
    delayMicroseconds (3);
}

void ZDI::write_memory_32bit(uint32_t address, uint32_t value) {
    write_memory(address, 4, (uint8_t *)&value);
}

void ZDI::write_memory_24bit(uint32_t address, uint32_t value) {
    write_memory(address, 3, (uint8_t *)&value);
}

void ZDI::write_memory_16bit(uint32_t address, uint16_t value) {
    write_memory(address, 2, (uint8_t *)&value);
}

void ZDI::write_memory_8bit(uint32_t address, uint8_t value) {
    write_memory(address, 1, &value);
}

void ZDI::set_debugflags(uint8_t flags) {
    write_register(ZDI_BRK_CTL, flags);
}
