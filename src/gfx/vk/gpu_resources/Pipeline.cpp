#include "Pipeline.h"

#include <fmt/format.h>

// Pipeline::Pipeline() {
//
// }

const VkPipeline& Pipeline::getUnderlying() const {
    return m_underlying;
}