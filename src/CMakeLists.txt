idf_component_register(SRCS "state_machine/DetectWakeWordState.cpp"
                            "wake_word_detector.cpp"
                   INCLUDE_DIRS "state_machine"
                   REQUIRES audio_input audio_processor)