file(GLOB_RECURSE libatomthreads_SOURCES "atomthreads/kernel/*.c")
file(GLOB_RECURSE libatomthreads_HEADERS "atomthreads/kernel/*.h")

include_directories(atomthreads/kernel atomthreads/ports/avr)

add_avr_library(
   libatomthreads
   ${libatomthreads_SOURCES}
   atomthreads/ports/avr/atomport.c
   atomthreads/ports/avr/uart.c
   atomthreads/ports/avr/atomport-asm.s
)
