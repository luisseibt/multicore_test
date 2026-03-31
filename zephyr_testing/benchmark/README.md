# building the display benchmark application:

## -DBOARD_ROOT and -DSOC_ROOT are specified in the applications cmake
cd <PATH_TO_ZEPHYR_PROJECT>
source .venv/bin/activate
cd zephyr

west build -b pydrofoil_32 <PATH_TO_MULTICORE_TEST>/zephyr_testing/applications/demos --pristine -d <PATH_TO_MULTICORE_TEST>/zephyr_testing/benchmark/build

## example:
west build -b pydrofoil_32 /home/seibt/thesis/multicore_test/zephyr_testing/applications/demos --pristine -d /home/seibt/thesis/multicore_test/zephyr_testing/benchmark/build

# debugging
cd <PATH_TO_MULTICORE_TEST>/zephyr_testing/benchmark
gdb-multiarch -x gdb_cmd.gdb build/zephyr/zephyr.elf