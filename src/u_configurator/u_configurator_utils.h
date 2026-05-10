/*
* u_configurator is © 2026, Anton Granitov (onen-touw), BSTU Voenmeh
*
* u_configurator is published and distributed under 
* the Academic Software License v1.0 (ASL).
*
* u_configurator is distributed in the hope that it will be useful 
* for non-commercial academic research, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the ASL for more details.
*
* You should have received a copy of the ASL along with this program; 
* if not, write to anton.granitov123@gmail.com or https://github.com/onen-touw.  
* It is also published at LICENSE.md in root folder of this repository.
*
* You may contact the original licensor at anton.granitov123@gmail.com or https://github.com/onen-touw.
*/

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