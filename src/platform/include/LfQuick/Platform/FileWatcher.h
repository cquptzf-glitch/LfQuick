#pragma once

// Compatibility header. FileWatcher moved to the technology-neutral Io module.
// Existing source that includes <LfQuick/Platform/FileWatcher.h> keeps working,
// while new code should include <LfQuick/Io/FileWatcher.h> and link LfQuick::Io.
#include <LfQuick/Io/FileWatcher.h>
