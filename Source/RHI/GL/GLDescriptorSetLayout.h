
#include <algorithm>

#include "RHI/DriverEnums.h"

namespace RHI{
    
struct GLDescriptorSetLayout : public HwDescriptorSetLayout, public DescriptorSetLayout {
    using HwDescriptorSetLayout::HwDescriptorSetLayout;
    explicit GLDescriptorSetLayout(DescriptorSetLayout&& layout) noexcept
            : DescriptorSetLayout(std::move(layout)) {

        std::sort(bindings.begin(), bindings.end(),
                [](auto&& lhs, auto&& rhs){
            return lhs.binding < rhs.binding;
        });

        auto p = std::max_element(bindings.cbegin(), bindings.cend(),
                [](auto const& lhs, auto const& rhs) {
            return lhs.binding < rhs.binding;
        });
        maxDescriptorBinding = p->binding;
    }
    uint8_t maxDescriptorBinding = 0;
};
}
