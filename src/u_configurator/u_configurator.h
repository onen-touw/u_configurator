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

#include "u_configurator_base.h"

namespace u_configure
{

    class u_configurator_t : public u_configurator_base_t
    {
    public:
        using module_clb_t = void(*)(const parser_t&);
        
    private:
        std::vector<module_clb_t> _list;

    public:
        u_configurator_t(/* const std::string& cfg_file */) {}
        ~u_configurator_t() {}

        void register_clb(module_clb_t clb)
        {
            _list.push_back(clb);
        }

        u_configurator_ack_t configurate()
        {
            std::filesystem::path path = u_configurator_cfg_t::cfg_dir;
            path /=  u_configurator_cfg_t::cfg_file;

            auto res = parse_cfg(path);

            if (res != u_configurator_ack_t::OP_SUCCESS)
            {
                return res;
            }

            for (const auto &i : _list)
            {
                // auto pres = 
                i(_parser);
                // if (!pres)
                // {
                //     res = u_configurator_ack_t::USER_FUNCTION_ERROR;
                //     // and continue to work
                // }
            }

            return res;
        }
    };


    using configuration_module_callback_t = u_configurator_t::module_clb_t;
    using configuration_config_list_t = Console_parser::parser;
    
}