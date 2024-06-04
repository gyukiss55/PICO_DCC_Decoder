//PICO_ExtInterrupt.h

void ExtInterruptSetup(int interruptPin);

unsigned long GetLastMicros(unsigned long* buffer, unsigned long size);

void PrintSample();
void PrintStatistic();
