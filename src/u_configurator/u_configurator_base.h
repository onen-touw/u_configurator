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

#include <string>
#include <filesystem>
#include <string.h>

#ifdef CONFIG_IDF_TARGET_ESP32

#include "u_cns/impl/Console-parser/parser.h"
#include "u_sys/btflg.h"
#include "uffs/udir.h"
#include "uffs/ufile.h"

#else

#include "parser.h"
#include "btflg.h"
#include "udir.h"
#include "ufile.h"

#endif


#include "u_configurator_cfg.h"


namespace u_configure
{
    class u_configurator_base_t
    {
    protected:
        using parser_t = Console_parser::parser;

        parser_t _parser;
        bool _readed = false;

    public:
        u_configurator_base_t() : _parser(true) {}
        ~u_configurator_base_t() {}

    protected:
        u_configurator_ack_t parse_cfg(const std::filesystem::path& cfg_file)
        {
            if (_readed)
            {
                return u_configurator_ack_t::OP_SUCCESS;
            }
            
            std::string buff;

            ufo::ufile_t file(cfg_file, ufo::file_mode::read_rb);

            if (!file.exists(cfg_file))
            {
                printf("1\n");
                return u_configurator_ack_t::OP_FILE_ERROR;
            }

            if (!file.is_open())
            {
                printf("2\n");
                return u_configurator_ack_t::OP_FILE_ERROR;
            }
            
            while (!file.eof())
            {
                std::string line;

                file.getline(line);

                if (!line.empty())
                {
                    if (line[0] == '-')
                    {
                        buff.append(line);
                        buff += ' ';
                    }
                }
            }

            _parser.parse(std::move(buff));

            if (_parser.good())
            {
                _readed = true;
                return u_configurator_ack_t::OP_SUCCESS;
            }
            return u_configurator_ack_t::PARSING_ERROR;
        }
    public:
        void parser_log()
        {
            _parser.log();
        }
    };
}
