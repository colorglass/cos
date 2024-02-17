### 64bit data in 32bit machine cause inline asm error with : " error: inconsistent operand constraints in an 'asm'"
While 64bit data is sperated into two 32bits register when calculated on 32bit machine, there is no constraints in inline asm for 64bit data to fit in on a 32bit machine.
### never to set the irq flag (sti) before initialize extern irq controller
When testing irq handler, I got strange bug which caused by it.
```
asm volatile("sti");
asm volatile("int $20");
```
when set the irq flag, and after `int $20`, there caused a #DP execuption without errorcode maybe as a result of an uninitialized extern irq controller, which may assert `INTR` pin when handling `int $20`.