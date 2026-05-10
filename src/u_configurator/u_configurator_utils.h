#pragma once

#include <type_traits>

namespace u_configure
{

    /// @brief Limit v by lim if (v > lim) and return v casting to eTy
    /// @tparam eTy enum type of limiting value
    /// @tparam iTy integral type of raw value
    /// @param v raw value
    /// @param lim limit-value
    /// @return limited eTy-value
    template<typename eTy, typename iTy, std::enable_if_t<sizeof(eTy) == sizeof(iTy), bool> = true>
    eTy check_ei_field(iTy v, eTy lim)
    {
        uint8_t ilim = static_cast<iTy>(lim);
        if (v > ilim)
        {
            v = ilim;
        }
        return static_cast<eTy>(v);
    }
    
}