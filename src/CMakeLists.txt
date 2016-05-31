file(GLOB_RECURSE iPot_SOURCES "*.c" "*.cpp")
file(GLOB_RECURSE iPot_HEADERS "*.h" "*.hpp")

#add_directory(../libatomthreads) 

include_directories(../libatomthreads/atomthreads/kernel ../libatomthreads/atomthreads/ports/avr)

add_avr_executable(iPot-fw ${iPot_SOURCES})
avr_target_link_libraries(iPot-fw libatomthreads)

add_dependencies(iPot-fw libatomthreads)
