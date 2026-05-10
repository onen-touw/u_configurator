#pragma once

#include "u_configurator_cfg.h"
#include "u_configurator_base.h"

namespace u_configure
{
    class u_reconfigurator_t : public u_configurator_base_t
    {
    private:
        enum class internal_e 
        {
            // !!!no more than 15 and no less than max(u_configure_cfg_falgs_e)
            access = 10,
            changed = 11,
        };
        
    private:
        using module_clb_t = bool(*)(const parser_t&);
        using reboot_clb_t = bool(*)(void);
        
    public:
        using paths_t = u_configurator_cfg_t;

    private:
        static constexpr const char* scfg_psw = "322";

    private:
        std::filesystem::path path_to_cfg;
        ufo::bit_flag_t<uint16_t> _cfg_flags;
        std::map<std::string, std::string> _rcfg;
        
        reboot_clb_t _reboot_callback = nullptr;

    public:
        u_reconfigurator_t()
        {
            _cfg_flags = ufo::bit_flag_t<uint16_t>(
                // u_configure_cfg_falgs_e::auto_reboot_after_change,
                u_configure_cfg_falgs_e::enable_dynamic_configuration,
                // u_configure_cfg_falgs_e::enable_add,
                // u_configure_cfg_falgs_e::enable_self_reconfiduration,
                u_configure_cfg_falgs_e::requare_root_pass);

            path_to_cfg = paths_t::cfg_dir;
            path_to_cfg /= paths_t::cfg_file;

        }
        ~u_reconfigurator_t() {}

        template<typename... Args>
        u_configurator_ack_t self_reconfig(Args... flags)
        {
            if (!_cfg_flags.get(u_configure_cfg_falgs_e::enable_self_reconfiduration))
            {
                return u_configurator_ack_t::OP_LOCKED;
            }

            ufo::bit_flag_t<uint16_t> new_self_cfg(flags...);
            auto res = reconfig("s-cfg-flg", std::to_string(new_self_cfg.get()));
            return res;
        }

        void set_reboot_clb(reboot_clb_t clb) { _reboot_callback = clb; }

        bool is_psw_requared() const
        {
            return _cfg_flags.get(u_configure_cfg_falgs_e::requare_root_pass);
        }

        bool is_add_enabled() const
        {
            return _cfg_flags.get(u_configure_cfg_falgs_e::enable_add);
        }

        bool is_reconfiguration_enabled() const
        {
            return _cfg_flags.get(u_configure_cfg_falgs_e::enable_dynamic_configuration);
        }

        bool is_autoreboot_enabled() const
        {
            return _cfg_flags.get(u_configure_cfg_falgs_e::auto_reboot_after_change);
        }

        bool is_self_modication_enabled() const
        {
            return _cfg_flags.get(u_configure_cfg_falgs_e::enable_self_reconfiduration);
        }

        const auto& get_changes() const 
        {
            return _rcfg;
        }

        bool get_access(const char* psw) 
        {
            if (psw)
            {
                if (!strcmp(psw, scfg_psw))
                {
                    _cfg_flags.set(internal_e::access);
                    return true;
                }
            }
            _cfg_flags.unset(internal_e::access);
            return false;
        }

        /// @brief 
        /// @param psw 
        /// @return OP_ERROR if reboot_callback return false, NO_ACCESS if incorrect psw OP_FILE_ERROR if cant create new cfg-file
        u_configurator_ack_t apply_config()
        {
            std::filesystem::path temp_file_name = paths_t::cfg_file;
            temp_file_name += paths_t::temp_postfix;

            auto result = write_new_cfg(paths_t::cfg_dir / temp_file_name);
            if (result != u_configurator_ack_t::OP_SUCCESS)
            {
                return result;
            }

            {
                ufo::udir_t check_dir(paths_t::cfg_dir);
                if (!check_dir.is_valid())
                {
                    return u_configurator_ack_t::OP_DIR_ERROR;
                }

                bool fop_res = check_dir.file_exists(temp_file_name);
                if (!fop_res)
                {
                    return u_configurator_ack_t::OP_FILE_ERROR;
                }

                fop_res = check_dir.file_exists(paths_t::cfg_file);
                if (!fop_res)
                {
                    return u_configurator_ack_t::OP_FILE_ERROR;
                }

                std::filesystem::path old_file_name = paths_t::cfg_file;
                old_file_name += paths_t::old_postfix;

                if (check_dir.file_exists(old_file_name))
                {
                    fop_res = check_dir.remove_file(old_file_name);
                    if (!fop_res)
                    {
                        return u_configurator_ack_t::OP_FILE_ERROR;
                    }
                }

                fop_res = check_dir.rename_file(paths_t::cfg_file, old_file_name);
                if (!fop_res)
                {
                    // if old _cfg_file stayed there and wasnt renamed it is good result if error case, so just return error 
                    return u_configurator_ack_t::OP_FILE_ERROR;
                }

                fop_res = check_dir.rename_file(temp_file_name, paths_t::cfg_file);
                if (!fop_res)
                {
                    // recover config that storred in _parser. is it true behavior?
                    // if (check_dir.file_exists(_cfg_file))
                    {
                        return _recover_from_parser();
                    }
                    return u_configurator_ack_t::OP_FILE_ERROR;
                }
            }
            
            // all files should be closed
            if (_cfg_flags.get(u_configure_cfg_falgs_e::auto_reboot_after_change))
            {
                if (_reboot_callback)
                {
                    if (!_reboot_callback())
                    {
                        return u_configurator_ack_t::OP_ERROR;
                    }
                }
            }

            // never reach if auto reboot
            return u_configurator_ack_t::OP_SUCCESS;
        }

        void reset() { 
            _rcfg.clear(); 
            _cfg_flags.unset(internal_e::changed);
            _cfg_flags.unset(internal_e::access);
        }
        
        void cancel()
        {
            reset();
        }

        u_configurator_ack_t reconfig(std::string&& key, std::string&& value)
        {
            if (!is_reconfiguration_enabled())
            {
                return u_configurator_ack_t::OP_LOCKED;
            }
            
            auto pres = parse_cfg(path_to_cfg);

            if (pres != u_configurator_ack_t::OP_SUCCESS)
            {
                // std::cout << "cant read current config file\n";
                return pres;
            }

            auto opt_list = _parser.get_native_options();
            auto opt = opt_list.find(key);

            if (opt != opt_list.end())
            {
                auto res = _rcfg.emplace(key, value);
                
                if (!res.second)
                {
                    return u_configurator_ack_t::PARAM_DUBLICATION;
                }
                _cfg_flags.set(internal_e::changed);
                return u_configurator_ack_t::OP_SUCCESS;
            }
            else
            {
                if (is_add_enabled())
                {
                    auto res = _rcfg.emplace(key, value);
                    if (!res.second)
                    {
                        return u_configurator_ack_t::PARAM_DUBLICATION;
                    }
                    _cfg_flags.set(internal_e::changed);

                    return u_configurator_ack_t::OP_SUCCESS;
                }
                return u_configurator_ack_t::OP_LOCKED;
            }
            return u_configurator_ack_t::OP_ERROR;
        }

        u_configurator_ack_t write_new_cfg(const std::filesystem::path& path)
        {
            if (!_is_changed())
            {
                return u_configurator_ack_t::OP_SUCCESS;
            }
            
            ufo::ufile_t out_file(path, ufo::file_mode::write_wb);

            if (!out_file.is_open())
            {
                return u_configurator_ack_t::OP_FILE_ERROR;
            }

            const auto& old_params = _parser.get_native_options();

            for (const auto &old_item : old_params)
            {
                auto fitem = _rcfg.find(old_item.first);
                if (fitem == _rcfg.end())
                {
                    out_file << '-' << old_item.first << '=' << old_item.second << '\n';
                }
                else    
                {
                    out_file << '-' << fitem->first << '=' << fitem->second << '\n';
                }
            }

            for (const auto& [key, value] : _rcfg) {
                if (old_params.find(key) == old_params.end()) {
                    out_file << '-' << key << '=' << value << '\n';
                }
            }
            return u_configurator_ack_t::OP_SUCCESS;
        }

        // swap current and old config
        u_configurator_ack_t recover_from_old()
        {
            ufo::udir_t check_dir(paths_t::cfg_dir);
            if (!check_dir.is_valid())
            {
                return u_configurator_ack_t::OP_DIR_ERROR;
            }

            std::filesystem::path old_name = paths_t::cfg_file;
            old_name += paths_t::old_postfix;
            
            bool fop_res = check_dir.file_exists(old_name);
            if (!fop_res)
            {
                return u_configurator_ack_t::OP_FILE_ERROR;
            }

            std::filesystem::path temp_name = paths_t::cfg_file;
            temp_name += paths_t::temp_postfix;

            fop_res = check_dir.file_exists(paths_t::cfg_file);
            if (fop_res)
            {
                check_dir.copy_file(paths_t::cfg_file, temp_name);
                check_dir.remove_file(paths_t::cfg_file);
            }

            check_dir.rename_file(old_name, paths_t::cfg_file);
            check_dir.rename_file(temp_name, old_name);

            return u_configurator_ack_t::OP_SUCCESS;
        }

        // swap current and old config
        u_configurator_ack_t reset_to_default()
        {
            ufo::udir_t check_dir(paths_t::cfg_dir);
            if (!check_dir.is_valid())
            {
                return u_configurator_ack_t::OP_DIR_ERROR;
            }

            // check if default cfg file exists
            auto fop_res = check_dir.file_exists(paths_t::cfg_file_def);
            if (!fop_res)
            {
                return u_configurator_ack_t::OP_FILE_ERROR;
            }
            
            // check if current exist
            fop_res = check_dir.file_exists(paths_t::cfg_file);
            if (!fop_res)
            {
                // if not exist just copy default and names current

                fop_res = check_dir.copy_file(paths_t::cfg_file_def, paths_t::cfg_file);
                return fop_res ? u_configurator_ack_t::OP_SUCCESS : u_configurator_ack_t::OP_FILE_ERROR;
            }
            
            // if exist we have to remove .old if exists and current set to .olg
            // than copy default to current    

            {
                std::filesystem::path old_name = paths_t::cfg_file;
                old_name += paths_t::old_postfix;

                // check and delete old
                fop_res = check_dir.file_exists(old_name);
                if (fop_res)
                {
                    fop_res = check_dir.remove_file(old_name);
                    if (!fop_res)
                    {
                        return u_configurator_ack_t::OP_FILE_ERROR;
                    }
                }

                // do current .old
                fop_res = check_dir.rename_file(paths_t::cfg_file, old_name);
                if (!fop_res)
                {
                    return u_configurator_ack_t::OP_FILE_ERROR;
                }
            }

            // do default current
            fop_res = check_dir.copy_file(paths_t::cfg_file_def, paths_t::cfg_file);
            return fop_res ? u_configurator_ack_t::OP_SUCCESS : u_configurator_ack_t::OP_FILE_ERROR;
        }

    private:
        u_configurator_ack_t _recover_from_parser()
        {
            ufo::ufile_t out_file(path_to_cfg, ufo::file_mode::write_wb);
            if (!out_file.is_open())
            {
                return u_configurator_ack_t::OP_FILE_ERROR;
            }

            std::string temp;

            const auto& params = _parser.get_native_options();
            for (const auto &item : params)
            {
                _parser.bind(&temp, item.first);
                out_file << '-' << item.first << '=' << temp << '\n';
            }

            return u_configurator_ack_t::OP_FILE_RECOVERY;
        }

        bool _is_changed() const { return _cfg_flags.get(internal_e::changed); }
        bool _is_access() const { return _cfg_flags.get(internal_e::access); }
    };

} // namespace u_configure