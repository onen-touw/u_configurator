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