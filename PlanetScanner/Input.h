#pragma once

#include "Planet.h"
#include "Config.h"

// =============================================================================
// Input Handler — pots & buttons with debounce / edge detection
// =============================================================================

enum class ButtonEvent : uint8_t {
  None,
  ScanPressed,
  MenuPressed
};

class InputHandler {
public:
  void begin();
  void update();

  /// Current planet index from upper pot (0 .. PlanetDatabase::COUNT-1).
  int planetIndex() const { return _planetIndex; }

  /// Current info page from lower pot.
  InfoPage infoPage() const { return _infoPage; }

  /// Consume one-shot button events (clears after read).
  ButtonEvent pollButton();

  /// True when upper pot moved to a new planet slot this frame.
  bool planetChanged() const { return _planetChanged; }

  /// True when lower pot moved to a new page slot this frame.
  bool pageChanged() const { return _pageChanged; }

  void clearChangeFlags();

private:
  int mapPotToIndex(int raw, int count) const;
  InfoPage mapPotToPage(int raw) const;

  int _planetIndex = 0;
  InfoPage _infoPage = InfoPage::Overview;
  bool _planetChanged = false;
  bool _pageChanged = false;

  int _lastPlanetSlot = -1;
  int _lastPageSlot = -1;

  bool _btnScanPrev = true;
  bool _btnMenuPrev = true;
  ButtonEvent _pendingEvent = ButtonEvent::None;
};
