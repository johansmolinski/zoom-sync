#include "../main/pulse.cpp"
#include <gtest/gtest.h>

#define setup() \
  uint8_t REG_OUT = 0, REG_IN = 0b00011000, REG_DEBUG = 0;\
  state_t state;\
  state.register_in = &REG_IN;\
  state.trig_in = 2;\
  state.gate_in = 3;\
  state.stop_in = 4;\
  state.gate_on = false;\
  state.register_out = &REG_OUT;\
  state.register_debug = &REG_DEBUG;\
  state.pulse_out = 2;\
  state.start_out = 3;\
  state.multiplier = 16;\
  state.pulse_time = 200

#define run(f, n) for(int i=0; i<n; i++) f(&state)

#define flank_trig_high() REG_IN |= 1 << state.trig_in
#define flank_trig_low() REG_IN &= ~(1 << state.trig_in)

#define press_start() REG_IN &= ~(1 << state.gate_in)
#define release_start() REG_IN |= 1 << state.gate_in
#define press_stop() REG_IN &= ~(1 << state.stop_in)
#define release_stop() REG_IN |= 1 << state.stop_in

#define read_flank(bit_in) REG_IN & (1 << bit_in)

#define read_sync_pulse() (REG_OUT & (1 << state.pulse_out))
#define read_sync_start() (REG_OUT & (1 << state.start_out))

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

  state.gate_on = true;
  ASSERT_EQ(0, state.sync_pulse_on); // No pulse counter
  ASSERT_EQ(0, read_sync_pulse()); // Sync is off
  makePulse(&state);
  syncManager(&state, &state.sync_pulse_on, state.pulse_out);
  ASSERT_NE(0, state.sync_pulse_on); // Pulse is on
  ASSERT_NE(0, read_sync_pulse()); // Sync is on

  for (int i = 0; i < state.pulse_time; i++) {
    ASSERT_NE(0, read_sync_pulse()); // Sync is on
    syncManager(&state, &(state.sync_pulse_on), state.pulse_out);
  }
  ASSERT_EQ(0, read_sync_pulse()); // Sync is off
}

TEST(Pulse, SyncNotActiveOnGateLow) {
  setup();

  state.gate_on = false;
  makePulse(&state);
  syncManager(&state, &state.sync_pulse_on, state.pulse_out);
  ASSERT_EQ(0, state.sync_pulse_on); // Pulse is on
  ASSERT_EQ(0, read_sync_pulse()); // Sync is on
}

TEST(Pulse, GateOnStartButton) {
  setup();

  release_start();
  ASSERT_TRUE(read_in(state.register_in, state.gate_in));
  buttonManager(&state);
  ASSERT_FALSE(state.gate_on);
  press_start();
  ASSERT_FALSE(read_in(state.register_in, state.gate_in));
  buttonManager(&state);
  ASSERT_TRUE(state.gate_on);
  release_start();
  buttonManager(&state);
  ASSERT_TRUE(state.gate_on);
}

TEST(Pulse, GateOffStopButton) {
  setup();

  // Press start button
  press_start();
  buttonManager(&state);
  release_start();
  ASSERT_TRUE(state.gate_on);
  // Press stop button
  press_stop();
  buttonManager(&state);
  ASSERT_FALSE(state.gate_on);
}

TEST(Pulse, StartHostBeat) {
  setup();
  state.measure_counter = 200;
  int tempo = state.measure_counter / state.multiplier;

  press_start();
  buttonManager(&state);
  release_start();
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

  press_start();
  flank_trig_high();
  run(pulse, loops);
  release_start();
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

  press_start();
  run(pulse, 78);
  release_start();
  flank_trig_high();
  run(pulse, 10);
  flank_trig_low();
  run(pulse, 10);
  flank_trig_high();
  run(pulse, 10);
  ASSERT_EQ(5, state.tempo);
  flank_trig_low();
  press_stop();
  run(pulse, 70);
  ASSERT_EQ(5, state.tempo);
  release_stop();
  ASSERT_FALSE(state.gate_on);
  press_start();
  flank_trig_high();
  run(pulse, 10);
  release_start();
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

TEST(Pulse, Gate2) {
  setup();

  state.multiplier = 4;

  flank_trig_high();
  run(pulse, 1);
  ASSERT_FALSE(state.gate_on);
  ASSERT_EQ(0, state.sync_pulse_on);

  flank_trig_low();
  run(pulse, 1);

  press_start();
  run(pulse, 1);
  release_start();
  flank_trig_high();
  run(pulse, 1);
  press_stop();
  run(pulse, 1);
  release_stop();
  run(pulse, 1);
  ASSERT_FALSE(state.gate_on);
  ASSERT_NE(0, state.sync_pulse_on);
}

TEST(Pulse, MakePulse) {
  setup();

  // Activate gate
  run(pulse, 1);
  press_start();
  run(pulse, 1);
  release_start();
  run(pulse, 1);
  ASSERT_TRUE(state.gate_on);

  // Trig sync pulse
  ASSERT_EQ(0, state.sync_pulse_on);
  flank_trig_high();
  run(pulse, 1);
  flank_trig_low();
  run(pulse, 1);
  ASSERT_NE(0, state.sync_pulse_on);
  ASSERT_NE(0, read_sync_pulse());
  run(pulse, 400);
  ASSERT_EQ(0, state.sync_pulse_on);
  run(pulse, 1);
  run(pulse, 1);

}

TEST(Pulse, StartSignal) {
  setup();

  press_start();
  run(pulse, 1);
  ASSERT_EQ(0, read_sync_start());
  flank_trig_high();
  run(pulse, 1);
  ASSERT_NE(0, read_sync_start());
  ASSERT_TRUE(state.gate_on);
  press_stop();
  run(pulse, 2);
  ASSERT_FALSE(state.gate_on);
  ASSERT_EQ(0, read_sync_start());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

