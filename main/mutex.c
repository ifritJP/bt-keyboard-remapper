#include "mutex.h"

#include <freertos/semphr.h>


SMutex_t SMutex_create( void ) {
    SMutex_t mutex = xSemaphoreCreateMutex();
    xSemaphoreGive( mutex );
    return mutex;
}

void SMutex_lock( SMutex_t mutex ) {
    while ( true ) {
        if ( xSemaphoreTake( mutex, 100000 ) == pdPASS ) {
            return;
        }
    }
}

void SMutex_unlock( SMutex_t mutex ) {
    xSemaphoreGive( mutex );
}


SChan_t SChan_create( void ) {
    return SQueue_create(1,1);
}

void SChan_get( SChan_t chan ) {
    uint8_t val;
    SQueue_get( chan, &val );
}

void SChan_put( SChan_t chan ) {
    uint8_t val = 1;
    SQueue_put( chan, &val );
}


SQueue_t SQueue_create( int len, int size ) {
    return xQueueCreate( len, size );
}

void SQueue_get( SQueue_t chan, void * const pvBuffer ) {
    while ( true ) {
        if ( xQueueReceive( chan, pvBuffer, 100000 ) == pdPASS ) {
            return;
        }
    }
}

void SQueue_put( SQueue_t chan, const void * pData ) {
    while ( true ) {
        if ( xQueueSendToBack( chan, pData, 100000 ) == pdPASS ) {
            return;
        }
    }
}

void SQueue_put_isr( SQueue_t chan, const void * pData ) {
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xQueueSendToBackFromISR( chan, pData, &xHigherPriorityTaskWoken );
}
