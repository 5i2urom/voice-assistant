idf_component_register(SRCS "src/AudioProcessor.cpp"
                            "src/HammingWindow.cpp"
                            "src/kissfft/kiss_fft.c"        
                   INCLUDE_DIRS "src" "src/kissfft"
                   REQUIRES audio_input
                   )