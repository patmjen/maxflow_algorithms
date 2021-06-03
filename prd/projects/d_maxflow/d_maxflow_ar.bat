set home=%~dp0
cd ../../
7z a -tzip -r -ssw -ssc d_maxflow_ar-1.1.zip @%home%\d_maxflow_dirs.txt -xr@%home%\d_maxflow_dirs_exclude.txt
cd %home%