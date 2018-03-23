#set(CMAKE_VERBOSE_MAKEFILE on)
set(LIB_SRC 
nd_rudp_api.c
nd_rudp.c
nd_rudp_channel.c
nd_rudp_endpoint.c
nd_rudp_event.c
nd_rudp_packet.c
nd_core/nd_heap.c
nd_core/nd_mutex.c
nd_core/nd_pool.c
nd_core/nd_rbtree.c
nd_core/nd_thread.c
nd_core/nd_time.c

)
add_library(librudp ${LIB_SRC})
set_target_properties(librudp PROPERTIES OUTPUT_NAME "rudp")
