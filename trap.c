#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "signal.h"


// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){

  case T_DIVIDE:
    // SIGFPE
    //tf->eip = (uint)(proc->signal_handlers[SIGFPE]);

    if (0) {
      cprintf("askdj");
    }

    if (proc->signal_handlers[SIGFPE] == (sighandler_t) -1) {
      cprintf("Error: no handler for signal. Kill process.");
      kill(proc->pid);
    }

    siginfo_t info;      
    info.signum = SIGFPE;

    /**
     * Save old registers for signal handler to return to main
     */
    /*uint old_eip = (uint) proc->tf->eip;
    *((uint*)(proc->tf->esp - 4)) = (uint) old_eip;
    *((siginfo_t*)(proc->tf->esp - 20)) = info;
    *((uint*)(proc->tf->esp - 24)) = proc->trampoline_address;

    proc->tf->esp = (uint)((proc->tf->esp) - (sizeof(siginfo_t))- 24);*/

    uint old_eip = proc->tf->eip + 4;
    *((uint*)(proc->tf->esp - 4)) = old_eip; 
    *((uint*)(proc->tf->esp - 8)) = tf->eax; //Volatile registers
    *((uint*)(proc->tf->esp - 12)) = tf->ecx;
    *((uint*)(proc->tf->esp - 16)) = tf->edx;
    *((siginfo_t*)(proc->tf->esp - 20)) = info;
    *((uint*) (proc->tf->esp - 24)) = proc->trampoline_address;

    proc->tf->esp-=24;


    //tf->esp = tf->esp-24;
    // what address the program will go to after current stack is done
    tf->eip = (uint)(proc->signal_handlers[SIGFPE]);
    break;

  case T_IRQ0 + IRQ_TIMER:
    // SIGALRM

    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();

    if (proc && ((tf->cs & 3) == 3) && proc->alarm_state == ALARM_SET) {
      
      proc->ticks --;

      if(proc->ticks == 0) {
        // send signal here, alarm is activated
        proc->alarm_state = ALARM_ACTIVATED;
        //tf->esp = tf->esp - 8; 
        //tf->esp = (uint)((tf->esp) - (sizeof(siginfo_t))- 4);
        proc->ticks = proc->alarm_ticks;
        //cprintf("ALARM_ACTIVATED");
      }
    
    }

    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER) {
    yield();
    if (proc->alarm_state == ALARM_ACTIVATED) {
      siginfo_t info;      
      info.signum = SIGALRM;
      /**((siginfo_t*)(proc->tf->esp - 4)) = info;
          *((uint) (tf->esp - sizeof(siginfo_t) - 4)) = tf->eip;

      proc->tf->esp = (uint)((proc->tf->esp) - (sizeof(siginfo_t))- 4);*/

      uint old_eip = proc->tf->eip;
      uint old_eax = proc->tf->eax;
      uint old_edx = proc->tf->edx;
      uint old_ecx = proc->tf->ecx;

      *((uint*)(proc->tf->esp - 4))  = old_eip; 
      *((uint*)(proc->tf->esp - 8))  = old_eax; //Volatile registers
      *((uint*)(proc->tf->esp - 12)) = old_ecx;
      *((uint*)(proc->tf->esp - 16)) = old_edx;
      *((siginfo_t*)(proc->tf->esp - 20)) = info;
      *((uint*) (proc->tf->esp - 24)) = proc->trampoline_address;

      proc->tf->esp-=24;

      proc->tf->eip = (uint)(proc->signal_handlers[SIGALRM]);
    }
  }

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
