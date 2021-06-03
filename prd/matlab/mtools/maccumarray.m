function A = maccumarray(subs,val,sz,varargin)
% 
% A = maccumarray(subs,val,sz,fun,fillval,issparse)
% allows nans in subs and and such elements are ommited
%

if iscell(subs)
subs = cell2mat(subs);
end

m = ~any(isnan(subs),2);

A = accumarray(subs(m,:),val(m),sz,varargin{:});

end