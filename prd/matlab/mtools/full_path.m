function ff = full_path(pth)
%	full_path(pth) converts any relative path to full path
%
% AS

%% get caller's location
stack = dbstack('-completenames');
if (length(stack)>1)
	root = stack(2).file;
else
	root = '';
end
r1 = strfind(root,'/');
r2 = strfind(root,'\');
r = max([r1 r2]);
root = root(1:r-1);
%% if called from command line -- no caller, use cd
if(isempty(root))
	root = cd;
end

%% fprintf('%s\n',root);return;

if (strcmp(pth,'.')) %% path contains only '.'
	pth = '';
	ff = root;
else
	ff = [root '/' pth];
end
%%

ff = fixpath(ff);

%fprintf('path %s\n',ff);

end