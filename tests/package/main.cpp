#ifdef LFQUICK_SMOKE_MODELS
#include <LfQuick/Models/ObjectListModel.h>
#endif

#ifdef LFQUICK_SMOKE_IO
#include <LfQuick/Io/FileDocument.h>
#endif

int main()
{
#ifdef LFQUICK_SMOKE_MODELS
    LfQuick::ObjectListModel model;
    if (model.rowCount() != 0)
        return 1;
#endif

#ifdef LFQUICK_SMOKE_IO
    LfQuick::FileDocument document;
    if (document.dirty())
        return 2;
#endif

    return 0;
}
