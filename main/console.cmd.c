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
    struct arg_lit * pBle;
    struct arg_lit * pListConns;
    struct arg_lit * pPairedDevices;
    struct arg_str * pScan;
    struct arg_str * pUnpair;
    
    
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
        "ble", NULL, "ble", "set ble mode" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "list", "l", NULL, "list connections" ),
    CONSOLE_ARG_FLAG_OP_SET(
        "paired devices", "p", NULL, "paired devices" ),
    CONSOLE_ARG_STR_OP_SET(
        "on or off or now", NULL, "scan", "scan devices. on or off" ),
    CONSOLE_ARG_STR_OP_SET(
        "addr or 'all'", NULL, "unpair", "remove pair." ),

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
        .help = "display my version",
        .func = console_state_command,
        .argNum = CONSOLE_ARG_LEN( s_argInfo_state ),
        .argInfoList = s_argInfo_state,
        .pArgStrust = &s_console_arg_state,
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
    }
};
