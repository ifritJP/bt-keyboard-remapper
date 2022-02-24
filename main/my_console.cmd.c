//============== test
static struct {
    struct arg_str * pHoge;
    
    CONSOLE_DECL_ARG_TERM;
} s_console_test_arg;
static const console_command_arg_t s_test_argInfo[] = {
    CONSOLE_ARG_STR_OP( "test-type", "glossary" ),
    
};

//============== myver
static struct {
    CONSOLE_DECL_ARG_TERM;
} s_console_arg_myver;
static const console_command_arg_t s_argInfo_myver[] = {
};

//============== state
static struct {
    CONSOLE_DECL_ARG_TERM;
} s_console_arg_state;
static const console_command_arg_t s_argInfo_state[] = {
};

//============== config
static struct {
    struct arg_str * pMode;
    struct arg_str * pDevMode;
    struct arg_int * pDemo;
    
    
    CONSOLE_DECL_ARG_TERM;
    
} s_console_arg_config;
static const console_command_arg_t s_argInfo_config[] = {
    CONSOLE_ARG_STR_OP_SET( "mode", "m", NULL, "set the mode. 'normal' or 'setup'" ),
    CONSOLE_ARG_STR_OP_SET( "hidDevMode", "h", NULL, "set the hid dev mode. 'bt' or 'le'" ),
    CONSOLE_ARG_INT_OP_SET( "demo <0 or 1>", NULL, "demo", "set the demo mode 0 or 1" ),
};

//============== wifi
static struct {
    struct arg_str * pCryptKey;
    struct arg_str * pSsid;
    struct arg_str * pPass;
    struct arg_lit * pDump;
    struct arg_lit * pClear;
    struct arg_lit * pOta;
    struct arg_str * pOtaAuthB64;
    
    CONSOLE_DECL_ARG_TERM;
} s_console_arg_wifi;
static const console_command_arg_t s_argInfo_wifi[] = {
    CONSOLE_ARG_STR_OP_SET( "cryptKey", "k", NULL, "crypt key" ),
    CONSOLE_ARG_STR_OP_SET( "ssid", "s", NULL, "ssid" ),
    CONSOLE_ARG_STR_OP_SET( "pass", "p", NULL, "password" ),
    CONSOLE_ARG_FLAG_OP_SET( "dump", "d", NULL, "dump" ),
    CONSOLE_ARG_FLAG_OP_SET( "clear", "c", NULL, "clear" ),
    CONSOLE_ARG_FLAG_OP_SET( "ota", "o", NULL, "start otad" ),
    CONSOLE_ARG_STR_OP_SET( "otaAuthB64", "A", NULL, "ota auth base64" ),
};

//============== bt
static struct {
    struct arg_int * pConnectToDevice;
    struct arg_lit * pChannel;
    struct arg_int * pConnectDevice;
    struct arg_lit * pDiscoverable;
    struct arg_lit * pUndiscoverable;
    struct arg_str * pSendKey;
    struct arg_lit * pInitDevice;
    struct arg_lit * pInitHost;
    struct arg_lit * pListConns;
    struct arg_lit * pPairedDevices;
    struct arg_str * pScan;
    struct arg_str * pUnpair;
    struct arg_str * pPasskey;
    
    
    CONSOLE_DECL_ARG_TERM;
} s_console_arg_bt_dev;
static const console_command_arg_t s_argInfo_bt_dev[] = {
    CONSOLE_ARG_INT_OP_SET(
        "dev-id", "s", NULL, "connect to device" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "channel", "c", NULL, "dump channel" ),
    CONSOLE_ARG_INT_OP_SET(
        "host-id", "d", NULL, "connect as device" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "discoverable", "D", NULL, "set discoverable as device" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "undiscoverable", "F", NULL, "clear discoverable as device" ),
    CONSOLE_ARG_STR_OP_SET(
        "sendkey", NULL, "sendkey", "send key as device" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "initDevice", NULL, "initdev", "init as device" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "initHost", NULL, "inithost", "init as host" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "list", "l", NULL, "list connections" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "paired devices", "p", NULL, "paired devices" ),
    CONSOLE_ARG_STR_OP_SET(
        "on or off or now", NULL, "scan", "scan devices. on or off" ),
    CONSOLE_ARG_STR_OP_SET(
        "addr or 'all'", NULL, "unpair", "remove pair." ),
    CONSOLE_ARG_STR_OP_SET(
        "passkey'", NULL, "passkey", "send passkey for pairing." ),

};

//============== remap
static struct {
    struct arg_lit * pUpload;
    struct arg_lit * pDump;
    struct arg_str * pKey;
    struct arg_str * pConv;
    struct arg_lit * pSave;
    struct arg_lit * pLoad;
    struct arg_lit * pClear;
    struct arg_lit * pDump64;
    struct arg_lit * pLoad64;
    
    
    CONSOLE_DECL_ARG_TERM;
} s_console_arg_remap;
static const console_command_arg_t s_argInfo_remap[] = {
    CONSOLE_ARG_FLAG_OP_SET(
        "upload", "u", NULL, "upload the remap data" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "dump", "p", NULL, "dump the remap" ),
    CONSOLE_ARG_STR_OP_SET(
        "old,new", "k", NULL, "set remap key. old,new. e.g. a->b: 4,5  a->z: 4,29" ),
    CONSOLE_ARG_STR_OP_SET(
        "code,mask,result,code,xor", "c", NULL,
        "set convert key. e.g. S-SPC->1: 44,2,2,30,2" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "save", "s", NULL, "save the remap" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "load", "l", NULL, "load the remap" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "clear", NULL, "clear", "clear the remap" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "base64dump", NULL, "b64dump", "dump the remap as base64" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "base64read", NULL, "b64read", "read the remap as base64" ),
};


//===================
static const console_command_t s_commandInfo [] = {
    {
        .name = "test",
        .help = "test help",
        .func = console_test_command,
        .argNum = CONSOLE_ARG_LEN( s_test_argInfo ),
        .argInfoList = s_test_argInfo,
        .pArgStrust = &s_console_test_arg,
    },
    {
        .name = "myver",
        .help = "display my version",
        .func = console_myver_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_myver ),
        .argInfoList = s_argInfo_myver,
        .pArgStrust = &s_console_arg_myver,
    },
    {
        .name = "state",
        .help = "state",
        .func = console_state_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_state ),
        .argInfoList = s_argInfo_state,
        .pArgStrust = &s_console_arg_state,
    },
    {
        .name = "config",
        .help = "config",
        .func = console_config_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_config ),
        .argInfoList = s_argInfo_config,
        .pArgStrust = &s_console_arg_config,
    },
    {
        .name = "wifi",
        .help = "set wifi",
        .func = console_wifi_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_wifi ),
        .argInfoList = s_argInfo_wifi,
        .pArgStrust = &s_console_arg_wifi,
    },
    {
        .name = "bt",
        .help = "control bluetooth device",
        .func = console_bt_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_bt_dev ),
        .argInfoList = s_argInfo_bt_dev,
        .pArgStrust = &s_console_arg_bt_dev,
    },
    {
        .name = "remap",
        .help = "remap the keyboard",
        .func = console_remap_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_remap ),
        .argInfoList = s_argInfo_remap,
        .pArgStrust = &s_console_arg_remap,
    }
};
