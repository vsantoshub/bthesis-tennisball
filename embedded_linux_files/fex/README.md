script-original.bin: it's the default board configuration script for PcDuino2 board.
script-fev2016.bin: it contains some modifications regarding some used pins in this project.

some modifications are: -All GPIOs are binded to output and low logic level at boot.
                        -PWM pins GPIO5 and GPIO6 are binded to mux level 2 (PWM).
                        -Some GPIOs are binded to high logic level at boot due to project purposes;
