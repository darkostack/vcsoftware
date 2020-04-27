#ifndef CORE_LOCATOR_GETTERS_HPP
#define CORE_LOCATOR_GETTERS_HPP

#include "core/config.h"

#include "core/instance.hpp"
#include "core/locator.hpp"

namespace mt {

template <> inline Instance &InstanceLocator::Get(void) const
{
    return GetInstance();
}

template <typename Type> inline Type &InstanceLocator::Get(void) const
{
    return GetInstance().Get<Type>();
}

} // namespace mt

#endif /* CORE_LOCATOR_GETTERS_HPP */
