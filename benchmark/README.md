# west build command
cd ~/thesis/zephyrproject/zephyr
west build -b pydrofoil_32 ~/thesis/bare_metal_tests/pydrofoil_app  --pristine -DBOARD_ROOT=/home/seibt/thesis/bare_metal_tests -d ~/thesis/bare_metal_tests/benchmark/build/



# gdb command:
cd ~/thesis/bare_metal_tests/benchmark
gdb-multiarch -x gdb_cmd.gdb build/zephyr/zephyr.elf

