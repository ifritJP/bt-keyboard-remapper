#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>

#define BUF_SIZE  512

#define TAG "MY_OTA"

#define DISP_UNIT (20 * 1024)

static void restart_task( void * pParam ) {
    int count = 3;
    for ( ; count > 0; count-- ) {
        printf( "reboot -- %d\n", count );
        vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
    esp_restart();
}

static char s_otaAuthB64[50] = "";

static bool processOta( esp_ota_handle_t otaHandle, httpd_req_t *req )
{
    int firmSize = req->content_len;
    ESP_LOGI(TAG, "firmware size: %d KB.", firmSize / 1024);
    
    int totalSize = 0;
    int remain = req->content_len;
    char buf[ BUF_SIZE ];
    int prevNotifySize = 0;
    while ( remain > 0 ) {
        int accessSize = sizeof( buf );
        if ( remain < accessSize ) {
            accessSize = remain;
        }
        int readSize = httpd_req_recv(req, buf, accessSize );
        if ( readSize <= 0 ) {
            ESP_LOGE( TAG, "failed to httpd_req_recv" );
            return false;
        }

        if ( esp_ota_write(otaHandle, buf, readSize) != ESP_OK ) {
            ESP_LOGE( TAG, "failed to esp_ota_write" );
            return false;
        };
 
        remain -= readSize;
        totalSize += readSize;
        if ( totalSize - prevNotifySize >= DISP_UNIT ) {
            httpd_resp_sendstr_chunk(req, ".");
            printf( "." );
            fflush( stdout );
            prevNotifySize = ( totalSize / DISP_UNIT ) *  DISP_UNIT;
        }
    }
    return true;
}
 
static esp_err_t http_handle_ota( httpd_req_t *req )
{
    ESP_LOGI(TAG, "start");
 
    ESP_ERROR_CHECK(
        httpd_resp_set_type(req, "text/plain" ) );

    if ( strcmp( s_otaAuthB64, "" ) != 0 ) {
        // basic 認証
        char authBuf[ 50 ];
        if ( httpd_req_get_hdr_value_str(
                 req, "Authorization", authBuf, sizeof( authBuf ) ) == ESP_OK )
        {
            char * pAuth = strchr( authBuf, ' ' );
            if ( pAuth != NULL ) {
                pAuth++;
            } else {
                pAuth = authBuf;
            }
            if ( strcmp( s_otaAuthB64, pAuth ) != 0 ) {
                ESP_LOGI(TAG, "unmatch" );
                httpd_resp_send_err( req, HTTPD_403_FORBIDDEN, NULL );
                return ESP_OK;
            }
            ESP_LOGI(TAG, "match" );
        }
        else {
            ESP_LOGI(TAG, "no auth" );
            httpd_resp_send_err( req, HTTPD_403_FORBIDDEN, NULL );
            return ESP_OK;
        }
    } else {
        ESP_LOGI(TAG, "free auth" );
    }
    
    
    ESP_ERROR_CHECK(
        httpd_resp_sendstr_chunk(req, "preparing ota...\n"));

    esp_ota_handle_t otaHandle;
    const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
    ESP_ERROR_CHECK(esp_ota_begin(part, req->content_len, &otaHandle));

    httpd_resp_sendstr_chunk(req, "start\n");

    bool result = processOta( otaHandle, req );
    if ( !result ) {
        ESP_LOGI(TAG, "NG" );
        httpd_resp_sendstr_chunk(req, "\nNG\n");
        ESP_ERROR_CHECK(esp_ota_abort( otaHandle ));
        httpd_resp_sendstr_chunk(req, NULL);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "OK" );
    httpd_resp_sendstr_chunk(req, "\nOK\n");
    ESP_ERROR_CHECK(esp_ota_end( otaHandle ));
    ESP_ERROR_CHECK(esp_ota_set_boot_partition( part ));
    ESP_LOGI(TAG, "Finish" );
    
    httpd_resp_sendstr_chunk(req, NULL);

    xTaskCreate(restart_task, "restart_task", 1024 * 2, NULL, 10, NULL);

    return ESP_OK;
}
 
static void ota_register_handler(httpd_handle_t server)
{
    httpd_uri_t uri = {
        .uri = "/ota/",
        .method = HTTP_POST,
        .handler = http_handle_ota,
    };
    
    ESP_ERROR_CHECK( httpd_register_uri_handler(server, &uri) );
 
    esp_ota_img_states_t ota_state;
    if ( esp_ota_get_state_partition(
             esp_ota_get_running_partition(), &ota_state ) == ESP_OK )
    {
        if ( ota_state == ESP_OTA_IMG_PENDING_VERIFY ) {
            ESP_ERROR_CHECK( esp_ota_mark_app_valid_cancel_rollback() );
        }
    }
}

void start_otad( const char * pAuthB64 ) {
    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 3;
    config.stack_size = 8 * 1024;

    strcpy( s_otaAuthB64, pAuthB64 );
    if ( strcmp( s_otaAuthB64, "" ) != 0 ) {
        ESP_LOGI( TAG, "OTA needs the BASIC AUTH" );
    }
 
    ESP_ERROR_CHECK(httpd_start( &server, &config ));
    ota_register_handler( server );
}
