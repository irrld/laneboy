#include "cpu.h"
#include "debug.h"

int main() {
  using clock = std::chrono::steady_clock;

  std::vector<u8> boot = LoadBin("rom/cgb_boot.bin");
  std::vector<u8> game = LoadBin("rom/zelda.gb");

  CPU cpu;
  cpu.clock_speed_ = 4194304; // 4.19MHz, in T-cycles
  //cpu.clock_speed_ = 4194; // 4.19MHz, in T-cycles
  cpu.timer_period_ = cpu.clock_speed_ / 4096; // 1024
  cpu.div_period_ = cpu.clock_speed_ / 16384; // 256

  cpu.LoadBootRom(boot.data(), boot.size());
  cpu.LoadRom(game.data(), game.size());

  INIT_DEBUGGER(cpu);

  std::chrono::time_point<clock> next_cycle_time = clock::now();
  cpu.running_ = true;
  while (cpu.running_) {
    if (clock::now() >= next_cycle_time) {
      if (!cpu.halted_) {
        cpu.HandleInterrupts();
        cpu.Step();
      }
      // todo check
      next_cycle_time += std::chrono::microseconds(1000000 / cpu.clock_speed_);
    }
  }

  cpu.registers_.Print();

  // destroy

}
