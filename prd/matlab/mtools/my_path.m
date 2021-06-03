function root = my_path()
% returns caller's location

%% get caller's location
stack = dbstack('-completenames');
if (length(stack)>1)
	root = stack(2).file;
else
	root = '';
end

root = unix_slashes(root);

r1 = strfind(root,'/');
r = max(r1);
root = root(1:r-1);
%% if called from command line -- no caller, use cd
if(isempty(root))
	root = unix_slashes(cd);
end

end