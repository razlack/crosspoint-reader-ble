#include "NewTheme.h"

#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>

#include <cstdint>
#include <string>
#include <vector>

#include "RecentBooksStore.h"
#include "components/UITheme.h"
#include "components/icons/book.h"
#include "components/icons/book24.h"
#include "components/icons/cover.h"
#include "components/icons/file24.h"
#include "components/icons/folder.h"
#include "components/icons/folder24.h"
#include "components/icons/hotspot.h"
#include "components/icons/image24.h"
#include "components/icons/library.h"
#include "components/icons/recent.h"
#include "components/icons/settings2.h"
#include "components/icons/text24.h"
#include "components/icons/transfer.h"
#include "components/icons/wifi.h"
#include "fontIds.h"

namespace {
constexpr int cornerRadius = 6;
constexpr int mainMenuIconSize = 32;

const uint8_t* iconForName(UIIcon icon, int size) {
  if (size == 24) {
    switch (icon) {
      case UIIcon::Folder:
        return Folder24Icon;
      case UIIcon::Text:
        return Text24Icon;
      case UIIcon::Image:
        return Image24Icon;
      case UIIcon::Book:
        return Book24Icon;
      case UIIcon::File:
        return File24Icon;
      default:
        return nullptr;
    }
  }
  if (size == 32 || size == 64) {
    switch (icon) {
      case UIIcon::Folder:
        return FolderIcon;
      case UIIcon::Book:
        return BookIcon;
      case UIIcon::Recent:
        return RecentIcon;
      case UIIcon::Settings:
        return Settings2Icon;
      case UIIcon::Transfer:
        return TransferIcon;
      case UIIcon::Library:
        return LibraryIcon;
      case UIIcon::Wifi:
        return WifiIcon;
      case UIIcon::Hotspot:
        return HotspotIcon;
      default:
        return nullptr;
    }
  }
  return nullptr;
}
}  // namespace

void NewTheme::drawRecentBookCover(GfxRenderer& renderer, Rect rect, const std::vector<RecentBook>& recentBooks,
                                   const int selectorIndex, bool& coverRendered, bool& coverBufferStored,
                                   bool& bufferRestored, std::function<bool()> storeCoverBuffer) const {
  const bool hasContinueReading = !recentBooks.empty();
  const int coverHeight = NewMetrics::values.homeCoverHeight;
  int coverWidth = std::min(rect.width - 12, static_cast<int>(coverHeight * 0.78f));
  int coverX = rect.x + (rect.width - coverWidth) / 2;
  const int coverY = rect.y;
  const bool selected = selectorIndex == 0;

  if (hasContinueReading) {
    RecentBook book = recentBooks[0];
    bool hasCover = true;
    const std::string coverBmpPath = UITheme::getCoverThumbPath(book.coverBmpPath, coverHeight);

    const bool needCoverRerender = !coverRendered || !bufferRestored;
    if (needCoverRerender) {
      if (book.coverBmpPath.empty()) {
        hasCover = false;
      } else {
        FsFile file;
        if (Storage.openFileForRead("HOME", coverBmpPath, file)) {
          Bitmap bitmap(file);
          if (bitmap.parseHeaders() == BmpReaderError::Ok) {
            // Adjust coverWidth to match image aspect ratio
            float aspect = (float)bitmap.getWidth() / bitmap.getHeight();
            int maxWidth = rect.width - 12;
            coverWidth = std::min(maxWidth, static_cast<int>(coverHeight * aspect));
            coverX = rect.x + (rect.width - coverWidth) / 2;  // Recalculate coverX
            renderer.fillRect(coverX, coverY, coverWidth, coverHeight, false);
            renderer.drawBitmap(bitmap, coverX, coverY, coverWidth, coverHeight);
          } else {
            hasCover = false;
          }
          file.close();
        } else {
          hasCover = false;
        }
      }

      renderer.drawRect(coverX, coverY, coverWidth, coverHeight, true);
      if (!hasCover) {
        renderer.fillRect(coverX, coverY, coverWidth, coverHeight, false);
        renderer.fillRect(coverX, coverY + (coverHeight / 3), coverWidth, 2 * coverHeight / 3, true);
        renderer.drawIcon(CoverIcon, coverX + 24, coverY + 24, 32, 32);
      }

      if (!coverRendered) {
        coverBufferStored = storeCoverBuffer();
        coverRendered = coverBufferStored;
      }
    } else {
      hasCover = !book.coverBmpPath.empty();
    }

    if (selected) {
      renderer.fillRoundedRect(coverX - 4, coverY - 4, coverWidth + 8, coverHeight + 8, cornerRadius,
                               Color::LightGray);

      if (hasCover) {
        FsFile file;
        if (Storage.openFileForRead("HOME", coverBmpPath, file)) {
          Bitmap bitmap(file);
          if (bitmap.parseHeaders() == BmpReaderError::Ok) {
            float aspect = (float)bitmap.getWidth() / bitmap.getHeight();
            int maxWidth = rect.width - 12;
            coverWidth = std::min(maxWidth, static_cast<int>(coverHeight * aspect));
            coverX = rect.x + (rect.width - coverWidth) / 2;
            renderer.fillRect(coverX, coverY, coverWidth, coverHeight, false);
            renderer.drawBitmap(bitmap, coverX, coverY, coverWidth, coverHeight);
          } else {
            hasCover = false;
          }
          file.close();
        } else {
          hasCover = false;
        }
      }

      if (!hasCover) {
        renderer.fillRect(coverX, coverY, coverWidth, coverHeight, false);
        renderer.drawIcon(CoverIcon, coverX + 24, coverY + 24, 32, 32);
      }

      renderer.drawRect(coverX, coverY, coverWidth, coverHeight, true);
    }

    const int textY = coverY + coverHeight + NewMetrics::values.verticalSpacing;
    const int textWidth = rect.width - 40;
    const int titleLinesLimit = book.author.empty() ? 2 : 1;
    auto titleLines = renderer.wrappedText(UI_12_FONT_ID, book.title.c_str(), textWidth, titleLinesLimit, EpdFontFamily::BOLD);
    auto author = renderer.truncatedText(UI_10_FONT_ID, book.author.c_str(), textWidth);

    int currentY = textY;
    for (const auto& line : titleLines) {
      const int textX = rect.x + (rect.width - renderer.getTextWidth(UI_12_FONT_ID, line.c_str(), EpdFontFamily::BOLD)) / 2;
      renderer.drawText(UI_12_FONT_ID, textX, currentY, line.c_str(), true, EpdFontFamily::BOLD);
      currentY += renderer.getLineHeight(UI_12_FONT_ID);
    }

    if (!book.author.empty()) {
      currentY += 2;
      const int textX = rect.x + (rect.width - renderer.getTextWidth(UI_10_FONT_ID, author.c_str(), EpdFontFamily::REGULAR)) / 2;
      renderer.drawText(UI_10_FONT_ID, textX, currentY, author.c_str(), true);
    }

    const int textHeight = currentY - textY;
    // Debug border for text layout
    // renderer.drawRect(coverX, coverY, coverWidth, coverHeight, true);
    // renderer.drawRect(rect.x + 20, textY - 1, textWidth, textHeight, true);
  } else {
    const int textWidth = rect.width - 40;
    //
    // renderer.drawRect(coverX, coverY, coverWidth, coverHeight, true);
    renderer.fillRect(coverX, coverY + (coverHeight / 3), coverWidth, 2 * coverHeight / 3, true);
    renderer.drawIcon(CoverIcon, coverX + 24, coverY + 24, 32, 32);
    const char* noBooksText = tr(STR_NO_RECENT_BOOKS);
    const int textX = rect.x + (rect.width - renderer.getTextWidth(UI_12_FONT_ID, noBooksText, EpdFontFamily::REGULAR)) / 2;
    renderer.drawText(UI_12_FONT_ID, textX, coverY + coverHeight + NewMetrics::values.verticalSpacing, noBooksText, true);
    // Debug border for cover layout
    // renderer.drawRect(coverX, coverY, coverWidth, coverHeight, true);
    // Debug border for text layout
    // renderer.drawRect(rect.x + 20, coverY + coverHeight + NewMetrics::values.verticalSpacing - 1,
                    //   textWidth, renderer.getLineHeight(UI_12_FONT_ID) + 2, true);
  }
}

void NewTheme::drawButtonMenu(GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
                              const std::function<std::string(int index)>& buttonLabel,
                              const std::function<UIIcon(int index)>& rowIcon) const {
  const int iconSize = mainMenuIconSize;
  const int spacing = 24;
  const int totalWidth = buttonCount * iconSize + (buttonCount - 1) * spacing;
  const int startX = rect.x + (rect.width - totalWidth) / 2;
  const int y = rect.y + 10;

  // Draw a subtle container so buttons are clearly visible below the cover.
//   renderer.drawLine(rect.x + 8, rect.y, rect.x + rect.width - 8, rect.y);
//   renderer.drawRect(rect.x + 4, rect.y, rect.width - 8, rect.height, true);

  for (int i = 0; i < buttonCount; ++i) {
    const int x = startX + i * (iconSize + spacing);
    const bool selected = selectedIndex == i;
    if (selected) {
      renderer.fillRoundedRect(x - 12, y - 12, iconSize + 24, iconSize + 24, cornerRadius, Color::LightGray);
    }

    if (rowIcon != nullptr) {
      UIIcon icon = rowIcon(i);
      const uint8_t* iconBitmap = iconForName(icon, iconSize);
      if (iconBitmap != nullptr) {
        renderer.drawIcon(iconBitmap, x, y, iconSize, iconSize);
      }
    }
    // Debug Borders for button layout
    // renderer.drawRect(x, y, iconSize, iconSize, true);
  }
}