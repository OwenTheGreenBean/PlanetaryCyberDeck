#pragma once

#include <Arduino.h>
#include "Config.h"

/// Non-blocking passive-buzzer player (GPIO8). Call tick() every loop.
class Audio {
public:
  static Audio& get() {
    static Audio inst;
    return inst;
  }

  void begin();
  void tick(uint32_t nowMs);

  void playStartup();
  void playPlanetSwitch();

private:
  struct Note {
    uint16_t freq;
    uint16_t durMs;
  };

  void queue(const Note* notes, int count);
  void stopTone();
  void startTone(uint16_t freq);

  static constexpr int kQueueMax = 12;
  Note _queue[kQueueMax];
  int _queueLen = 0;
  int _queuePos = 0;
  uint32_t _noteEndMs = 0;
  bool _playing = false;
  bool _attached = false;
};
