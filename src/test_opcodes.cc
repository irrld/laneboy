#include "emulator.h"
#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "lib/tester.h"

static size_t instruction_mem_size;

static int num_mem_accesses;
static struct mem_access mem_accesses[16];

static std::unique_ptr<MemoryBus> bus;
static std::unique_ptr<EventBus> event_bus;
static std::unique_ptr<CPU> cpu;
static std::unique_ptr<MemoryDevice> mem_md_;
static std::array<u8, 0x10000> mock_;
static std::unique_ptr<MemoryDevice> mock_md_;

u8 ReadWriteCallback(u16 address, u8 value, bool failed, MemoryAccess type) {
  if (type == kMemoryAccessRead) {
    return value;
  }

  if (type == kMemoryAccessRead) {
    struct mem_access *access = &mem_accesses[num_mem_accesses++];
    access->type = MEM_ACCESS_READ;
    access->addr = address;
    access->val = value;
  } else if (type == kMemoryAccessWrite) {
    struct mem_access *access = &mem_accesses[num_mem_accesses++];
    access->type = MEM_ACCESS_WRITE;
    access->addr = address;
    access->val = value;
    return 0xAA;
  }
  return value;
}

/*
 * Called once during startup. The area of memory pointed to by
 * tester_instruction_mem will contain instructions the tester will inject, and
 * should be mapped read-only at addresses [0,tester_instruction_mem_size).
 */
static void mycpu_init(size_t tester_instruction_mem_size,
                       uint8_t *tester_instruction_mem) {
  instruction_mem_size = tester_instruction_mem_size;

  bus = std::make_unique<MemoryBus>();
  event_bus = std::make_unique<EventBus>();
  cpu = std::make_unique<CPU>(*event_bus, *bus);
  cpu->bus_.Reset();
  mem_md_ = std::make_unique<TesterMemoryDevice>(0x0000, tester_instruction_mem, instruction_mem_size, ReadWriteCallback, kMemoryAccessRead);
  bus->AddDevice(0x0000, instruction_mem_size - 1, mem_md_.get(), false);
  mock_.fill(0xAA);
  mock_md_ = std::make_unique<TesterMemoryDevice>(0x0000, mock_.data(), mock_.size(), ReadWriteCallback, kMemoryAccessBoth);
  bus->AddDevice(0x0000, 0xFFFF, mock_md_.get(), false);
}

/*
 * Resets the CPU state (e.g., registers) to a given state state.
 */
static void mycpu_set_state(struct state *state) {
  num_mem_accesses = state->num_mem_accesses;
  if (num_mem_accesses > 0) {
    memcpy(mem_accesses, state->mem_accesses, num_mem_accesses);
  }

  cpu->halted_ = state->halted;
  cpu->ime_ = state->interrupts_master_enabled;
  cpu->registers_.sp = state->SP;
  cpu->registers_.pc = state->PC;
  cpu->registers_.a = state->reg8.A;
  cpu->registers_.flags.v = state->reg8.F;
  cpu->registers_.bc.v = state->reg16.BC;
  cpu->registers_.de.v = state->reg16.DE;
  cpu->registers_.hl.v = state->reg16.HL;
}

/*
 * Query the current state of the CPU.
 */
static void mycpu_get_state(struct state *state)
{
  state->num_mem_accesses = num_mem_accesses;
  memcpy(state->mem_accesses, mem_accesses, sizeof(mem_accesses));

  /* ... Copy your current CPU state into the provided struct ... */
  state->halted = cpu->halted_;
  state->interrupts_master_enabled = cpu->ime_;
  state->SP = cpu->registers_.sp;
  state->PC = cpu->registers_.pc;
  state->reg8.A = cpu->registers_.a;
  state->reg8.F = cpu->registers_.flags.v;
  state->reg16.BC = cpu->registers_.bc.v;
  state->reg16.DE = cpu->registers_.de.v;
  state->reg16.HL = cpu->registers_.hl.v;
}

/*
 * Step a single instruction of the CPU. Returns the amount of cycles this took
 * (e.g., NOP should return 4).
 */
static int mycpu_step(void) {
  cpu->cycles_consumed_ = 0;
  cpu->Step();
  return cpu->cycles_consumed_;
}

struct tester_operations myops = {
    .init = mycpu_init,
    .set_state = mycpu_set_state,
    .get_state = mycpu_get_state,
    .step = mycpu_step,
};


extern struct tester_operations myops;

static struct tester_flags flags = {
    .keep_going_on_mismatch = 0,
    .enable_cb_instruction_testing = 1,
    .print_tested_instruction = 0,
    .print_verbose_inputs = 0,
};

static void print_usage(char *progname)
{
  printf("Usage: %s [option]...\n\n", progname);
  printf("Game Boy Instruction Tester.\n\n");
  printf("Options:\n");
  printf(" -k, --keep-going       Skip to the next instruction on a mismatch "
      "(instead of aborting all tests).\n");
  printf(" -c, --no-enable-cb     Disable testing of CB prefixed "
      "instructions.\n");
  printf(" -p, --print-inst       Print instruction undergoing tests.\n");
  printf(" -v, --print-input      Print every inputstate that is tested.\n");
  printf(" -h, --help             Show this help.\n");
}

static int parse_args(int argc, char **argv) {
  while (1) {
    static struct option long_options[] = {
        {"keep-going",   no_argument,        0,  'k'},
        {"no-enable-cb", no_argument,        0,  'c'},
        {"print-inst",   no_argument,        0,  'p'},
        {"print-input",  no_argument,        0,  'v'},
        {"help",         no_argument,        0,  'h'},
        {0, 0, 0, 0}
    };

    char c = getopt_long(argc, argv, "kcpvh", long_options, NULL);

    if (c == -1)
      break;

    switch (c) {
      case 'k':
        flags.keep_going_on_mismatch = 1;
        break;

      case 'c':
        flags.enable_cb_instruction_testing = 0;
        break;

      case 'p':
        flags.print_tested_instruction = 1;
        break;

      case 'v':
        flags.print_verbose_inputs = 1;
        break;

      case 'h':
        print_usage(argv[0]);
        exit(0);

      default:
        print_usage(argv[0]);
        return 1;
    }
  }

  if (optind != argc) {
    // We should not have any leftover arguments.
    print_usage(argv[0]);
    return 1;
  }

  return 0;
}

int test_opcodes() {
//  if (parse_args(argc, argv)) {
//    return 1;
//  }

  flags.enable_cb_instruction_testing = 1;
  flags.print_tested_instruction = 1;
  flags.print_verbose_inputs = 1;
  int i = tester_run(&flags, &myops);

  return i;
}