#include "../main/pulse.cpp"
#include <gtest/gtest.h>

#define setup() \
  uint8_t REG_OUT = 0, REG_IN = 0, REG_DEBUG = 0;\
  state_t state;\
  state.register_in = &REG_IN;\
  state.trig_in = 2;\
  state.gate_in = 3;\
  state.register_out = &REG_OUT;\
  state.register_debug = &REG_DEBUG;\
  state.pulse_out = 2;\
  state.start_out = 3;\
  state.multiplier = 16;\
  state.pulse_time = 200

#define run(f, n) for(int i=0; i<n; i++) f(&state)
#define flank_trig_high() REG_IN |= 1 << state.trig_in
#define flank_trig_low() REG_IN &= ~(1 << state.trig_in)
#define flank_gate_high() REG_IN |= 1 << state.gate_in
#define flank_gate_low() REG_IN &= ~(1 << state.gate_in)

#define read_sync() (REG_OUT & (1 << state.pulse_out))

TEST(Pulse, FlankReset) {
  setup();

  ASSERT_EQ(0, state.flank_trig_read); // Flank not triggered
  flankResetOnPulseLow(&state, &(state.trig_in), &(state.flank_trig_read));
  ASSERT_FALSE(flankTriggerOnPulseHigh(&state, &(state.trig_in), &(state.flank_trig_read)));
  ASSERT_EQ(0, state.flank_trig_read); // Flank not triggered
  flank_trig_high();
  flankResetOnPulseLow(&state, &(state.trig_in), &(state.flank_trig_read));
  ASSERT_TRUE(flankTriggerOnPulseHigh(&state, &(state.trig_in), &(state.flank_trig_read)));
  ASSERT_NE(0, state.flank_trig_read); // Flank triggered
  flank_trig_low();
  flankResetOnPulseLow(&state, &(state.trig_in), &(state.flank_trig_read));
  flankTriggerOnPulseHigh(&state, &(state.trig_in), &(state.flank_trig_read));
  ASSERT_EQ(0, state.flank_trig_read); // Flank not triggered
}

TEST(Pulse, SyncOnAndOff) {
  setup();

  ASSERT_EQ(0, state.sync_pulse_on); // No pulse counter
  ASSERT_EQ(0, read_sync()); // Sync is off
  makePulse(&state);
  syncManager(&state, &state.sync_pulse_on, state.pulse_out);
  ASSERT_NE(0, state.sync_pulse_on); // Pulse is on
  ASSERT_NE(0, read_sync()); // Sync is on

  for (int i = 0; i < state.pulse_time; i++) {
    ASSERT_NE(0, read_sync()); // Sync is on
    syncManager(&state, &(state.sync_pulse_on), state.pulse_out);
  }
  ASSERT_EQ(0, read_sync()); // Sync is off
}

TEST(Pulse, StartHostBeat) {
  setup();
  state.measure_counter = 200;
  int tempo = state.measure_counter / state.multiplier;

  startHostBeat(&state);
  ASSERT_NE(0, state.sync_pulse_on); // Pulse is on
  ASSERT_EQ(tempo, state.tempo); // Tempo is Measure Counter divided by MULTIPLYER
}

TEST(Pulse, BeatDivider) {
  setup();
  state.measure_counter = 100;
  state.beat_counter = 24;

  startHostBeat(&state);
  beatDividerTrig(&state);
  ASSERT_EQ(1, state.beat_counter);
}

TEST(Pulse, Pulse) {
  setup();

  int loops = 10;

  flank_gate_high();
  flank_trig_high();
  run(pulse, loops);
  flank_trig_low();
  run(pulse, loops); // 20 loops so far
  ASSERT_EQ(2 * loops, state.measure_counter);

  flank_trig_high();
  run(pulse, loops);
  ASSERT_EQ(2 * loops / state.multiplier, state.tempo);
  //ASSERT_EQ(5, state.beat_counter);
  ASSERT_EQ(5, state.pulse_counter);
}

TEST(Pulse, Gate) {
  setup();

  state.multiplier = 4;

  flank_gate_high();
  run(pulse, 78);
  flank_trig_high();
  run(pulse, 10);
  flank_trig_low();
  run(pulse, 10);
  flank_trig_high();
  run(pulse, 10);
  ASSERT_EQ(5, state.tempo);
  flank_trig_low();
  flank_gate_low();
  run(pulse, 50);
  flank_gate_high();
  flank_trig_high();
  run(pulse, 10);
  // If gate support is incorrect, the tempo will be off. When gate runs low,
  // any ongoing tempo measurement should be discarded and the last known tempo
  // should be saved.
  ASSERT_EQ(5, state.tempo);
  run(pulse, 10);

  //flank_trig_high();
  //run(pulse, 10);
  //flank_trig_low();
  //run(pulse, 10);
  //ASSERT_EQ(20, state.measure_counter);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

