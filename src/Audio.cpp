#include "Audio.h"

#ifndef BUZZER_LEDC_CHANNEL
#define BUZZER_LEDC_CHANNEL 0
#endif

void Audio::begin() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(BUZZER_PIN, 2000, 8);
#else
  ledcSetup(BUZZER_LEDC_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_LEDC_CHANNEL);
#endif
  _attached = true;
  stopTone();
}

void Audio::stopTone() {
  if (!_attached) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(BUZZER_PIN, 0);
  ledcWrite(BUZZER_PIN, 0);
#else
  ledcWriteTone(BUZZER_LEDC_CHANNEL, 0);
  ledcWrite(BUZZER_LEDC_CHANNEL, 0);
#endif
}

void Audio::startTone(uint16_t freq) {
  if (!_attached) return;
  if (freq == 0) {
    stopTone();
    return;
  }
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(BUZZER_PIN, freq);
#else
  ledcWriteTone(BUZZER_LEDC_CHANNEL, freq);
#endif
}

void Audio::queue(const Note* notes, int count) {
  if (count > kQueueMax) count = kQueueMax;
  for (int i = 0; i < count; i++) _queue[i] = notes[i];
  _queueLen = count;
  _queuePos = 0;
  _playing = false;
  _noteEndMs = 0;
}

void Audio::playStartup() {
  static const Note kBoot[] = {
    {523, 80},
    {0, 30},
    {659, 80},
    {0, 30},
    {784, 120},
    {0, 40},
    {1047, 160},
  };
  queue(kBoot, (int)(sizeof(kBoot) / sizeof(kBoot[0])));
}

void Audio::playPlanetSwitch() {
  static const Note kSwitch[] = {
    {880, 60},
    {0, 25},
    {1175, 90},
  };
  queue(kSwitch, (int)(sizeof(kSwitch) / sizeof(kSwitch[0])));
}

void Audio::tick(uint32_t nowMs) {
  if (_queuePos >= _queueLen) {
    if (_playing) {
      stopTone();
      _playing = false;
    }
    return;
  }

  if (!_playing || nowMs >= _noteEndMs) {
    if (_playing) {
      _queuePos++;
      if (_queuePos >= _queueLen) {
        stopTone();
        _playing = false;
        return;
      }
    }

    const Note& n = _queue[_queuePos];
    startTone(n.freq);
    _noteEndMs = nowMs + n.durMs;
    _playing = true;
  }
}
