#pragma once

#include <LfQuick/Platform/ClipboardService.h>
#include <LfQuick/Platform/FileWatcher.h>
#include <LfQuick/Platform/HotkeyRouter.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/InputTarget.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/MockScanner.h>
#include <LfQuick/Platform/RuntimeCapabilities.h>
#include <LfQuick/Platform/ScannerSession.h>
#include <LfQuick/Platform/ScreenService.h>
#include <LfQuick/Platform/SystemAppearance.h>

#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick::PlatformQml {

struct InputRouterForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::InputRouter)
    QML_NAMED_ELEMENT(InputRouter)
};

struct InputTargetForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::InputTarget)
    QML_NAMED_ELEMENT(InputTarget)
};

struct ScannerSessionForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::ScannerSession)
    QML_NAMED_ELEMENT(ScannerSession)
    QML_UNCREATABLE("ScannerSession is owned by a scanner input source")
};

struct KeyboardWedgeScannerForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::KeyboardWedgeScanner)
    QML_NAMED_ELEMENT(KeyboardWedgeScanner)
};

struct MockScannerForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::MockScanner)
    QML_NAMED_ELEMENT(MockScanner)
};

struct HotkeyRouterForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::HotkeyRouter)
    QML_NAMED_ELEMENT(HotkeyRouter)
};

struct SystemAppearanceForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::SystemAppearance)
    QML_NAMED_ELEMENT(SystemAppearance)
};

struct ScreenServiceForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::ScreenService)
    QML_NAMED_ELEMENT(ScreenService)
};

struct ClipboardServiceForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::ClipboardService)
    QML_NAMED_ELEMENT(ClipboardService)
};

struct FileWatcherForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::FileWatcher)
    QML_NAMED_ELEMENT(FileWatcher)
};

struct RuntimeCapabilitiesForeign
{
    Q_GADGET
    QML_FOREIGN(LfQuick::RuntimeCapabilities)
    QML_NAMED_ELEMENT(RuntimeCapabilities)
};

} // namespace LfQuick::PlatformQml
