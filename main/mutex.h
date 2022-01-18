#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef QueueHandle_t SMutex_t;

extern SMutex_t SMutex_create( void );
extern void SMutex_lock( SMutex_t mutex );
extern void SMutex_unlock( SMutex_t mutex );

typedef QueueHandle_t SChan_t;
extern SChan_t SChan_create( void );
extern void SChan_get( SChan_t chan );
extern void SChan_put( SChan_t chan );
extern void SChan_put_isr( SChan_t chan );

typedef QueueHandle_t SQueue_t;
extern SQueue_t SQueue_create( int len, int size );
extern void SQueue_get( SQueue_t chan, void * const pvBuffer );
extern void SQueue_put( SQueue_t chan, const void * pData );
extern void SQueue_put_isr( SQueue_t chan, const void * pData );
