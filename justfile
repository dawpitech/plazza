outdir := env("BUILD_OUT", "build")

release: (_build "Release")

_build type:
    cmake -S . -B {{ outdir }} -DCMAKE_BUILD_TYPE={{ type }}
    cmake --build {{ outdir }} -j {{ num_cpus() }}

clean:
    rm -rf {{ outdir }}
    rm -f plazza

re: clean release
