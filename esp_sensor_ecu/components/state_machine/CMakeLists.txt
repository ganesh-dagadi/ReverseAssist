idf_component_register(SRCS "src/StateMachine.c"
                    INCLUDE_DIRS "include"
                    REQUIRES sensor filter streamer
                    )