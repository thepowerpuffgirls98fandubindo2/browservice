#include "control_bar.hpp"

#include "text.hpp"

namespace {

ImageSlice createSecureIcon() {
    return ImageSlice::createImageFromStrings({
        "WWWWWBBWWWWWW",
        "WWWWBWWBWWWWW",
        "WWWBWWWWBWWWW",
        "WWWBWWWWBWWWW",
        "WWWBWWWWBWWWW",
        "WEEEEEEEEEBWW",
        "WEGGGGGGGGBWW",
        "WEGGGGGGGGBWW",
        "WEGGGGGGGGBWW",
        "WEGGGGGGGGBWW",
        "WEGGGGGGGGBWW",
        "WBBBBBBBBBBWW",
        "WWWWWWWWWWWWW",
    }, {
        {'B', {0, 0, 0}},
        {'E', {128, 128, 128}},
        {'G', {192, 192, 192}},
        {'W', {255, 255, 255}},
    });
}
ImageSlice createWarningIcon() {
    return ImageSlice::createImageFromStrings({
        "WWWWWBBWvWWWW",
        "WWWWBWWvYjWWW",
        "WWWBWWWvYjWWW",
        "WWWBWWvYYYjWW",
        "WWWBWWvYBYjWW",
        "WEEEEEvYBYjWW",
        "WEGGGvYYBYYjW",
        "WEGGGvYYBYYjW",
        "WEGGGvYYBYYjW",
        "WEGGvYYYYYYYj",
        "WEGGvYYYBYYYj",
        "WBBBvYYYYYYYj",
        "WWWWyyyyyyyyy",
    }, {
        {'B', {0, 0, 0}},
        {'E', {128, 128, 128}},
        {'G', {192, 192, 192}},
        {'W', {255, 255, 255}},
        {'Y', {255, 255, 0}},
        {'y', {32, 32, 0}},
        {'j', {64, 64, 0}},
        {'v', {128, 128, 0}},
    });
}
ImageSlice createInsecureIcon() {
    return ImageSlice::createImageFromStrings({
        "WWWWWBBWWWRRW",
        "WWWWBWWBWRRRW",
        "WWWBWWWWRRRWW",
        "WWWBWWWRRRWWW",
        "WWWBWWRRRWWWW",
        "WEEEERRRWbBWW",
        "WEGGRRRWgGBWW",
        "WEGRRRWgGGBWW",
        "WERRRWgGGGBWW",
        "WRRRWgGGGGBWW",
        "RRRWgGGGGGBWW",
        "RRWbBBBBBBBWW",
        "WWWWWWWWWWWWW",
    }, {
        {'B', {0, 0, 0}},
        {'b', {128, 128, 128}},
        {'E', {128, 128, 128}},
        {'G', {192, 192, 192}},
        {'g', {224, 224, 224}},
        {'W', {255, 255, 255}},
        {'R', {255, 0, 0}},
    });
}

ImageSlice secureIcon = createSecureIcon();
ImageSlice warningIcon = createWarningIcon();
ImageSlice insecureIcon = createInsecureIcon();

ImageSlice securityStatusIcon(SecurityStatus status) {
    if(status == SecurityStatus::Secure) {
        return secureIcon;
    } else if(status == SecurityStatus::Warning) {
        return warningIcon;
    } else if(status == SecurityStatus::Insecure) {
        return insecureIcon;
    }
    CHECK(false);
    return insecureIcon;
}

}

struct ControlBar::Layout {
    Layout(int width) : width(width) {
        int contentStart = 1;
        int contentEnd = width - 1;

        const int SeparatorWidth = 4;
        const int AddressTextWidth = 52;
        const int QualityTextWidth = 46;

        qualitySelectorEnd = contentEnd;
        qualitySelectorStart = qualitySelectorEnd - QualitySelector::Width;

        qualityTextEnd = qualitySelectorStart;
        qualityTextStart = qualityTextEnd - QualityTextWidth;

        int separatorEnd = qualityTextStart;
        int separatorStart = separatorEnd - SeparatorWidth;
        separatorPos = separatorStart + SeparatorWidth / 2;

        int addrStart = contentStart;
        int addrEnd = separatorStart;

        addrTextStart = addrStart;
        addrTextEnd = addrTextStart + AddressTextWidth;

        addrBoxStart = addrTextEnd;
        addrBoxEnd = addrEnd;

        int addrBoxInnerStart = addrBoxStart + 2;
        int addrBoxInnerEnd = addrBoxEnd - 2;

        securityIconStart = addrBoxInnerStart + 4;
        int securityIconEnd = securityIconStart + 13;

        addrFieldStart = securityIconEnd + 4;
        addrFieldEnd = addrBoxInnerEnd;
    }

    int width;

    int addrTextStart;
    int addrTextEnd;

    int addrBoxStart;
    int addrBoxEnd;

    int securityIconStart;

    int addrFieldStart;
    int addrFieldEnd;

    int separatorPos;

    int qualityTextStart;
    int qualityTextEnd;

    int qualitySelectorStart;
    int qualitySelectorEnd;
};

ControlBar::ControlBar(CKey,
    weak_ptr<WidgetParent> widgetParent,
    weak_ptr<ControlBarEventHandler> eventHandler
)
    : Widget(widgetParent)
{
    CEF_REQUIRE_UI_THREAD();

    eventHandler_ = eventHandler;

    addrText_ = TextLayout::create();
    addrText_->setText("Address");

    qualityText_ = TextLayout::create();
    qualityText_->setText("Quality");

    securityStatus_ = SecurityStatus::Insecure;

    // Initialization is completed in afterConstruct_
}

void ControlBar::setSecurityStatus(SecurityStatus value) {
    CEF_REQUIRE_UI_THREAD();

    if(securityStatus_ != value) {
        securityStatus_ = value;
        signalViewDirty_();
    }
}

void ControlBar::setAddress(string addr) {
    CEF_REQUIRE_UI_THREAD();
    addrField_->setText(move(addr));
}

void ControlBar::onTextFieldSubmitted(string text) {
    CEF_REQUIRE_UI_THREAD();
    postTask(eventHandler_, &ControlBarEventHandler::onAddressSubmitted, text);
}

void ControlBar::onQualityChanged(int quality) {
    CEF_REQUIRE_UI_THREAD();
    postTask(eventHandler_, &ControlBarEventHandler::onQualityChanged, quality);
}

void ControlBar::afterConstruct_(shared_ptr<ControlBar> self) {
    addrField_ = TextField::create(self, self);
    qualitySelector_ = QualitySelector::create(self, self);
}

ControlBar::Layout ControlBar::layout_() {
    return Layout(getViewport().width());
}

void ControlBar::widgetViewportUpdated_() {
    CEF_REQUIRE_UI_THREAD();

    ImageSlice viewport = getViewport();
    Layout layout = layout_();

    addrField_->setViewport(viewport.subRect(
        layout.addrFieldStart, layout.addrFieldEnd, 5, Height - 6
    ));
    qualitySelector_->setViewport(viewport.subRect(
        layout.qualitySelectorStart, layout.qualitySelectorEnd, 1, Height - 4
    ));
}

void ControlBar::widgetRender_() {
    CEF_REQUIRE_UI_THREAD();

    ImageSlice viewport = getViewport();
    Layout layout = layout_();

    // Frame
    viewport.fill(0, layout.width - 1, 0, 1, 255);
    viewport.fill(0, 1, 1, Height - 4, 255);
    viewport.fill(layout.width - 1, layout.width, 0, Height - 3, 128);
    viewport.fill(0, layout.width - 1, Height - 4, Height - 3, 128);
    viewport.fill(0, layout.width, Height - 3, Height - 2, 255);
    viewport.fill(0, layout.width, Height - 2, Height - 1, 128);
    viewport.fill(0, layout.width, Height - 1, Height, 0, 0, 0);

    // Background
    viewport.fill(1, layout.width - 1, 1, Height - 4, 192);

    // "Address" text
    addrText_->render(
        viewport.subRect(layout.addrTextStart, layout.addrTextEnd, 1, Height - 4),
        3, 4
    );

    // Address field frame
    viewport.fill(layout.addrBoxStart, layout.addrBoxEnd - 1, 1, 2, 128);
    viewport.fill(layout.addrBoxStart, layout.addrBoxStart + 1, 2, Height - 5, 128);
    viewport.fill(layout.addrBoxEnd - 1, layout.addrBoxEnd, 1, Height - 4, 255);
    viewport.fill(layout.addrBoxStart, layout.addrBoxEnd - 1, Height - 5, Height - 4, 255);
    viewport.fill(layout.addrBoxStart + 1, layout.addrBoxEnd - 2, 2, 3, 0);
    viewport.fill(layout.addrBoxStart + 1, layout.addrBoxStart + 2, 3, Height - 6, 0);
    viewport.fill(layout.addrBoxEnd - 2, layout.addrBoxEnd - 1, 2, Height - 5, 192);
    viewport.fill(layout.addrBoxStart + 1, layout.addrBoxEnd - 2, Height - 6, Height - 5, 192);

    // Address field background
    viewport.fill(layout.addrBoxStart + 2, layout.addrBoxEnd - 2, 3, Height - 6, 255);

    // Security icon
    viewport.putImage(securityStatusIcon(securityStatus_), layout.securityIconStart, 6);

    // Separator
    viewport.fill(layout.separatorPos - 1, layout.separatorPos, 1, Height - 4, 128);
    viewport.fill(layout.separatorPos, layout.separatorPos + 1, 1, Height - 4, 255);

    // "Quality" text
    qualityText_->render(
        viewport.subRect(layout.qualityTextStart, layout.qualityTextEnd, 1, Height - 4),
        3, 4
    );
}

vector<shared_ptr<Widget>> ControlBar::widgetListChildren_() {
    CEF_REQUIRE_UI_THREAD();
    return {addrField_, qualitySelector_};
}
