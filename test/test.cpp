#include "../main/pulse.cpp"
#include <gtest/gtest.h>

#define setup() \
  uint8_t REG_OUT = 0, REG_IN = 0;\
  state_t state;\
  state.register_in = &REG_IN;\
  state.bit_in = 2;\
  state.register_out = &REG_OUT;\
  state.bit_out = 5

#define run(f, n) for(int i=0; i<n; i++) f(&state)
#define flank_high() REG_IN = 0xff
#define flank_low() REG_IN = 0x00

TEST(Pulse, FlankReset) {
  setup();

  ASSERT_EQ(0, state.flank_read); // Flank not triggered
  flankResetOnPulseLow(&state);
  ASSERT_FALSE(flankTriggerOnPulseHigh(&state));
  ASSERT_EQ(0, state.flank_read); // Flank not triggered
  flank_high();
  flankResetOnPulseLow(&state);
  ASSERT_TRUE(flankTriggerOnPulseHigh(&state));
  ASSERT_NE(0, state.flank_read); // Flank triggered
  flank_low();
  flankResetOnPulseLow(&state);
  flankTriggerOnPulseHigh(&state);
  ASSERT_EQ(0, state.flank_read); // Flank not triggered
}

TEST(Pulse, SyncOnAndOff) {
  setup();

  ASSERT_EQ(0, state.sync_on); // No pulse counter
  ASSERT_EQ(0, REG_OUT); // Sync is off
  makePulse(&state);
  syncManager(&state);
  ASSERT_NE(0, state.sync_on); // Pulse is on
  ASSERT_NE(0, REG_OUT); // Sync is on

  for (int i = 0; i < PULSE_TIME; i++) {
    ASSERT_NE(0, REG_OUT); // Sync is on
    syncManager(&state);
  }
  ASSERT_EQ(0, REG_OUT); // Sync is off
}

TEST(Pulse, StartHostBeat) {
  setup();
  state.measure_counter = 200;

  startHostBeat(&state);
  ASSERT_NE(0, state.sync_on); // Pulse is on
  ASSERT_EQ(50, state.tempo); // Tempo is Measure Counter divided by MULTIPLYER
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

  flank_high();
  run(pulse, loops);
  flank_low();
  run(pulse, loops); // 20 loops so far
  ASSERT_EQ(2 * loops, state.measure_counter);

  flank_high();
  run(pulse, loops);
  ASSERT_EQ(5, state.tempo);
  //ASSERT_EQ(5, state.beat_counter);
  ASSERT_EQ(1, state.pulse_counter);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

