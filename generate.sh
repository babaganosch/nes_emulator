if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    genie vs2022
else
    genie ninja
fi