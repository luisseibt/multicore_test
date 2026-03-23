# west build command
cd ~/thesis/zephyrproject/zephyr
west build -b pydrofoil_32 /home/seibt/thesis/multicore_test/zephyr_testing/pydrofoil_app --pristine -DBOARD_ROOT=/home/seibt/thesis/multicore_test/zephyr_testing -DSOC_ROOT=/home/seibt/thesis/multicore_test/zephyr_testing -d /home/seibt/thesis/multicore_test/zephyr_testing/benchmark/build
# gdb command:
cd /home/seibt/thesis/multicore_test/zephyr_testing/benchmark
gdb-multiarch -x gdb_cmd.gdb build/zephyr/zephyr.elf

