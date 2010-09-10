tmcc -B -host winnt -c ima_rw.c
tmcc -B -host winnt -c adpcm.c
tmcc -B -host winnt -c apiexample.c
tmcc -B -host winnt -c allcodecs.c
tmcc -B -host winnt -c utils.c
tmld ima_rw.o adpcm.o apiexample.o  utils.o allcodecs.o -o adpcm.o