%MAKE   Compiles the maxflow library.
% 
% >>make maxflowmulti
% >>make maxflowmultiint
%
function make(filename)
    
    if nargin == 0
        %Recursively call itself
		make('maxflowmulti');
		make('maxflowmultiint');
		make('maxflowsingle');
		make('maxflowsingleint');
		return
    end
    
	fprintf('Compiling %s...\n',filename);

    path = mfilename('fullpath');
    file = mfilename;
    path = path(1:end-length(file));

    if matlab_version < 7.5
        %Old version of Matlab
        disp('WARNING: You have a too old version of MATLAB');
        disp('I will try my best to compile anyway');
		pause
    end
    
    if matlab_version < 7.3
        %Old version of Matlab without mwSize and mwIndex types
		%and without the -largeArrayDims switch
        mex('-outdir', path(1:end-1), ...
            '-DmwSize=int', ...
            '-DmwIndex=int', ...
            [path 'split-cpp/' filename '.cpp'], ...
            [path 'split-cpp/maxflow-v3.01/graph.cpp'], ...
            [path 'split-cpp/maxflow-v3.01/maxflow.cpp']);
    else
        %Regular version of Matlab
        if ispc
            %Windows 
            mex('-outdir', path(1:end-1), ...
                '-largeArrayDims', ...
                [path 'split-cpp/' filename '.cpp'], ...
                [path 'split-cpp/maxflow-v3.01/graph.cpp'], ...
                [path 'split-cpp/maxflow-v3.01/maxflow.cpp']);
        else
            %Unix/Linux/MacOS
            mex('-outdir', path(1:end-1), ...
                '-largeArrayDims', ...
                '-lboost_thread-mt',...
                [path 'split-cpp/' filename '.cpp'], ...
                [path 'split-cpp/maxflow-v3.01/graph.cpp'], ...
                [path 'split-cpp/maxflow-v3.01/maxflow.cpp']);
        end
    end

    disp('Compilation OK');
    
    %Configuration with Boost (Windows):
    % 1. cd(prefdir); edit mexopts.bat
    % 2. Add Boost to include dir
    % 3. Add Boost/lib to library dir 
    %   C:\Program Files\boost_1_40_0
    %   C:\Program Files\boost_1_40_0\stage\lib
end

function v = matlab_version
    %File exchange #17285
    v = sscanf (version, '%d.%d.%d');
    v = 10.^(0:-1:-(length(v)-1)) * v;
end
