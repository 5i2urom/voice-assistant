idf_component_register(SRCS "src/AudioProcessor.cpp"
                            "src/HammingWindow.cpp"
                            "src/kissfft/kiss_fft.c"
                            "src/kissfft/tools/fftutil.c"
                            "src/kissfft/tools/kfc.c"
                            "src/kissfft/tools/kiss_fastfir.c"
                            "src/kissfft/tools/kiss_fftnd.c"
                            "src/kissfft/tools/kiss_fftndr.c"
                            "src/kissfft/tools/kiss_fftr.c"

                   INCLUDE_DIRS "src" "src/kissfft" "src/kissfft/tools"
                   REQUIRES audio_input
                   )