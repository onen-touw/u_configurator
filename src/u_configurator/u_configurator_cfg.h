#pragma once

namespace u_configure
{
    enum class u_configure_cfg_falgs_e
    {
        enable_dynamic_configuration,   /// can you change smth via console or via code or only in file before upload
        enable_self_reconfiduration,    /// can user change this config that also stored in cfg-file or not
        auto_reboot_after_change,       /// do reboot automaticaly after write changes. ignore flag: requare_reboot_after_change
        requare_root_pass,              /// requare password for write new cfg
        enable_add,                     /// can you add option when reconfigurate system if it isnt in initial-config-file
    };

    enum class u_configurator_ack_t 
    {
        NO_ACCESS,              // if flag requare_root_pass set and user not call get_access(psw) or enter incorrect psw when he try to change smth 
        OP_ERROR,               // error
        OP_LOCKED,              // operation locked by config_cfg. e.g. enable_add not setted and user try to add cfg-line (see u_configure_cfg_falgs_e)
        OP_SUCCESS,             // all good 
        OP_FILE_ERROR,          // error that happens while work with file e.g. file no exists
        OP_DIR_ERROR,           // error that happens when cant open dir-cfg
        OP_FILE_RECOVERY,
        PARAM_DUBLICATION,      // say that user call reconfig(key, val) twise or more. In that case only first parameter will be registred and write in final config
        PARSING_ERROR,          // error in parse_cfg function
        USER_FUNCTION_ERROR,    // if one of registred user funcrion return bad result
    };
    
    struct u_configurator_cfg_t
    {
        static constexpr const char* cfg_dir = "/spiffs";
        static constexpr const char* cfg_file = "u_cfg.txt";
        static constexpr const char* cfg_file_def = "u_cfgdef.txt";
        static constexpr const char* temp_postfix = ".temp";
        static constexpr const char* old_postfix = ".old";
    };
}