#include <core/core.h>

QDebug operator<<(QDebug debug, const resultST &result)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    if (!result.success) {
        debug  << result.message << " : " << result.code ;
    }

    return debug;
}
