#pragma once

#ifdef __cplusplus
extern "C" {
#endif


extern void state_run( void );
extern void state_connectedToHost( bool flag );
extern void state_connectedFromDevice( bool flag );
extern int state_dump( void );


#ifdef __cplusplus
}
#endif
