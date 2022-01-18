#include "console.h"
#include "mutex.h"
#include <string.h>
#include <ctype.h>

#define LEN_LINE_BUF 255
#define MAX_ARG 10

typedef struct {
    SMutex_t lockObj;
    SChan_t emptyEventChan;
    SChan_t existEventChan;
    SQueue_t inQueue;
    uint16_t workBufLen;
    uint8_t workBuf[ LEN_LINE_BUF + 1 ];
    uint8_t lineBuf[ LEN_LINE_BUF + 1 ];
    uint8_t argNum;
    char * argArray[ MAX_ARG ];
} Console_t;

static Console_t s_console = {};

static void Console_task( void * pParam );
static void Console_inputTask( void * pParam );

static char * SkipWhiteSpace( char * pTxt ) {
    while ( isspace( *pTxt ) ) {
        pTxt++;
    };
    if ( *pTxt == '\0' ) {
        return NULL;
    }
    return pTxt;
}

static void Console_lock( Console_t * pConsole ) {
    SMutex_lock( pConsole->lockObj );
}

static void Console_unlock( Console_t * pConsole ) {
    SMutex_unlock( pConsole->lockObj );
}

static void Console_setup( Console_t * pConsole ) {
    printf( "%s: handle = %p\n", __func__, xTaskGetCurrentTaskHandle() );
    
    pConsole->lockObj = SMutex_create();
    pConsole->emptyEventChan = SChan_create();
    pConsole->existEventChan = SChan_create();
    pConsole->inQueue = SQueue_create( 1, 10 );

    printf( "%s:start %p, %p, %p, %p\n", __func__,
            pConsole->lockObj,
            pConsole->emptyEventChan,
            pConsole->existEventChan,
            pConsole->inQueue );
    Console_lock( pConsole );
    Console_unlock( pConsole );
    SChan_put( pConsole->emptyEventChan );
    SChan_get( pConsole->emptyEventChan );
    SChan_put( pConsole->existEventChan );
    SChan_get( pConsole->existEventChan );
    uint8_t val = 0;
    SQueue_put( pConsole->inQueue, &val );
    SQueue_get( pConsole->inQueue, &val );
    printf( "%s:end\n", __func__ );


    // 最初はバッファが空いているので、 emptyEventChan をセットする
    SChan_put( pConsole->emptyEventChan );
}

void Console_init( void ) {
    Console_setup( &s_console );
    
    xTaskCreate( Console_task, "Console_task", 3 * 1024, NULL,
                 configMAX_PRIORITIES - 3, NULL );
    xTaskCreate( Console_inputTask, "Console_inputTask", 3 * 1024, NULL,
                 configMAX_PRIORITIES - 3, NULL );
    
}

static void Console_storeToLine( Console_t * pConsole, uint8_t crCode ) {
    // 空くのを待つ
    SChan_get( pConsole->emptyEventChan );
    
    memcpy( pConsole->lineBuf, pConsole->workBuf, LEN_LINE_BUF );
    pConsole->lineBuf[ pConsole->workBufLen ] = '\0';
    pConsole->workBufLen = 0;
    
    SChan_put( pConsole->existEventChan );
}

static void Console_split( Console_t * pConsole ) {
    char * pBuf = (char *)pConsole->lineBuf;
    for ( pConsole->argNum = 0; pConsole->argNum < MAX_ARG; )
    {
        pBuf = SkipWhiteSpace( pBuf );
        if ( pBuf == NULL ) {
            return;
        }
        pConsole->argArray[ pConsole->argNum ] = pBuf;
        pConsole->argNum++;
        
        pBuf = strchr( pBuf, ' ' );
        if ( pBuf == NULL ) {
            return;
        }
        *pBuf = '\0';
        pBuf++;
    }
}

static void Console_addChar( Console_t * pConsole, uint8_t code ) {
    if ( pConsole->workBufLen >= LEN_LINE_BUF ) {
        return;
    }
    pConsole->workBuf[ pConsole->workBufLen ] = code;
    pConsole->workBufLen++;
}

void Console_input( uint8_t code ) {
    SQueue_put_isr( s_console.inQueue, &code );
}

static void Console_inputTask( void * pParam ) {
    printf( "%s: handle = %p\n", __func__, xTaskGetCurrentTaskHandle() );
    
    Console_t * pConsole = &s_console;

    while ( true ) {
        uint8_t code = 0;
        SQueue_get( pConsole->inQueue, &code );
        if ( isgraph( code ) || code == ' ' || code == '\t' ) {
            printf( "%c", code );
        }
        //if ( code == '\r' || code == '\n' ) {
        if ( code == '\r' ) {
            printf( "\n" );
            Console_storeToLine( pConsole, code );
        } else {
            Console_addChar( pConsole, code );
        }
    }
}

static void Console_readLine( Console_t * pConsole, char * pBuf, int len ) {
    SChan_get( pConsole->existEventChan );

    strncpy( pBuf, (char *)pConsole->lineBuf, len - 1 );
    pBuf[ len ] = '\0';

                
    SChan_put( pConsole->emptyEventChan );
}

typedef void (Console_func_t)( const char * pArgs, int argLen );
typedef struct {
    Console_func_t * pFunc;
    char * command;
    char * description;
    
} Console_commandInfo_t;

static void Console_help( const char * pArgs, int argLen );
static void Console_reboot( const char * pArgs, int argLen );
static void Console_showDeviceInfo( const char * pArgs, int argLen );

static const Console_commandInfo_t s_commandInfoArray[] = {
    { Console_help, "help", "display help" },
    { Console_reboot, "reboot", "reboot device" },
    { Console_showDeviceInfo, "devinfo", "show device info" },
    { NULL, NULL }
};

static void Console_task( void * pParam ) {
    printf( "%s: handle = %p\n", __func__, xTaskGetCurrentTaskHandle() );

    Console_t * pConsole = &s_console;

    uint32_t lineCount = 0;
    
    while ( true ) {
        SChan_get( pConsole->existEventChan );

        Console_split( pConsole );
        /* int index; */
        /* for ( index = 0; index < pConsole->argNum; index++ ) { */
        /*     printf( "'%s' ", pConsole->argArray[ index ] ); */
        /* } */

        SChan_put( pConsole->emptyEventChan );

        if ( pConsole->argNum > 0 ) {
            int index;
            for ( index = 0; s_commandInfoArray[ index ].command != NULL; index++ ) {
                const Console_commandInfo_t * pInfo = &s_commandInfoArray[ index ];
                if ( strcmp( pInfo->command, pConsole->argArray[ 0 ] ) == 0 ) {
                    pInfo->pFunc( pConsole->argArray, pConsole->argNum );
                }
            }
        }
        
        /* if ( strcmp( pConsole->argArray[ 0 ], "ls" ) == 0 ) { */
        /*     printf( "\n" ); */
        /*     char line[ LEN_LINE_BUF ]; */
        /*     Console_readLine( pConsole, line, sizeof( line ) ); */
        /*     printf( "%s\n", line ); */
        /* } */

        printf( "\n%d$ ", lineCount );
        lineCount++;
    }
}

static void Console_help( const char * pArgs, int argLen )
{
    int index;
    printf( "command list:\n" );
    printf( "\n" );
    for ( index = 0; s_commandInfoArray[ index ].command != NULL; index++ ) {
        const Console_commandInfo_t * pInfo = &s_commandInfoArray[ index ];
        printf( " %s: %s\n", pInfo->command, pInfo->description );
    }
}

static void Console_reboot_handler(void)
{
    printf( "========== reboot ========\n" );
}

static void Console_reboot( const char * pArgs, int argLen ) {
    esp_register_shutdown_handler( Console_reboot_handler );
    esp_restart();
}

static void Console_showDeviceInfo( const char * pArgs, int argLen ) {
    //heap_caps_dump_all();
    printf( "free heap info:\n"
            " heap = %d\n"
            " internal_heap = %d\n"
            " minimum_free = %d\n",
            esp_get_free_heap_size(), esp_get_free_internal_heap_size(),
            esp_get_minimum_free_heap_size() );
}
