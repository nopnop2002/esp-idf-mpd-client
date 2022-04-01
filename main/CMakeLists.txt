set(srcs "main.c"
         "ili9340.c"
         "fontx.c"
         "m5stack.c"
         "tcp_client.c"
         "encoder.c")


idf_component_register(SRCS ${srcs} INCLUDE_DIRS ".")
